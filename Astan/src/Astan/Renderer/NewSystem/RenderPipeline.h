#pragma once
#include "MainCameraPass.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"
#include "ColorGradingPass.h"

namespace Astan 
{
	class RenderPipeline
	{
	public:
		RenderPipeline() = default;
		~RenderPipeline();
		void Initialize();
		void PreparePassData(std::shared_ptr<RenderResourceBase> render_resource);
		void ForwardRender(std::shared_ptr<VulkanRendererAPI> rhi, std::shared_ptr<RenderResourceBase> render_resource);
		void DeferredRender(std::shared_ptr<VulkanRendererAPI> rhi, std::shared_ptr<RenderResourceBase> render_resource);
		void PassUpdateAfterRecreateSwapchain();
	//TODO father class
	public:
		Ref<VulkanRendererAPI> m_RenderCommand;
	private:
		Ref<MainCameraPass> m_MainCameraPass;
		Ref<ColorGradingPass> m_ColorGradingPass;
	};
}