#pragma once
#include "MainCameraPass.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"
#include "ColorGradingPass.h"
#include "FxaaPass.h"
#include "ToneMappingPass.h"
#include "UIPass.h"
#include "CombineUIPass.h"
#include "ParticlePass.h"
#include "PointLightShadowPass.h"
#include "PickPass.h"
#include "DirectionalLightShadowPass.h"


namespace Astan 
{
	class RenderPipeline
	{
	public:
		RenderPipeline() = default;
		~RenderPipeline();
		void Initialize();
		void PreparePassData(Ref<Scene> Scene);
		void ForwardRender(Ref<VulkanRendererAPI> rhi, Ref<Scene> Scene);
		void DeferredRender(Ref<VulkanRendererAPI> rhi, Ref<RenderResourceBase> render_resource);
		void PassUpdateAfterRecreateSwapchain();
	//TODO father class
	public:
		Ref<VulkanRendererAPI> m_RenderCommand;
	private:
		Ref<MainCameraPass> m_MainCameraPass;
		Ref<PickPass> m_PickPass;
		Ref<PointLightShadowPass> m_PointLightShadowPass;
		Ref<DirectionalLightShadowPass> m_DirectionalLightShadowPass;
		Ref<ColorGradingPass> m_ColorGradingPass;
		Ref<FXAAPass> m_FXAAPass;
		Ref<ToneMappingPass> m_ToneMappingPass;
		Ref<UIPass> m_UIPass;
		Ref<CombineUIPass> m_CombineUIPass;
		Ref<ParticlePass> m_ParticlePass;
	};
}