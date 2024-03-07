#pragma once
#include "RenderPass.h"

namespace Astan
{
	class MainCameraPass : public RenderPass
	{
    public:
        // 描述集布局枚举
        // 1: per mesh layout
        // 2: global layout
        // 3: mesh per material layout
        // 4: sky box layout
        // 5: axis layout
        // 6: billboard type particle layout
        // 7: gbuffer lighting
        enum LayoutType : uint8_t
        {
            _per_mesh = 0,
            _mesh_global,
            _mesh_per_material,
            _skybox,
            _axis,
            _particle,
            _deferred_lighting,
            _layout_type_count
        };

        // 渲染管线类型
        // 1. model
        // 2. sky box
        // 3. axis
        // 4. billboard type particle
        enum RenderPipeLineType : uint8_t
        {
            _render_pipeline_type_mesh_gbuffer = 0,
            _render_pipeline_type_deferred_lighting,
            _render_pipeline_type_mesh_lighting,
            _render_pipeline_type_skybox,
            _render_pipeline_type_axis,
            _render_pipeline_type_particle,
            _render_pipeline_type_count
        };

	public:
		void Initialize() override;
		void Draw() override;

    private:
        void SetupParticlePass();
        void SetupAttachments();
        void SetupRenderPass();
        void SetupDescriptorSetLayout();
        void SetupPipelines();
        void SetupDescriptorSet();
        void SetupFramebufferDescriptorSet();
        void SetupSwapchainFramebuffers();

        void SetupModelGlobalDescriptorSet();
        void SetupSkyboxDescriptorSet();
        void SetupAxisDescriptorSet();
        void SetupParticleDescriptorSet();
        void SetupGbufferLightingDescriptorSet();

        void DrawMeshGbuffer();
        void DrawDeferredLighting();
        void DrawMeshLighting();
        void DrawSkybox();
        void DrawAxis();
	};
}