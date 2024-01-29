#include "aspch.h"
#include <vulkan/vulkan.h>
#include <glfw/glfw3.h>

#include "VulkanRendererAPI.h"

namespace Astan
{
	void VulkanRendererAPI::Init()
	{
		createInstance();
		initDebugMessenger();
		createWindowSurface();
		initPhysicalDevice();
		createLogicalDevice();
		createCommandPool();
		createCommandPools();
		createDescriptorPool();
		createSync();
		createSwapchain();
		createSwapchainImageViews();
		createFramebufferImageAndView();
		createAssetAllocator();
	}


	void VulkanRendererAPI::createInstance()
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


		uint32_t                    enabledLayerCount;
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
	void VulkanRendererAPI::initDebugMessenger()
	{
	}

	void VulkanRendererAPI::createWindowSurface()
	{
		if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS)
			AS_CORE_ERROR("glfwCreateWindowSurface failed!");
	}

	void VulkanRendererAPI::initPhysicalDevice()
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

			std::vector<std::pair<int,VkPhysicalDevice>>
		}

	}

	void VulkanRendererAPI::createLogicalDevice()
	{
	}

	void VulkanRendererAPI::createCommandPool()
	{
	}

	void VulkanRendererAPI::createCommandPools()
	{
	}

	void VulkanRendererAPI::createDescriptorPool()
	{
	}

	void VulkanRendererAPI::createSync()
	{
	}

	void VulkanRendererAPI::createSwapchain()
	{
	}

	void VulkanRendererAPI::createSwapchainImageViews()
	{
	}

	void VulkanRendererAPI::createFramebufferImageAndView()
	{
	}

	void VulkanRendererAPI::createAssetAllocator()
	{
	}
	// -----------------------------//


}