#include "aspch.h"
#include "VulkanRendererAPI.h"
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
		CreateCommandPools();
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
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);
		
		instance_create_info.enabledExtensionCount = glfwExtensionsCount;
		instance_create_info.ppEnabledExtensionNames = glfwExtensions;
		
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
		uint32_t physical_device_cout;
		vkEnumeratePhysicalDevices(m_instance, &physical_device_cout, nullptr);
		if (physical_device_cout)
		{
			AS_CORE_INFO("enumerate physical devices failed!");
		}
		else
		{
			std::vector<VkPhysicalDevice> physical_devices(physical_device_cout);
			vkEnumeratePhysicalDevices(m_instance, &physical_device_cout, physical_devices.data());

			for (const auto& device : physical_devices)
			{
				if (IsDeviceSuitable(device))
					m_physical_device = device;
					break;
			}
			if (m_physical_device == VK_NULL_HANDLE)
				AS_CORE_ERROR("faild to find a suitable GPU");
		}

	}

	void VulkanRendererAPI::CreateLogicalDevice()
	{
	}

	void VulkanRendererAPI::CreateCommandPool()
	{
	}

	void VulkanRendererAPI::CreateCommandPools()
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


	//----------------------------//

	void VulkanRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {}
	void VulkanRendererAPI::SetClearColor(const glm::vec4& color) {}
	void VulkanRendererAPI::Clear() {}

	void VulkanRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) {}
	void VulkanRendererAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) {}

	void VulkanRendererAPI::SetLineWidth(float thickness) {}
}