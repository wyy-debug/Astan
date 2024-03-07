#include "aspch.h"
#define GLFW_INCLUDE_VULKAN

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <optional>
#include <set>
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include "VulkanUtil.h"
#include "VulkanRendererAPI.h"
#include "VulkanRHIResource.h"

#define ASTAN_XSTR(s) ASTAN_STR(s)
#define ASTAN_STR(s) #s
namespace Astan
{

	void VulkanRendererAPI::SetWindowHandle()
	{
		Application& app = Application::Get();
		m_window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());
	}

	void VulkanRendererAPI::Initialize()
	{
		SetWindowHandle();
		std::array<int, 2> window_size = { Astan::Application::Get().GetWindow().GetWidth(), Astan::Application::Get().GetWindow().GetHeight() };
		m_viewport = { 0.0f, 0.0f, (float)window_size[0], (float)window_size[1], 0.0f, 1.0f };
		m_scissor = { {0, 0}, {(uint32_t)window_size[0], (uint32_t)window_size[1]} };

		m_enable_validation_Layers = true;
		m_enable_debug_utils_label = true;
		m_enable_point_light_shadow = true;

		char const* vk_layer_path = ASTAN_XSTR(ASTAN_VK_LAYER_PATH);
		SetEnvironmentVariableA("VK_LAYER_PATH", vk_layer_path);
		SetEnvironmentVariableA("DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_1", "1");

		CreateInstance();
		InitializeDebugMessenger();
		CreateWindowSurface();
		InitializePhysicalDevice();
		CreateLogicalDevice();
		CreateCommandPool();
		CreateCommandBuffers();
		CreateDescriptorPool();
		CreateSyncPrimitives();
		CreateSwapchain();
		CreateSwapchainImageViews();
		CreateFramebufferImageAndView();
		CreateAssetAllocator();
	}

	void VulkanRendererAPI::PrepareContext()
	{
	}

	bool VulkanRendererAPI::AllocateCommandBuffers(const RHICommandBufferAllocateInfo* pAllocateInfo, RHICommandBuffer*& pCommandBuffers)
	{
		return false;
	}

	bool VulkanRendererAPI::AllocateDescriptorSets(const RHIDescriptorSetAllocateInfo* pAllocateInfo, RHIDescriptorSet*& pDescriptorSets)
	{
		return false;
	}

	void VulkanRendererAPI::CreateSwapchain()
	{
		// query all supports of this physical device
		SwapChainSupportDetails swapchain_support_details = QuerySwapChainSupport(m_physical_device);

		// choose the best or fitting format
		VkSurfaceFormatKHR chosen_surface_format =
			chooseSwapchainSurfaceFormatFromDetails(swapchain_support_details.formats);
		// choose the best or fitting present mode
		VkPresentModeKHR chosen_presentMode =
			chooseSwapchainPresentModeFromDetails(swapchain_support_details.presentModes);
		// choose the best or fitting extent
		VkExtent2D chosen_extent = chooseSwapchainExtentFromDetails(swapchain_support_details.capabilities);

		uint32_t image_count = swapchain_support_details.capabilities.minImageCount + 1;
		if (swapchain_support_details.capabilities.maxImageCount > 0 &&
			image_count > swapchain_support_details.capabilities.maxImageCount)
		{
			image_count = swapchain_support_details.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_surface;

		createInfo.minImageCount = image_count;
		createInfo.imageFormat = chosen_surface_format.format;
		createInfo.imageColorSpace = chosen_surface_format.colorSpace;
		createInfo.imageExtent = chosen_extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		uint32_t queueFamilyIndices[] = { m_queue_indices.graphics_family.value(), m_queue_indices.present_family.value() };

		if (m_queue_indices.graphics_family != m_queue_indices.present_family)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform = swapchain_support_details.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = chosen_presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain) != VK_SUCCESS)
		{
			AS_CORE_ERROR("vk create swapchain khr");
		}

		vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, nullptr);
		m_swapchain_images.resize(image_count);
		vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, m_swapchain_images.data());

		m_swapchain_image_format = (RHIFormat)chosen_surface_format.format;
		m_swapchain_extent.height = chosen_extent.height;
		m_swapchain_extent.width = chosen_extent.width;

		m_scissor = { {0, 0}, {m_swapchain_extent.width, m_swapchain_extent.height} };
	}

	void VulkanRendererAPI::RecreateSwapchain()
	{
		int width = 0;
		int height = 0;
		glfwGetFramebufferSize(m_window, &width, &height);
		while (width == 0 || height == 0) // minimized 0,0, pause for now
		{
			glfwGetFramebufferSize(m_window, &width, &height);
			glfwWaitEvents();
		}

		VkResult res_wait_for_fences =
			_vkWaitForFences(m_device, k_max_frames_in_flight, m_is_frame_in_flight_fences, VK_TRUE, UINT64_MAX);
		if (VK_SUCCESS != res_wait_for_fences)
		{
			AS_CORE_ERROR("_vkWaitForFences failed");
			return;
		}

		DestroyImageView(m_depth_image_view);
		vkDestroyImage(m_device, ((VulkanImage*)m_depth_image)->getResource(), NULL);
		vkFreeMemory(m_device, m_depth_image_memory, NULL);

		for (auto imageview : m_swapchain_imageviews)
		{
			vkDestroyImageView(m_device, ((VulkanImageView*)imageview)->getResource(), NULL);
		}
		vkDestroySwapchainKHR(m_device, m_swapchain, NULL);

		CreateSwapchain();
		CreateSwapchainImageViews();
		CreateFramebufferImageAndView();
	}

	void VulkanRendererAPI::CreateSwapchainImageViews()
	{
		m_swapchain_imageviews.resize(m_swapchain_images.size());

		// create imageview (one for each this time) for all swapchain images
		for (size_t i = 0; i < m_swapchain_images.size(); i++)
		{
			VkImageView vk_image_view;
			vk_image_view = VulkanUtil::createImageView(m_device,
				m_swapchain_images[i],
				(VkFormat)m_swapchain_image_format,
				VK_IMAGE_ASPECT_COLOR_BIT,
				VK_IMAGE_VIEW_TYPE_2D,
				1,
				1);
			m_swapchain_imageviews[i] = new VulkanImageView();
			((VulkanImageView*)m_swapchain_imageviews[i])->setResource(vk_image_view);
		}
	}

	void VulkanRendererAPI::CreateFramebufferImageAndView()
	{
		VulkanUtil::createImage(m_physical_device,
			m_device,
			m_swapchain_extent.width,
			m_swapchain_extent.height,
			(VkFormat)m_depth_image_format,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			((VulkanImage*)m_depth_image)->getResource(),
			m_depth_image_memory,
			0,
			1,
			1);

		((VulkanImageView*)m_depth_image_view)->setResource(
			VulkanUtil::createImageView(m_device, ((VulkanImage*)m_depth_image)->getResource(), (VkFormat)m_depth_image_format, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D, 1, 1));

	}

	RHISampler* VulkanRendererAPI::GetOrCreateDefaultSampler(RHIDefaultSamplerType type)
	{
		return nullptr;
	}

	RHISampler* VulkanRendererAPI::GetOrCreateMipmapSampler(uint32_t width, uint32_t height)
	{
		return nullptr;
	}

	RHIShader* VulkanRendererAPI::CreateShaderModule(const std::vector<unsigned char>& shader_code)
	{
		RHIShader* shahder = new VulkanShader();

		VkShaderModule vk_shader = VulkanUtil::createShaderModule(m_device, shader_code);

		((VulkanShader*)shahder)->setResource(vk_shader);

		return shahder;
	}

	void VulkanRendererAPI::CreateBuffer(RHIDeviceSize size, RHIBufferUsageFlags usage, RHIMemoryPropertyFlags properties, RHIBuffer*& buffer, RHIDeviceMemory*& buffer_memory)
	{
		VkBuffer vk_buffer;
        VkDeviceMemory vk_device_memory;
        
        VulkanUtil::createBuffer(m_physical_device, m_device, size, usage, properties, vk_buffer, vk_device_memory);

        buffer = new VulkanBuffer();
        buffer_memory = new VulkanDeviceMemory();
        ((VulkanBuffer*)buffer)->setResource(vk_buffer);
        ((VulkanDeviceMemory*)buffer_memory)->setResource(vk_device_memory);
	}

	void VulkanRendererAPI::CreateBufferAndInitialize(RHIBufferUsageFlags usage, RHIMemoryPropertyFlags properties, RHIBuffer*& buffer, RHIDeviceMemory*& buffer_memory, RHIDeviceSize size, void* data, int datasize)
	{
		VkBuffer vk_buffer;
		VkDeviceMemory vk_device_memory;

		VulkanUtil::createBufferAndInitialize(m_device, m_physical_device, usage, properties, &vk_buffer, &vk_device_memory, size, data, datasize);

		buffer = new VulkanBuffer();
		buffer_memory = new VulkanDeviceMemory();
		((VulkanBuffer*)buffer)->setResource(vk_buffer);
		((VulkanDeviceMemory*)buffer_memory)->setResource(vk_device_memory);
	}

	bool VulkanRendererAPI::CreateBufferVMA(VmaAllocator allocator, const RHIBufferCreateInfo* pBufferCreateInfo, const VmaAllocationCreateInfo* pAllocationCreateInfo, RHIBuffer*& pBuffer, VmaAllocation* pAllocation, VmaAllocationInfo* pAllocationInfo)
	{
		VkBuffer vk_buffer;
		VkBufferCreateInfo buffer_create_info{};
		buffer_create_info.sType = (VkStructureType)pBufferCreateInfo->sType;
		buffer_create_info.pNext = (const void*)pBufferCreateInfo->pNext;
		buffer_create_info.flags = (VkBufferCreateFlags)pBufferCreateInfo->flags;
		buffer_create_info.size = (VkDeviceSize)pBufferCreateInfo->size;
		buffer_create_info.usage = (VkBufferUsageFlags)pBufferCreateInfo->usage;
		buffer_create_info.sharingMode = (VkSharingMode)pBufferCreateInfo->sharingMode;
		buffer_create_info.queueFamilyIndexCount = pBufferCreateInfo->queueFamilyIndexCount;
		buffer_create_info.pQueueFamilyIndices = (const uint32_t*)pBufferCreateInfo->pQueueFamilyIndices;

		pBuffer = new VulkanBuffer();
		VkResult result = vmaCreateBuffer(allocator,
			&buffer_create_info,
			pAllocationCreateInfo,
			&vk_buffer,
			pAllocation,
			pAllocationInfo);

		((VulkanBuffer*)pBuffer)->setResource(vk_buffer);

		if (result == VK_SUCCESS)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool VulkanRendererAPI::CreateBufferWithAlignmentVMA(VmaAllocator allocator, const RHIBufferCreateInfo* pBufferCreateInfo, const VmaAllocationCreateInfo* pAllocationCreateInfo, RHIDeviceSize minAlignment, RHIBuffer*& pBuffer, VmaAllocation* pAllocation, VmaAllocationInfo* pAllocationInfo)
	{
		VkBuffer vk_buffer;
		VkBufferCreateInfo buffer_create_info{};
		buffer_create_info.sType = (VkStructureType)pBufferCreateInfo->sType;
		buffer_create_info.pNext = (const void*)pBufferCreateInfo->pNext;
		buffer_create_info.flags = (VkBufferCreateFlags)pBufferCreateInfo->flags;
		buffer_create_info.size = (VkDeviceSize)pBufferCreateInfo->size;
		buffer_create_info.usage = (VkBufferUsageFlags)pBufferCreateInfo->usage;
		buffer_create_info.sharingMode = (VkSharingMode)pBufferCreateInfo->sharingMode;
		buffer_create_info.queueFamilyIndexCount = pBufferCreateInfo->queueFamilyIndexCount;
		buffer_create_info.pQueueFamilyIndices = (const uint32_t*)pBufferCreateInfo->pQueueFamilyIndices;

		pBuffer = new VulkanBuffer();
		VkResult result = vmaCreateBufferWithAlignment(allocator,
			&buffer_create_info,
			pAllocationCreateInfo,
			minAlignment,
			&vk_buffer,
			pAllocation,
			pAllocationInfo);

		((VulkanBuffer*)pBuffer)->setResource(vk_buffer);

		if (result == VK_SUCCESS)
		{
			return true;
		}
		else
		{
			AS_CORE_ERROR("vmaCreateBufferWithAlignment failed!");
			return false;
		}
	}

	void VulkanRendererAPI::Init()
	{
		Initialize();
	}

	VulkanRendererAPI::~VulkanRendererAPI()
	{
	}

	void VulkanRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {}
	void VulkanRendererAPI::SetClearColor(const glm::vec4& color) {}

	void VulkanRendererAPI::Clear() {}

	void VulkanRendererAPI::ClearSwapchain()
	{
	}

	void VulkanRendererAPI::DestroyDefaultSampler(RHIDefaultSamplerType type)
	{
	}

	void VulkanRendererAPI::DestroyMipmappedSampler()
	{
	}

	void VulkanRendererAPI::DestroyShaderModule(RHIShader* shader)
	{
	}

	void VulkanRendererAPI::DestroySemaphore(RHISemaphore* semaphore)
	{
	}

	void VulkanRendererAPI::DestroySampler(RHISampler* sampler)
	{
	}

	void VulkanRendererAPI::DestroyInstance(RHIInstance* instance)
	{
	}

	void VulkanRendererAPI::DestroyImageView(RHIImageView* imageView)
	{
	}

	void VulkanRendererAPI::DestroyImage(RHIImage* image)
	{
	}

	void VulkanRendererAPI::DestroyFramebuffer(RHIFramebuffer* framebuffer)
	{
	}

	void VulkanRendererAPI::DestroyFence(RHIFence* fence)
	{
	}

	void VulkanRendererAPI::DestroyDevice()
	{
	}

	void VulkanRendererAPI::DestroyCommandPool(RHICommandPool* commandPool)
	{
	}

	void VulkanRendererAPI::DestroyBuffer(RHIBuffer*& buffer)
	{
	}

	void VulkanRendererAPI::FreeCommandBuffers(RHICommandPool* commandPool, uint32_t commandBufferCount, RHICommandBuffer* pCommandBuffers)
	{
	}

	void VulkanRendererAPI::FreeMemory(RHIDeviceMemory*& memory)
	{
	}

	bool VulkanRendererAPI::MapMemory(RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size, RHIMemoryMapFlags flags, void** ppData)
	{
		return false;
	}

	void VulkanRendererAPI::UnmapMemory(RHIDeviceMemory* memory)
	{
	}

	void VulkanRendererAPI::InvalidateMappedMemoryRanges(void* pNext, RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size)
	{
	}

	void VulkanRendererAPI::FlushMappedMemoryRanges(void* pNext, RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size)
	{
	}

	RHISemaphore*& VulkanRendererAPI::GetTextureCopySemaphore(uint32_t index)
	{
		// TODO: 在此处插入 return 语句
		return m_image_available_for_texturescopy_semaphores[index];

	}

	void VulkanRendererAPI::CreateInstance()
	{
		// validation layer will be enabled in debug mode
		if (m_enable_validation_Layers && !checkValidationLayerSupport())
		{
			AS_CORE_ERROR("validation layers requested, but not available!");
		}

		m_vulkan_api_version = VK_API_VERSION_1_0;

		// app info
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "astan_renderer";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Astan";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = m_vulkan_api_version;

		// create info
		VkInstanceCreateInfo instance_create_info{};
		instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_create_info.pApplicationInfo = &appInfo; // the appInfo is stored here

		auto extensions = getRequiredExtensions();
		instance_create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		instance_create_info.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (m_enable_validation_Layers)
		{
			instance_create_info.enabledLayerCount = static_cast<uint32_t>(m_validation_layers.size());
			instance_create_info.ppEnabledLayerNames = m_validation_layers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			instance_create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else
		{
			instance_create_info.enabledLayerCount = 0;
			instance_create_info.pNext = nullptr;
		}

		// create m_vulkan_context._instance
		if (vkCreateInstance(&instance_create_info, nullptr, &m_instance) != VK_SUCCESS)
		{
			AS_CORE_ERROR("vk create instance");
		}
	}

	void VulkanRendererAPI::InitializeDebugMessenger()
	{
		if (m_enable_validation_Layers)
		{
			VkDebugUtilsMessengerCreateInfoEXT createInfo;
			populateDebugMessengerCreateInfo(createInfo);
			if (VK_SUCCESS != createDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debug_messenger))
			{
				AS_CORE_ERROR("failed to set up debug messenger!");
			}
		}

		if (m_enable_debug_utils_label)
		{
			_vkCmdBeginDebugUtilsLabelEXT =
				(PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(m_instance, "vkCmdBeginDebugUtilsLabelEXT");
			_vkCmdEndDebugUtilsLabelEXT =
				(PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(m_instance, "vkCmdEndDebugUtilsLabelEXT");
		}
	}

	void VulkanRendererAPI::CreateWindowSurface()
	{
		if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS)
		{
			AS_CORE_ERROR("glfwCreateWindowSurface failed!");
		}
	}

	void VulkanRendererAPI::InitializePhysicalDevice()
	{
		uint32_t physical_device_count;
		vkEnumeratePhysicalDevices(m_instance, &physical_device_count, nullptr);
		if (physical_device_count == 0)
		{
			AS_CORE_ERROR("enumerate physical devices failed!");
		}
		else
		{
			// find one device that matches our requirement
			// or find which is the best
			std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
			vkEnumeratePhysicalDevices(m_instance, &physical_device_count, physical_devices.data());

			std::vector<std::pair<int, VkPhysicalDevice>> ranked_physical_devices;
			for (const auto& device : physical_devices)
			{
				VkPhysicalDeviceProperties physical_device_properties;
				vkGetPhysicalDeviceProperties(device, &physical_device_properties);
				int score = 0;

				if (physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				{
					score += 1000;
				}
				else if (physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
				{
					score += 100;
				}

				ranked_physical_devices.push_back({ score, device });
			}

			std::sort(ranked_physical_devices.begin(),
				ranked_physical_devices.end(),
				[](const std::pair<int, VkPhysicalDevice>& p1, const std::pair<int, VkPhysicalDevice>& p2) {
					return p1 > p2;
				});

			for (const auto& device : ranked_physical_devices)
			{
				if (isDeviceSuitable(device.second))
				{
					m_physical_device = device.second;
					break;
				}
			}

			if (m_physical_device == VK_NULL_HANDLE)
			{
				AS_CORE_ERROR("failed to find suitable physical device");
			}
		}
	}

	void VulkanRendererAPI::CreateLogicalDevice()
	{
		m_queue_indices = findQueueFamilies(m_physical_device);

		std::vector<VkDeviceQueueCreateInfo> queue_create_infos; // all queues that need to be created
		std::set<uint32_t>                   queue_families = { m_queue_indices.graphics_family.value(),
											 m_queue_indices.present_family.value(),
											 m_queue_indices.m_compute_family.value() };

		float queue_priority = 1.0f;
		for (uint32_t queue_family : queue_families) // for every queue family
		{
			// queue create info
			VkDeviceQueueCreateInfo queue_create_info{};
			queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_info.queueFamilyIndex = queue_family;
			queue_create_info.queueCount = 1;
			queue_create_info.pQueuePriorities = &queue_priority;
			queue_create_infos.push_back(queue_create_info);
		}

		// physical device features
		VkPhysicalDeviceFeatures physical_device_features = {};

		physical_device_features.samplerAnisotropy = VK_TRUE;

		// support inefficient readback storage buffer
		physical_device_features.fragmentStoresAndAtomics = VK_TRUE;

		// support independent blending
		physical_device_features.independentBlend = VK_TRUE;

		// support geometry shader
		if (m_enable_point_light_shadow)
		{
			physical_device_features.geometryShader = VK_TRUE;
		}

		// device create info
		VkDeviceCreateInfo device_create_info{};
		device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_create_info.pQueueCreateInfos = queue_create_infos.data();
		device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
		device_create_info.pEnabledFeatures = &physical_device_features;
		device_create_info.enabledExtensionCount = static_cast<uint32_t>(m_device_extensions.size());
		device_create_info.ppEnabledExtensionNames = m_device_extensions.data();
		device_create_info.enabledLayerCount = 0;

		if (vkCreateDevice(m_physical_device, &device_create_info, nullptr, &m_device) != VK_SUCCESS)
		{
			AS_CORE_ERROR("vk create device");
		}

		// initialize queues of this device
		VkQueue vk_graphics_queue;
		vkGetDeviceQueue(m_device, m_queue_indices.graphics_family.value(), 0, &vk_graphics_queue);
		m_graphics_queue = new VulkanQueue();
		((VulkanQueue*)m_graphics_queue)->setResource(vk_graphics_queue);

		vkGetDeviceQueue(m_device, m_queue_indices.present_family.value(), 0, &m_present_queue);

		VkQueue vk_compute_queue;
		vkGetDeviceQueue(m_device, m_queue_indices.m_compute_family.value(), 0, &vk_compute_queue);
		m_compute_queue = new VulkanQueue();
		((VulkanQueue*)m_compute_queue)->setResource(vk_compute_queue);

		// more efficient pointer
		_vkResetCommandPool = (PFN_vkResetCommandPool)vkGetDeviceProcAddr(m_device, "vkResetCommandPool");
		_vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer)vkGetDeviceProcAddr(m_device, "vkBeginCommandBuffer");
		_vkEndCommandBuffer = (PFN_vkEndCommandBuffer)vkGetDeviceProcAddr(m_device, "vkEndCommandBuffer");
		_vkCmdBeginRenderPass = (PFN_vkCmdBeginRenderPass)vkGetDeviceProcAddr(m_device, "vkCmdBeginRenderPass");
		_vkCmdNextSubpass = (PFN_vkCmdNextSubpass)vkGetDeviceProcAddr(m_device, "vkCmdNextSubpass");
		_vkCmdEndRenderPass = (PFN_vkCmdEndRenderPass)vkGetDeviceProcAddr(m_device, "vkCmdEndRenderPass");
		_vkCmdBindPipeline = (PFN_vkCmdBindPipeline)vkGetDeviceProcAddr(m_device, "vkCmdBindPipeline");
		_vkCmdSetViewport = (PFN_vkCmdSetViewport)vkGetDeviceProcAddr(m_device, "vkCmdSetViewport");
		_vkCmdSetScissor = (PFN_vkCmdSetScissor)vkGetDeviceProcAddr(m_device, "vkCmdSetScissor");
		_vkWaitForFences = (PFN_vkWaitForFences)vkGetDeviceProcAddr(m_device, "vkWaitForFences");
		_vkResetFences = (PFN_vkResetFences)vkGetDeviceProcAddr(m_device, "vkResetFences");
		_vkCmdDrawIndexed = (PFN_vkCmdDrawIndexed)vkGetDeviceProcAddr(m_device, "vkCmdDrawIndexed");
		_vkCmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers)vkGetDeviceProcAddr(m_device, "vkCmdBindVertexBuffers");
		_vkCmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer)vkGetDeviceProcAddr(m_device, "vkCmdBindIndexBuffer");
		_vkCmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets)vkGetDeviceProcAddr(m_device, "vkCmdBindDescriptorSets");
		_vkCmdClearAttachments = (PFN_vkCmdClearAttachments)vkGetDeviceProcAddr(m_device, "vkCmdClearAttachments");

		m_depth_image_format = (RHIFormat)findDepthFormat();
	}

	void VulkanRendererAPI::CreateCommandPool()
	{
		// default graphics command pool
		{
			m_rhi_command_pool = new VulkanCommandPool();
			VkCommandPool vk_command_pool;
			VkCommandPoolCreateInfo command_pool_create_info{};
			command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			command_pool_create_info.pNext = NULL;
			command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			command_pool_create_info.queueFamilyIndex = m_queue_indices.graphics_family.value();

			if (vkCreateCommandPool(m_device, &command_pool_create_info, nullptr, &vk_command_pool) != VK_SUCCESS)
			{
				AS_CORE_ERROR("vk create command pool");
			}

			((VulkanCommandPool*)m_rhi_command_pool)->setResource(vk_command_pool);
		}

		// other command pools
		{
			VkCommandPoolCreateInfo command_pool_create_info;
			command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			command_pool_create_info.pNext = NULL;
			command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
			command_pool_create_info.queueFamilyIndex = m_queue_indices.graphics_family.value();

			for (uint32_t i = 0; i < k_max_frames_in_flight; ++i)
			{
				if (vkCreateCommandPool(m_device, &command_pool_create_info, NULL, &m_command_pools[i]) != VK_SUCCESS)
				{
					AS_CORE_ERROR("vk create command pool");
				}
			}
		}
	}

	void VulkanRendererAPI::CreateCommandBuffers()
	{
		VkCommandBufferAllocateInfo command_buffer_allocate_info{};
		command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		command_buffer_allocate_info.commandBufferCount = 1U;

		for (uint32_t i = 0; i < k_max_frames_in_flight; ++i)
		{
			command_buffer_allocate_info.commandPool = m_command_pools[i];
			VkCommandBuffer vk_command_buffer;
			if (vkAllocateCommandBuffers(m_device, &command_buffer_allocate_info, &vk_command_buffer) != VK_SUCCESS)
			{
				AS_CORE_ERROR("vk allocate command buffers");
			}
			m_vk_command_buffers[i] = vk_command_buffer;
			m_command_buffers[i] = new VulkanCommandBuffer();
			((VulkanCommandBuffer*)m_command_buffers[i])->setResource(vk_command_buffer);
		}
	}

	void VulkanRendererAPI::CreateDescriptorPool()
	{
		VkDescriptorPoolSize pool_sizes[7];
		pool_sizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		pool_sizes[0].descriptorCount = 3 + 2 + 2 + 2 + 1 + 1 + 3 + 3;
		pool_sizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		pool_sizes[1].descriptorCount = 1 + 1 + 1 * m_max_vertex_blending_mesh_count;
		pool_sizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pool_sizes[2].descriptorCount = 1 * m_max_material_count;
		pool_sizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		pool_sizes[3].descriptorCount = 3 + 5 * m_max_material_count + 1 + 1; // ImGui_ImplVulkan_CreateDeviceObjects
		pool_sizes[4].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		pool_sizes[4].descriptorCount = 4 + 1 + 1 + 2;
		pool_sizes[5].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		pool_sizes[5].descriptorCount = 3;
		pool_sizes[6].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		pool_sizes[6].descriptorCount = 1;

		VkDescriptorPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.poolSizeCount = sizeof(pool_sizes) / sizeof(pool_sizes[0]);
		pool_info.pPoolSizes = pool_sizes;
		pool_info.maxSets =
			1 + 1 + 1 + m_max_material_count + m_max_vertex_blending_mesh_count + 1 + 1; // +skybox + axis descriptor set
		pool_info.flags = 0U;

		if (vkCreateDescriptorPool(m_device, &pool_info, nullptr, &m_vk_descriptor_pool) != VK_SUCCESS)
		{
			AS_CORE_ERROR("create descriptor pool");
		}

		m_descriptor_pool = new VulkanDescriptorPool();
		((VulkanDescriptorPool*)m_descriptor_pool)->setResource(m_vk_descriptor_pool);
	}

	void VulkanRendererAPI::CreateSyncPrimitives()
	{
		VkSemaphoreCreateInfo semaphore_create_info{};
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_create_info{};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT; // the fence is initialized as signaled

		for (uint32_t i = 0; i < k_max_frames_in_flight; i++)
		{
			m_image_available_for_texturescopy_semaphores[i] = new VulkanSemaphore();
			if (vkCreateSemaphore(
				m_device, &semaphore_create_info, nullptr, &m_image_available_for_render_semaphores[i]) !=
				VK_SUCCESS ||
				vkCreateSemaphore(
					m_device, &semaphore_create_info, nullptr, &m_image_finished_for_presentation_semaphores[i]) !=
				VK_SUCCESS ||
				vkCreateSemaphore(
					m_device, &semaphore_create_info, nullptr, &(((VulkanSemaphore*)m_image_available_for_texturescopy_semaphores[i])->getResource())) !=
				VK_SUCCESS ||
				vkCreateFence(m_device, &fence_create_info, nullptr, &m_is_frame_in_flight_fences[i]) != VK_SUCCESS)
			{
				AS_CORE_ERROR("vk create semaphore & fence");
			}

			m_rhi_is_frame_in_flight_fences[i] = new VulkanFence();
			((VulkanFence*)m_rhi_is_frame_in_flight_fences[i])->setResource(m_is_frame_in_flight_fences[i]);
		}
	}

	void VulkanRendererAPI::CreateAssetAllocator()
	{
		VmaVulkanFunctions vulkanFunctions = {};
		vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
		vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

		VmaAllocatorCreateInfo allocatorCreateInfo = {};
		allocatorCreateInfo.vulkanApiVersion = m_vulkan_api_version;
		allocatorCreateInfo.physicalDevice = m_physical_device;
		allocatorCreateInfo.device = m_device;
		allocatorCreateInfo.instance = m_instance;
		allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

		vmaCreateAllocator(&allocatorCreateInfo, &m_assets_allocator);
	}

	bool VulkanRendererAPI::isPointLightShadowEnabled()
	{
		return false;
	}

	void VulkanRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) {}
	void VulkanRendererAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) {}

	void VulkanRendererAPI::SetLineWidth(float thickness) {}
	void VulkanRendererAPI::CopyBuffer(RHIBuffer* srcBuffer, RHIBuffer* dstBuffer, RHIDeviceSize srcOffset, RHIDeviceSize dstOffset, RHIDeviceSize size)
	{
	}
	void VulkanRendererAPI::CreateImage(uint32_t image_width, uint32_t image_height, RHIFormat format, RHIImageTiling image_tiling, RHIImageUsageFlags image_usage_flags, RHIMemoryPropertyFlags memory_property_flags, RHIImage*& image, RHIDeviceMemory*& memory, RHIImageCreateFlags image_create_flags, uint32_t array_layers, uint32_t miplevels)
	{
		VkImage vk_image;
		VkDeviceMemory vk_device_memory;
		VulkanUtil::createImage(
			m_physical_device,
			m_device,
			image_width,
			image_height,
			(VkFormat)format,
			(VkImageTiling)image_tiling,
			(VkImageUsageFlags)image_usage_flags,
			(VkMemoryPropertyFlags)memory_property_flags,
			vk_image,
			vk_device_memory,
			(VkImageCreateFlags)image_create_flags,
			array_layers,
			miplevels);

		image = new VulkanImage();
		memory = new VulkanDeviceMemory();
		((VulkanImage*)image)->setResource(vk_image);
		((VulkanDeviceMemory*)memory)->setResource(vk_device_memory);
	}
	void VulkanRendererAPI::CreateImageView(RHIImage* image, RHIFormat format, RHIImageAspectFlags image_aspect_flags, RHIImageViewType view_type, uint32_t layout_count, uint32_t miplevels, RHIImageView*& image_view)
	{
		image_view = new VulkanImageView();
		VkImage vk_image = ((VulkanImage*)image)->getResource();
		VkImageView vk_image_view;
		vk_image_view = VulkanUtil::createImageView(m_device, vk_image, (VkFormat)format, image_aspect_flags, (VkImageViewType)view_type, layout_count, miplevels);
		((VulkanImageView*)image_view)->setResource(vk_image_view);
	}
	void VulkanRendererAPI::CreateGlobalImage(RHIImage*& image, RHIImageView*& image_view, VmaAllocation& image_allocation, uint32_t texture_image_width, uint32_t texture_image_height, void* texture_image_pixels, RHIFormat texture_image_format, uint32_t miplevels)
	{
	}
	void VulkanRendererAPI::CreateCubeMap(RHIImage*& image, RHIImageView*& image_view, VmaAllocation& image_allocation, uint32_t texture_image_width, uint32_t texture_image_height, std::array<void*, 6> texture_image_pixels, RHIFormat texture_image_format, uint32_t miplevels)
	{
	}
	bool VulkanRendererAPI::CreateCommandPool(const RHICommandPoolCreateInfo* pCreateInfo, RHICommandPool*& pCommandPool)
	{
		return false;
	}
	bool VulkanRendererAPI::CreateDescriptorPool(const RHIDescriptorPoolCreateInfo* pCreateInfo, RHIDescriptorPool*& pDescriptorPool)
	{
		return false;
	}
	bool VulkanRendererAPI::CreateDescriptorSetLayout(const RHIDescriptorSetLayoutCreateInfo* pCreateInfo, RHIDescriptorSetLayout*& pSetLayout)
	{
		return false;
	}
	bool VulkanRendererAPI::CreateFence(const RHIFenceCreateInfo* pCreateInfo, RHIFence*& pFence)
	{
		return false;
	}
	bool VulkanRendererAPI::CreateFramebuffer(const RHIFramebufferCreateInfo* pCreateInfo, RHIFramebuffer*& pFramebuffer)
	{
		return false;
	}
	bool VulkanRendererAPI::CreateGraphicsPipelines(RHIPipelineCache* pipelineCache, uint32_t createInfoCount, const RHIGraphicsPipelineCreateInfo* pCreateInfos, RHIPipeline*& pPipelines)
	{
		return false;
	}
	bool VulkanRendererAPI::CreateComputePipelines(RHIPipelineCache* pipelineCache, uint32_t createInfoCount, const RHIComputePipelineCreateInfo* pCreateInfos, RHIPipeline*& pPipelines)
	{
		return false;
	}
	bool VulkanRendererAPI::CreatePipelineLayout(const RHIPipelineLayoutCreateInfo* pCreateInfo, RHIPipelineLayout*& pPipelineLayout)
	{
		//descriptor_set_layout
		int descriptor_set_layout_size = pCreateInfo->setLayoutCount;
		std::vector<VkDescriptorSetLayout> vk_descriptor_set_layout_list(descriptor_set_layout_size);
		for (int i = 0; i < descriptor_set_layout_size; ++i)
		{
			const auto& rhi_descriptor_set_layout_element = pCreateInfo->pSetLayouts[i];
			auto& vk_descriptor_set_layout_element = vk_descriptor_set_layout_list[i];

			vk_descriptor_set_layout_element = ((VulkanDescriptorSetLayout*)rhi_descriptor_set_layout_element)->getResource();
		};

		VkPipelineLayoutCreateInfo create_info{};
		create_info.sType = (VkStructureType)pCreateInfo->sType;
		create_info.pNext = (const void*)pCreateInfo->pNext;
		create_info.flags = (VkPipelineLayoutCreateFlags)pCreateInfo->flags;
		create_info.setLayoutCount = pCreateInfo->setLayoutCount;
		create_info.pSetLayouts = vk_descriptor_set_layout_list.data();

		pPipelineLayout = new VulkanPipelineLayout();
		VkPipelineLayout vk_pipeline_layout;
		VkResult result = vkCreatePipelineLayout(m_device, &create_info, nullptr, &vk_pipeline_layout);
		((VulkanPipelineLayout*)pPipelineLayout)->setResource(vk_pipeline_layout);

		if (result == VK_SUCCESS)
		{
			return RHI_SUCCESS;
		}
		else
		{
			AS_CORE_ERROR("vkCreatePipelineLayout failed!");
			return false;
		}
	}
	bool VulkanRendererAPI::CreateRenderPass(const RHIRenderPassCreateInfo* pCreateInfo, RHIRenderPass*& pRenderPass)
	{
		// attachment convert
		std::vector<VkAttachmentDescription> vk_attachments(pCreateInfo->attachmentCount);
		for (int i = 0; i < pCreateInfo->attachmentCount; ++i)
		{
			const auto& rhi_desc = pCreateInfo->pAttachments[i];
			auto& vk_desc = vk_attachments[i];

			vk_desc.flags = (VkAttachmentDescriptionFlags)(rhi_desc).flags;
			vk_desc.format = (VkFormat)(rhi_desc).format;
			vk_desc.samples = (VkSampleCountFlagBits)(rhi_desc).samples;
			vk_desc.loadOp = (VkAttachmentLoadOp)(rhi_desc).loadOp;
			vk_desc.storeOp = (VkAttachmentStoreOp)(rhi_desc).storeOp;
			vk_desc.stencilLoadOp = (VkAttachmentLoadOp)(rhi_desc).stencilLoadOp;
			vk_desc.stencilStoreOp = (VkAttachmentStoreOp)(rhi_desc).stencilStoreOp;
			vk_desc.initialLayout = (VkImageLayout)(rhi_desc).initialLayout;
			vk_desc.finalLayout = (VkImageLayout)(rhi_desc).finalLayout;
		};

		// subpass convert
		int totalAttachmentRefenrence = 0;
		for (int i = 0; i < pCreateInfo->subpassCount; i++)
		{
			const auto& rhi_desc = pCreateInfo->pSubpasses[i];
			totalAttachmentRefenrence += rhi_desc.inputAttachmentCount; // pInputAttachments
			totalAttachmentRefenrence += rhi_desc.colorAttachmentCount; // pColorAttachments
			if (rhi_desc.pDepthStencilAttachment != nullptr)
			{
				totalAttachmentRefenrence += rhi_desc.colorAttachmentCount; // pDepthStencilAttachment
			}
			if (rhi_desc.pResolveAttachments != nullptr)
			{
				totalAttachmentRefenrence += rhi_desc.colorAttachmentCount; // pResolveAttachments
			}
		}
		std::vector<VkSubpassDescription> vk_subpass_description(pCreateInfo->subpassCount);
		std::vector<VkAttachmentReference> vk_attachment_reference(totalAttachmentRefenrence);
		int currentAttachmentRefence = 0;
		for (int i = 0; i < pCreateInfo->subpassCount; ++i)
		{
			const auto& rhi_desc = pCreateInfo->pSubpasses[i];
			auto& vk_desc = vk_subpass_description[i];

			vk_desc.flags = (VkSubpassDescriptionFlags)(rhi_desc).flags;
			vk_desc.pipelineBindPoint = (VkPipelineBindPoint)(rhi_desc).pipelineBindPoint;
			vk_desc.preserveAttachmentCount = (rhi_desc).preserveAttachmentCount;
			vk_desc.pPreserveAttachments = (const uint32_t*)(rhi_desc).pPreserveAttachments;

			vk_desc.inputAttachmentCount = (rhi_desc).inputAttachmentCount;
			vk_desc.pInputAttachments = &vk_attachment_reference[currentAttachmentRefence];
			for (int i = 0; i < (rhi_desc).inputAttachmentCount; i++)
			{
				const auto& rhi_attachment_refence_input = (rhi_desc).pInputAttachments[i];
				auto& vk_attachment_refence_input = vk_attachment_reference[currentAttachmentRefence];

				vk_attachment_refence_input.attachment = rhi_attachment_refence_input.attachment;
				vk_attachment_refence_input.layout = (VkImageLayout)(rhi_attachment_refence_input.layout);

				currentAttachmentRefence += 1;
			};

			vk_desc.colorAttachmentCount = (rhi_desc).colorAttachmentCount;
			vk_desc.pColorAttachments = &vk_attachment_reference[currentAttachmentRefence];
			for (int i = 0; i < (rhi_desc).colorAttachmentCount; ++i)
			{
				const auto& rhi_attachment_refence_color = (rhi_desc).pColorAttachments[i];
				auto& vk_attachment_refence_color = vk_attachment_reference[currentAttachmentRefence];

				vk_attachment_refence_color.attachment = rhi_attachment_refence_color.attachment;
				vk_attachment_refence_color.layout = (VkImageLayout)(rhi_attachment_refence_color.layout);

				currentAttachmentRefence += 1;
			};

			if (rhi_desc.pResolveAttachments != nullptr)
			{
				vk_desc.pResolveAttachments = &vk_attachment_reference[currentAttachmentRefence];
				for (int i = 0; i < (rhi_desc).colorAttachmentCount; ++i)
				{
					const auto& rhi_attachment_refence_resolve = (rhi_desc).pResolveAttachments[i];
					auto& vk_attachment_refence_resolve = vk_attachment_reference[currentAttachmentRefence];

					vk_attachment_refence_resolve.attachment = rhi_attachment_refence_resolve.attachment;
					vk_attachment_refence_resolve.layout = (VkImageLayout)(rhi_attachment_refence_resolve.layout);

					currentAttachmentRefence += 1;
				};
			}

			if (rhi_desc.pDepthStencilAttachment != nullptr)
			{
				vk_desc.pDepthStencilAttachment = &vk_attachment_reference[currentAttachmentRefence];
				for (int i = 0; i < (rhi_desc).colorAttachmentCount; ++i)
				{
					const auto& rhi_attachment_refence_depth = (rhi_desc).pDepthStencilAttachment[i];
					auto& vk_attachment_refence_depth = vk_attachment_reference[currentAttachmentRefence];

					vk_attachment_refence_depth.attachment = rhi_attachment_refence_depth.attachment;
					vk_attachment_refence_depth.layout = (VkImageLayout)(rhi_attachment_refence_depth.layout);

					currentAttachmentRefence += 1;
				};
			};
		};
		if (currentAttachmentRefence != totalAttachmentRefenrence)
		{
			AS_CORE_ERROR("currentAttachmentRefence != totalAttachmentRefenrence");
			return false;
		}

		std::vector<VkSubpassDependency> vk_subpass_depandecy(pCreateInfo->dependencyCount);
		for (int i = 0; i < pCreateInfo->dependencyCount; ++i)
		{
			const auto& rhi_desc = pCreateInfo->pDependencies[i];
			auto& vk_desc = vk_subpass_depandecy[i];

			vk_desc.srcSubpass = rhi_desc.srcSubpass;
			vk_desc.dstSubpass = rhi_desc.dstSubpass;
			vk_desc.srcStageMask = (VkPipelineStageFlags)(rhi_desc).srcStageMask;
			vk_desc.dstStageMask = (VkPipelineStageFlags)(rhi_desc).dstStageMask;
			vk_desc.srcAccessMask = (VkAccessFlags)(rhi_desc).srcAccessMask;
			vk_desc.dstAccessMask = (VkAccessFlags)(rhi_desc).dstAccessMask;
			vk_desc.dependencyFlags = (VkDependencyFlags)(rhi_desc).dependencyFlags;
		};

		VkRenderPassCreateInfo create_info{};
		create_info.sType = (VkStructureType)pCreateInfo->sType;
		create_info.pNext = (const void*)pCreateInfo->pNext;
		create_info.flags = (VkRenderPassCreateFlags)pCreateInfo->flags;
		create_info.attachmentCount = pCreateInfo->attachmentCount;
		create_info.pAttachments = vk_attachments.data();
		create_info.subpassCount = pCreateInfo->subpassCount;
		create_info.pSubpasses = vk_subpass_description.data();
		create_info.dependencyCount = pCreateInfo->dependencyCount;
		create_info.pDependencies = vk_subpass_depandecy.data();

		pRenderPass = new VulkanRenderPass();
		VkRenderPass vk_render_pass;
		VkResult result = vkCreateRenderPass(m_device, &create_info, nullptr, &vk_render_pass);
		((VulkanRenderPass*)pRenderPass)->setResource(vk_render_pass);

		if (result == VK_SUCCESS)
		{
			return RHI_SUCCESS;
		}
		else
		{
			AS_CORE_ERROR("vkCreateRenderPass failed!");
			return false;
		}
	}
	bool VulkanRendererAPI::CreateSampler(const RHISamplerCreateInfo* pCreateInfo, RHISampler*& pSampler)
	{
		return false;
	}
	bool VulkanRendererAPI::createSemaphore(const RHISemaphoreCreateInfo* pCreateInfo, RHISemaphore*& pSemaphore)
	{
		return false;
	}
	bool VulkanRendererAPI::WaitForFencesPFN(uint32_t fenceCount, RHIFence* const* pFence, RHIBool32 waitAll, uint64_t timeout)
	{
		return false;
	}
	bool VulkanRendererAPI::ResetFencesPFN(uint32_t fenceCount, RHIFence* const* pFences)
	{
		return false;
	}
	bool VulkanRendererAPI::ResetCommandPoolPFN(RHICommandPool* commandPool, RHICommandPoolResetFlags flags)
	{
		return false;
	}
	bool VulkanRendererAPI::BeginCommandBufferPFN(RHICommandBuffer* commandBuffer, const RHICommandBufferBeginInfo* pBeginInfo)
	{
		return false;
	}
	bool VulkanRendererAPI::EndCommandBufferPFN(RHICommandBuffer* commandBuffer)
	{
		return false;
	}
	void VulkanRendererAPI::CmdBeginRenderPassPFN(RHICommandBuffer* commandBuffer, const RHIRenderPassBeginInfo* pRenderPassBegin, RHISubpassContents contents)
	{
	}
	void VulkanRendererAPI::CmdNextSubpassPFN(RHICommandBuffer* commandBuffer, RHISubpassContents contents)
	{
	}
	void VulkanRendererAPI::CmdEndRenderPassPFN(RHICommandBuffer* commandBuffer)
	{
	}
	void VulkanRendererAPI::CmdBindPipelinePFN(RHICommandBuffer* commandBuffer, RHIPipelineBindPoint pipelineBindPoint, RHIPipeline* pipeline)
	{
	}
	void VulkanRendererAPI::CmdSetViewportPFN(RHICommandBuffer* commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const RHIViewport* pViewports)
	{
	}
	void VulkanRendererAPI::CmdSetScissorPFN(RHICommandBuffer* commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const RHIRect2D* pScissors)
	{
	}
	void VulkanRendererAPI::CmdBindVertexBuffersPFN(RHICommandBuffer* commandBuffer, uint32_t firstBinding, uint32_t bindingCount, RHIBuffer* const* pBuffers, const RHIDeviceSize* pOffsets)
	{
	}
	void VulkanRendererAPI::CmdBindIndexBufferPFN(RHICommandBuffer* commandBuffer, RHIBuffer* buffer, RHIDeviceSize offset, RHIIndexType indexType)
	{
	}
	void VulkanRendererAPI::CmdBindDescriptorSetsPFN(RHICommandBuffer* commandBuffer, RHIPipelineBindPoint pipelineBindPoint, RHIPipelineLayout* layout, uint32_t firstSet, uint32_t descriptorSetCount, const RHIDescriptorSet* const* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets)
	{
	}
	void VulkanRendererAPI::CmdDrawIndexedPFN(RHICommandBuffer* commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
	{
	}
	void VulkanRendererAPI::CmdClearAttachmentsPFN(RHICommandBuffer* commandBuffer, uint32_t attachmentCount, const RHIClearAttachment* pAttachments, uint32_t rectCount, const RHIClearRect* pRects)
	{
	}
	bool VulkanRendererAPI::BeginCommandBuffer(RHICommandBuffer* commandBuffer, const RHICommandBufferBeginInfo* pBeginInfo)
	{
		return false;
	}
	void VulkanRendererAPI::CmdCopyImageToBuffer(RHICommandBuffer* commandBuffer, RHIImage* srcImage, RHIImageLayout srcImageLayout, RHIBuffer* dstBuffer, uint32_t regionCount, const RHIBufferImageCopy* pRegions)
	{
	}
	void VulkanRendererAPI::CmdCopyImageToImage(RHICommandBuffer* commandBuffer, RHIImage* srcImage, RHIImageAspectFlagBits srcFlag, RHIImage* dstImage, RHIImageAspectFlagBits dstFlag, uint32_t width, uint32_t height)
	{
	}
	void VulkanRendererAPI::CmdCopyBuffer(RHICommandBuffer* commandBuffer, RHIBuffer* srcBuffer, RHIBuffer* dstBuffer, uint32_t regionCount, RHIBufferCopy* pRegions)
	{
	}
	void VulkanRendererAPI::CmdDraw(RHICommandBuffer* commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
	{
	}
	void VulkanRendererAPI::CmdDispatch(RHICommandBuffer* commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
	{
	}
	void VulkanRendererAPI::CmdDispatchIndirect(RHICommandBuffer* commandBuffer, RHIBuffer* buffer, RHIDeviceSize offset)
	{
	}
	void VulkanRendererAPI::CmdPipelineBarrier(RHICommandBuffer* commandBuffer, RHIPipelineStageFlags srcStageMask, RHIPipelineStageFlags dstStageMask, RHIDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const RHIMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const RHIBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const RHIImageMemoryBarrier* pImageMemoryBarriers)
	{
	}
	bool VulkanRendererAPI::EndCommandBuffer(RHICommandBuffer* commandBuffer)
	{
		return false;
	}
	void VulkanRendererAPI::UpdateDescriptorSets(uint32_t descriptorWriteCount, const RHIWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const RHICopyDescriptorSet* pDescriptorCopies)
	{
	}
	bool VulkanRendererAPI::QueueSubmit(RHIQueue* queue, uint32_t submitCount, const RHISubmitInfo* pSubmits, RHIFence* fence)
	{
		return false;
	}
	bool VulkanRendererAPI::QueueWaitIdle(RHIQueue* queue)
	{
		return false;
	}
	void VulkanRendererAPI::ResetCommandPool()
	{
	}
	void VulkanRendererAPI::WaitForFences()
	{
	}
	bool VulkanRendererAPI::WaitForFences(uint32_t fenceCount, const RHIFence* const* pFences, RHIBool32 waitAll, uint64_t timeout)
	{
		return false;
	}
	void VulkanRendererAPI::GetPhysicalDeviceProperties(RHIPhysicalDeviceProperties* pProperties)
	{
	}
	RHICommandBuffer* VulkanRendererAPI::GetCurrentCommandBuffer() const
	{
		return nullptr;
	}
	RHICommandBuffer* const* VulkanRendererAPI::GetCommandBufferList() const
	{
		return nullptr;
	}
	RHICommandPool* VulkanRendererAPI::GetCommandPoor() const
	{
		return nullptr;
	}
	RHIDescriptorPool* VulkanRendererAPI::GetDescriptorPoor() const
	{
		return nullptr;
	}
	RHIFence* const* VulkanRendererAPI::GetFenceList() const
	{
		return nullptr;
	}
	QueueFamilyIndices VulkanRendererAPI::GetQueueFamilyIndices() const
	{
		return QueueFamilyIndices();
	}
	RHIQueue* VulkanRendererAPI::GetGraphicsQueue() const
	{
		return nullptr;
	}
	RHIQueue* VulkanRendererAPI::GetComputeQueue() const
	{
		return nullptr;
	}
	RHISwapChainDesc VulkanRendererAPI::GetSwapchainInfo()
	{
		RHISwapChainDesc desc;
		desc.image_format = m_swapchain_image_format;
		desc.extent = m_swapchain_extent;
		desc.viewport = &m_viewport;
		desc.scissor = &m_scissor;
		desc.imageViews = m_swapchain_imageviews;
		return desc;
	}
	RHIDepthImageDesc VulkanRendererAPI::GetDepthImageInfo() const
	{
		RHIDepthImageDesc desc;
		desc.depth_image_format = m_depth_image_format;
		desc.depth_image_view = m_depth_image_view;
		desc.depth_image = m_depth_image;
		return desc;
	}
	uint8_t VulkanRendererAPI::GetMaxFramesInFlight() const
	{
		return 0;
	}
	uint8_t VulkanRendererAPI::GetCurrentFrameIndex() const
	{
		return 0;
	}
	void VulkanRendererAPI::SetCurrentFrameIndex(uint8_t index)
	{
	}
	RHICommandBuffer* VulkanRendererAPI::BeginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = ((VulkanCommandPool*)m_rhi_command_pool)->getResource();
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer command_buffer;
		vkAllocateCommandBuffers(m_device, &allocInfo, &command_buffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		_vkBeginCommandBuffer(command_buffer, &beginInfo);

		RHICommandBuffer* rhi_command_buffer = new VulkanCommandBuffer();
		((VulkanCommandBuffer*)rhi_command_buffer)->setResource(command_buffer);
		return rhi_command_buffer;
	}
	void VulkanRendererAPI::EndSingleTimeCommands(RHICommandBuffer* command_buffer)
	{
		VkCommandBuffer vk_command_buffer = ((VulkanCommandBuffer*)command_buffer)->getResource();
		_vkEndCommandBuffer(vk_command_buffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &vk_command_buffer;

		vkQueueSubmit(((VulkanQueue*)m_graphics_queue)->getResource(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(((VulkanQueue*)m_graphics_queue)->getResource());

		vkFreeCommandBuffers(m_device, ((VulkanCommandPool*)m_rhi_command_pool)->getResource(), 1, &vk_command_buffer);
		delete(command_buffer);
	}
	bool VulkanRendererAPI::PrepareBeforePass(std::function<void()> passUpdateAfterRecreateSwapchain)
	{
		return false;
	}
	void VulkanRendererAPI::SubmitRendering(std::function<void()> passUpdateAfterRecreateSwapchain)
	{
	}
	void VulkanRendererAPI::PushEvent(RHICommandBuffer* commond_buffer, const char* name, const float* color)
	{
	}
	void VulkanRendererAPI::PopEvent(RHICommandBuffer* commond_buffer)
	{
	}
	bool VulkanRendererAPI::checkValidationLayerSupport()
	{
		return false;
	}
	std::vector<const char*> VulkanRendererAPI::getRequiredExtensions()
	{
		uint32_t     glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (m_enable_validation_Layers || m_enable_debug_utils_label)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	// debug callback
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT,
		VkDebugUtilsMessageTypeFlagsEXT,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void*)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}

	void VulkanRendererAPI::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}
	VkResult VulkanRendererAPI::createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		return VkResult();
	}
	void VulkanRendererAPI::destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
	}
	QueueFamilyIndices VulkanRendererAPI::findQueueFamilies(VkPhysicalDevice physicalm_device)
	{
		QueueFamilyIndices indices;
		uint32_t           queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalm_device, &queue_family_count, nullptr);
		std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalm_device, &queue_family_count, queue_families.data());

		int i = 0;
		for (const auto& queue_family : queue_families)
		{
			if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) // if support graphics command queue
			{
				indices.graphics_family = i;
			}

			if (queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) // if support compute command queue
			{
				indices.m_compute_family = i;
			}


			VkBool32 is_present_support = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalm_device,
				i,
				m_surface,
				&is_present_support); // if support surface presentation
			if (is_present_support)
			{
				indices.present_family = i;
			}

			if (indices.isComplete())
			{
				break;
			}
			i++;
		}
		return indices;
	}

	bool VulkanRendererAPI::checkDeviceExtensionSupport(VkPhysicalDevice physicalm_device)
	{
		uint32_t extension_count;
		vkEnumerateDeviceExtensionProperties(physicalm_device, nullptr, &extension_count, nullptr);

		std::vector<VkExtensionProperties> available_extensions(extension_count);
		vkEnumerateDeviceExtensionProperties(physicalm_device, nullptr, &extension_count, available_extensions.data());

		std::set<std::string> required_extensions(m_device_extensions.begin(), m_device_extensions.end());
		for (const auto& extension : available_extensions)
		{
			required_extensions.erase(extension.extensionName);
		}

		return required_extensions.empty();
	}

	bool VulkanRendererAPI::isDeviceSuitable(VkPhysicalDevice physicalm_device)
	{
		auto queue_indices = findQueueFamilies(physicalm_device);
		bool is_extensions_supported = checkDeviceExtensionSupport(physicalm_device);
		bool is_swapchain_adequate = false;
		if (is_extensions_supported)
		{
			SwapChainSupportDetails swapchain_support_details = QuerySwapChainSupport(physicalm_device);
			is_swapchain_adequate =
				!swapchain_support_details.formats.empty() && !swapchain_support_details.presentModes.empty();
		}

		VkPhysicalDeviceFeatures physicalm_device_features;
		vkGetPhysicalDeviceFeatures(physicalm_device, &physicalm_device_features);

		if (!queue_indices.isComplete() || !is_swapchain_adequate || !physicalm_device_features.samplerAnisotropy)
		{
			return false;
		}

		return true;
	}

	SwapChainSupportDetails VulkanRendererAPI::QuerySwapChainSupport(VkPhysicalDevice physicalm_device)
	{
		SwapChainSupportDetails details_result;

		// capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalm_device, m_surface, &details_result.capabilities);

		// formats
		uint32_t format_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalm_device, m_surface, &format_count, nullptr);
		if (format_count != 0)
		{
			details_result.formats.resize(format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(
				physicalm_device, m_surface, &format_count, details_result.formats.data());
		}

		// present modesQuerySwapChainSupport
		uint32_t presentmode_count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalm_device, m_surface, &presentmode_count, nullptr);
		if (presentmode_count != 0)
		{
			details_result.presentModes.resize(presentmode_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(
				physicalm_device, m_surface, &presentmode_count, details_result.presentModes.data());
		}
		return details_result;
	}
	VkFormat VulkanRendererAPI::findDepthFormat()
	{
		return findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}
	VkFormat VulkanRendererAPI::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(m_physical_device, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		AS_CORE_ERROR("findSupportedFormat failed");
		return VkFormat();
	}
	VkSurfaceFormatKHR VulkanRendererAPI::chooseSwapchainSurfaceFormatFromDetails(const std::vector<VkSurfaceFormatKHR>& available_surface_formats)
	{
		for (const auto& surface_format : available_surface_formats)
		{
			// TODO: select the VK_FORMAT_B8G8R8A8_SRGB surface format,
			// there is no need to do gamma correction in the fragment shader
			if (surface_format.format == VK_FORMAT_B8G8R8A8_UNORM &&
				surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return surface_format;
			}
		}
		return available_surface_formats[0];
	}
	VkPresentModeKHR VulkanRendererAPI::chooseSwapchainPresentModeFromDetails(const std::vector<VkPresentModeKHR>& available_present_modes)
	{
		for (VkPresentModeKHR present_mode : available_present_modes)
        {
            if (VK_PRESENT_MODE_MAILBOX_KHR == present_mode)
            {
                return VK_PRESENT_MODE_MAILBOX_KHR;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
	}
	VkExtent2D VulkanRendererAPI::chooseSwapchainExtentFromDetails(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			return capabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(m_window, &width, &height);

			VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

			actualExtent.width =
				std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height =
				std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}
}