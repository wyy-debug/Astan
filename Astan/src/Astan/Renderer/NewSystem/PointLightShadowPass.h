#pragma once

#include "RenderPass.h"
namespace Astan
{
    class RenderResourceBase;

    class PointLightShadowPass : public RenderPass
    {
    public:
        void Initialize(const RenderPassInitInfo* init_info) override final;
        void PostInitialize();
        void PreparePassData(Ref<Scene> Scene) ;
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
        MeshPointLightShadowPerframeStorageBufferObject m_mesh_point_light_shadow_perframe_storage_buffer_object;
    };
} // namespace Piccolo
