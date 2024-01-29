#pragma once
#include "Astan/Renderer/RendererAPI.h"

#include <vulkan/vulkan.h>
#include <glfw/glfw3.h>

namespace Astan
{
	class VulkanRendererAPI :public RendererAPI
	{
	public:
		virtual void Init() override;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) override;
		virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) override;

		virtual void SetLineWidth(float thickness) override;

	private:
		/// Init Vulkan 
		/// 1.	Create Vulkan Instance
		/// 2.	Init Debug Message
		/// 3.	Create windows surface
		/// 4.	Init physical device
		/// 5.	Create Logical device
		/// 6.	Create command pool
		/// 7.	Create command buffers
		/// 8.	Create descriptor pool
		/// 9.	Swapchain
		/// 10.	Sync
		/// 11.	Swap view
		/// 12.	Framebuffer image and view
		/// 13.	Asset allocator

		void CreateInstance();
		void InitDebugMessenger();
		void CreateWindowSurface();
		void InitPhysicalDevice();
		void CreateLogicalDevice();
		void CreateCommandPool();
		void CreateCommandPools();
		void CreateDescriptorPool();
		void CreateSync();
		void CreateSwapchain();
		void CreateSwapchainImageViews();
		void CreateFramebufferImageAndView();
		void CreateAssetAllocator();

		// ----------------------------- //
		bool IsDeviceSuitable(VkPhysicalDevice physicalm_device);
	public:
		VkInstance m_instance{ nullptr };
		GLFWwindow* m_window{ nullptr };
		VkSurfaceKHR m_surface{ nullptr };
		VkPhysicalDevice m_physical_device{ nullptr };
		uint32_t m_vulkan_api_version{ VK_API_VERSION_1_0 };

	};
}