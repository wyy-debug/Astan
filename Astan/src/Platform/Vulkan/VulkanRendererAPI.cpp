#include "aspch.h"
#include "VulkanRendererAPI.h"
#include <set>
namespace Astan
{
	void VulkanRendererAPI::Init()
	{
		CreateInstance();
		InitDebugMessenger();
		CreateWindowSurface();
		InitPhysicalDevice();
		CreateLogicalDevice();
		CreateCommandPool();
		CreateCommandBuffers();
		CreateDescriptorPool();
		CreateSync();
		CreateSwapchain();
		CreateSwapchainImageViews();
		CreateFramebufferImageAndView();
		CreateAssetAllocator();
	}


	void VulkanRendererAPI::CreateInstance()
	{
		m_vulkan_api_version = VK_API_VERSION_1_0;

		// APP info
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Astan";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Astan";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = m_vulkan_api_version;

		uint32_t enabledLayerCount;
		const char* const* ppEnabledLayerNames;
		const char* const* ppEnabledExtensionNames;

		// Create Instance Info
		VkInstanceCreateInfo instance_create_info{};
		instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_create_info.pApplicationInfo = &appInfo;
		
		// TODO fix this
		uint32_t glfwExtensionsCount = 0;
		
		m_glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);
		
		instance_create_info.enabledExtensionCount = glfwExtensionsCount;
		instance_create_info.ppEnabledExtensionNames = m_glfwExtensions;
		
		// TODO next
		instance_create_info.enabledLayerCount = 0;
		instance_create_info.pNext = nullptr;

		if (vkCreateInstance(&instance_create_info, nullptr, &m_instance) != VK_SUCCESS)
			AS_CORE_ERROR("vk create instance");
	}

	// TODO Debug Manage
	void VulkanRendererAPI::InitDebugMessenger()
	{
	}

	void VulkanRendererAPI::CreateWindowSurface()
	{
		if (m_window == nullptr)
			SetWindowHandle();
		if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS)
			AS_CORE_ERROR("glfwCreateWindowSurface failed!");
	}

	void VulkanRendererAPI::InitPhysicalDevice()
	{
		uint32_t physicalDeviceCount;
		vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr);
		if (physicalDeviceCount == 0)
		{
			AS_CORE_INFO("enumerate physical devices failed!");
		}
		else
		{
			std::vector<VkPhysicalDevice> physical_devices(physicalDeviceCount);
			vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physical_devices.data());

			for (const auto& device : physical_devices)
			{
				if (IsDeviceSuitable(device))
					m_physicalDevice = device;
					break;
			}
			if (m_physicalDevice == VK_NULL_HANDLE)
				AS_CORE_ERROR("faild to find a suitable GPU");
		}

	}

	void VulkanRendererAPI::CreateLogicalDevice()
	{
		m_queueIndices = findQueueFamilies(m_physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> queueFamilies = { m_queueIndices.graphicsFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : queueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		// physical device features
		VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
		physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;

		// support inerfficient readback storage buffer
		physicalDeviceFeatures.fragmentStoresAndAtomics = VK_TRUE;

		// support independent blending
		physicalDeviceFeatures.independentBlend = VK_TRUE;

		// device create info

		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;
		deviceCreateInfo.enabledExtensionCount = 0;
		deviceCreateInfo.ppEnabledExtensionNames = m_glfwExtensions;
		deviceCreateInfo.enabledLayerCount = 0;

		if (vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device) != VK_SUCCESS)
			AS_CORE_ERROR("vulkan create device error");

		// init queues of this device
		VkQueue vkGraphicsQueue;
		vkGetDeviceQueue(m_device, m_queueIndices.graphicsFamily.value(), 0, &vkGraphicsQueue);
		// TODO Set Graphics Queue

	}

	void VulkanRendererAPI::CreateCommandPool()
	{
		// graphics command pool 
		{
			VkCommandPoolCreateInfo cmdPoolInfo = {};
			cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			cmdPoolInfo.queueFamilyIndex = m_queueIndices.graphicsFamily.value();
			cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

			if (vkCreateCommandPool(m_device, &cmdPoolInfo, nullptr, &m_cmdPool) != VK_SUCCESS)
			{
				AS_CORE_ERROR("vk create command pool");
			}
		}
	}

	void VulkanRendererAPI::CreateCommandBuffers()
	{
	}

	void VulkanRendererAPI::CreateDescriptorPool()
	{
	}

	void VulkanRendererAPI::CreateSync()
	{
	}

	void VulkanRendererAPI::CreateSwapchain()
	{
	}

	void VulkanRendererAPI::CreateSwapchainImageViews()
	{
	}

	void VulkanRendererAPI::CreateFramebufferImageAndView()
	{
	}

	void VulkanRendererAPI::CreateAssetAllocator()
	{
	}
	// -----------------------------//
	// TODO pick GPU
	bool VulkanRendererAPI::IsDeviceSuitable(VkPhysicalDevice physicalm_device)
	{
		return true;
	}

	void VulkanRendererAPI::SetWindowHandle()
	{
		Application& app = Application::Get();
		m_window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());
	}

	QueueFamilyIndices VulkanRendererAPI::findQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
		
		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				indices.graphicsFamily = i;

			if (indices.isComplete())
				break;
			i++;
		}
		return indices;
	}


	//----------------------------//

	void VulkanRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {}
	void VulkanRendererAPI::SetClearColor(const glm::vec4& color) {}
	void VulkanRendererAPI::Clear() {}

	void VulkanRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) {}
	void VulkanRendererAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) {}

	void VulkanRendererAPI::SetLineWidth(float thickness) {}
}