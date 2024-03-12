#pragma once

#include "RenderPass.h"

namespace Astan
{
    class RenderResourceBase;

    class DirectionalLightShadowPass : public RenderPass
    {
    public:
        void Initialize(const RenderPassInitInfo* init_info) override final;
        void PostInitialize();
        void PreparePassData(std::shared_ptr<RenderResourceBase> render_resource);
        void Draw() override final;

        void SetPerMeshLayout(RHIDescriptorSetLayout* layout) { m_per_mesh_layout = layout; }

    private:
        void SetupAttachments();
        void SetupRenderPass();
        void SetupFramebuffer();
        void SetupDescriptorSetLayout();
        void SetupPipelines();
        void SetupDescriptorSet();
        void DrawModel();

    private:
        RHIDescriptorSetLayout* m_per_mesh_layout;
        MeshDirectionalLightShadowPerframeStorageBufferObject
            m_mesh_directional_light_shadow_perframe_storage_buffer_object;
    };
} // namespace Piccolo
