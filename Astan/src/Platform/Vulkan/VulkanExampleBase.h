#pragma once

#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <ShellScalingAPI.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include <array>
#include <unordered_map>
#include <numeric>
#include <ctime>
#include <iostream>
#include <chrono>
#include <random>
#include <algorithm>
#include <sys/stat.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <numeric>
#include <array>

#include "vulkan/vulkan.h"

#include "VulkanTools.h"
#include "VulkanSwapChain.h"
#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "VulkanTexture.h"

#include "VulkanInitializers.h"
namespace Astan
{
	class VulkanExampleBase
	{
	private:
		std::string getWindowTitle();
		uint32_t destWidth;
		uint32_t destHeight;
		bool resizing = false;
		void windowResize();
		void handleMouseMove(int32_t x, int32_t y);
		void nextFrame();
		void updateOverlay();
		void createPipelineCache();
		void createCommandPool();
		void createSynchronizationPrimitives();
		void initSwapchain();
		void setupSwapChain();
		void createCommandBuffers();
		void destroyCommandBuffers();
		std::string shaderDir = "glsl";
	protected:
		std::string getShadersPath() const;
		std::string getHomeworkShadersPath() const;
		uint32_t frameCounter = 0;
		uint32_t lastFPS = 0;
		std::chrono::time_point<std::chrono::high_resolution_clock> lastTimestamp, tPrevEnd;
		VkInstance instance;
		std::vector<std::string> supportedInstanceExtensions;
		VkPhysicalDevice physicalDevice;
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
		VkPhysicalDeviceFeatures enabledFeatures{};
		std::vector<const char*> enabledDeviceExtensions;
		std::vector<const char*> enabledInstanceExtensions;
		void* deviceCreatepNextChain = nullptr;
		VkDevice device;
		VkQueue queue;
		VkFormat depthFormat;
		VkCommandPool cmdPool;
		VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submitInfo;
		std::vector<VkCommandBuffer> drawCmdBuffers;
		VkRenderPass renderPass = VK_NULL_HANDLE;
		std::vector<VkFramebuffer>frameBuffers;
		uint32_t currentBuffer = 0;
		VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
		std::vector<VkShaderModule> shaderModules;
		VkPipelineCache pipelineCache;
		VulkanSwapChain swapChain;
		struct {
			VkSemaphore presentComplete;
			VkSemaphore renderComplete;
		} semaphores;
		std::vector<VkFence> waitFences;
	public:
		bool prepared = false;
		bool resized = false;
		bool viewUpdated = false;
		uint32_t width = 1280;
		uint32_t height = 720;


		float frameTimer = 1.0f;


		vks::VulkanDevice* vulkanDevice;

		struct Settings {
			bool validation = false;
			bool fullscreen = false;
			bool vsync = false;
			bool overlay = true;
		} settings;

		VkClearColorValue defaultClearColor = { { 0.025f, 0.025f, 0.025f, 1.0f } };

		static std::vector<const char*> args;

		float timer = 0.0f;
		float timerSpeed = 0.25f;
		bool paused = false;

		glm::vec2 mousePos;

		std::string title = "Vulkan Example";
		std::string name = "vulkanExample";
		uint32_t apiVersion = VK_API_VERSION_1_0;

		struct {
			VkImage image;
			VkDeviceMemory mem;
			VkImageView view;
		} depthStencil;

		struct {
			glm::vec2 axisLeft = glm::vec2(0.0f);
			glm::vec2 axisRight = glm::vec2(0.0f);
		} gamePadState;

		struct {
			bool left = false;
			bool right = false;
			bool middle = false;
		} mouseButtons;

#if defined(_WIN32)
		HWND window;
		HINSTANCE windowInstance;
#endif

		bool initVulkan();

		virtual VkResult createInstance(bool enableValidation);
		virtual void render() = 0;
		virtual void buildCommandBuffers();
		virtual void setupDepthStencil();
		virtual void setupFrameBuffer();
		virtual void setupRenderPass();
		virtual void getEnabledFeatures();
		virtual void getEnabledExtensions();

		virtual void prepare();

		VkPipelineShaderStageCreateInfo loadShader(std::string fileName, VkShaderStageFlagBits stage);
		void submitFrame();
		virtual void renderFrame();

	};

	// OS specific macros for the example main entry points
#if defined(_WIN32)
// Windows entry point
#define VULKAN_EXAMPLE_MAIN()																		\
VulkanExample *vulkanExample;																		\
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)						\
{																									\
	if (vulkanExample != NULL)																		\
	{																								\
		vulkanExample->handleMessages(hWnd, uMsg, wParam, lParam);									\
	}																								\
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));												\
}																									\
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)									\
{																									\
	for (int32_t i = 0; i < __argc; i++) { VulkanExample::args.push_back(__argv[i]); };  			\
	vulkanExample = new VulkanExample();															\
	vulkanExample->initVulkan();																	\
	vulkanExample->setupWindow(hInstance, WndProc);													\
	vulkanExample->prepare();																		\
	vulkanExample->renderLoop();																	\
	delete(vulkanExample);																			\
	return 0;																						\
}
#endif
}