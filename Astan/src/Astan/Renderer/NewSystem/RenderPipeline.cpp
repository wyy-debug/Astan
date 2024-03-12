#include "aspch.h"
#include "RenderPipeline.h"
#include "RenderResource.h"



namespace Astan
{
	RenderPipeline::~RenderPipeline()
	{
	}

	void RenderPipeline::Initialize()
	{
		m_MainCameraPass = CreateRef<MainCameraPass>();

		m_MainCameraPass->SetCommonInfo(m_RenderCommand);


	}
	void RenderPipeline::PreparePassData(Ref<Scene> Scene)
	{
        m_MainCameraPass->PreparePassData(Scene);
        m_PickPass->PreparePassData(Scene);
        m_DirectionalLightShadowPass->PreparePassData(Scene);
        m_PointLightShadowPass->PreparePassData(Scene);
        m_ParticlePass->PreparePassData(Scene);
        //g_runtime_global_context.m_debugdraw_manager->preparePassData(render_resource);
	}
	void RenderPipeline::ForwardRender(Ref<VulkanRendererAPI> rhi, Ref<Scene> scene)
	{
        VulkanRendererAPI* vulkan_rhi = static_cast<VulkanRendererAPI*>(rhi.get());

        scene->ResetRingBufferOffset(vulkan_rhi->m_current_frame_index);

        vulkan_rhi->WaitForFences();

        vulkan_rhi->ResetCommandPool();

        bool recreate_swapchain =
            vulkan_rhi->PrepareBeforePass(std::bind(&RenderPipeline::PassUpdateAfterRecreateSwapchain, this));
        if (recreate_swapchain)
        {
            return;
        }


        ColorGradingPass& color_grading_pass = *(static_cast<ColorGradingPass*>(m_ColorGradingPass.get()));
        FXAAPass& fxaa_pass = *(static_cast<FXAAPass*>(m_FXAAPass.get()));
        ToneMappingPass& tone_mapping_pass = *(static_cast<ToneMappingPass*>(m_ToneMappingPass.get()));
        UIPass& ui_pass = *(static_cast<UIPass*>(m_UIPass.get()));
        CombineUIPass& combine_ui_pass = *(static_cast<CombineUIPass*>(m_CombineUIPass.get()));
        ParticlePass& particle_pass = *(static_cast<ParticlePass*>(m_ParticlePass.get()));

        static_cast<ParticlePass*>(m_ParticlePass.get())
            ->SetRenderCommandBufferHandle(
                static_cast<MainCameraPass*>(m_MainCameraPass.get())->GetRenderCommandBuffer());

        static_cast<MainCameraPass*>(m_MainCameraPass.get())
            ->DrawForward(color_grading_pass,
                fxaa_pass,
                tone_mapping_pass,
                ui_pass,
                combine_ui_pass,
                particle_pass,
                vulkan_rhi->m_current_swapchain_image_index);


        //g_runtime_global_context.m_debugdraw_manager->draw(vulkan_rhi->m_current_swapchain_image_index);

        vulkan_rhi->SubmitRendering(std::bind(&RenderPipeline::PassUpdateAfterRecreateSwapchain, this));
        //static_cast<ParticlePass*>(m_particle_pass.get())->copyNormalAndDepthImage();
        //static_cast<ParticlePass*>(m_particle_pass.get())->simulate();
	}
	void RenderPipeline::DeferredRender(Ref<VulkanRendererAPI> rhi, Ref<RenderResourceBase> render_resource)
	{
	}


    void RenderPipeline::PassUpdateAfterRecreateSwapchain()
    {
        MainCameraPass& mainCameraPass = *(static_cast<MainCameraPass*>(m_MainCameraPass.get()));


        mainCameraPass.UpdateAfterFramebufferRecreate();
        
    }
}