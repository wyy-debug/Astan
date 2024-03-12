#pragma once
#include "RenderPass.h"

namespace Astan
{
    class RenderResourceBase;

    struct PickPassInitInfo : RenderPass::RenderPassInitInfo
    {
        RHIDescriptorSetLayout* per_mesh_layout;
    };

    class PickPass : public RenderPass
    {
    public:
        void Initialize(const RenderPassInitInfo* init_info) override final;
        void PostInitialize();
        void PreparePassData(Ref<Scene> Scene);
        void Draw() override final;

        uint32_t Pick(const glm::vec2& picked_uv);
        void     RecreateFramebuffer();

        MeshInefficientPickPerframeStorageBufferObject _mesh_inefficient_pick_perframe_storage_buffer_object;

    private:
        void SetupAttachments();
        void SetupRenderPass();
        void SetupFramebuffer();
        void SetupDescriptorSetLayout();
        void SetupPipelines();
        void SetupDescriptorSet();

    private:
        RHIImage* _object_id_image = nullptr;
        RHIDeviceMemory* _object_id_image_memory = nullptr;
        RHIImageView* _object_id_image_view = nullptr;

        RHIDescriptorSetLayout* _per_mesh_layout = nullptr;
    };
} // namespace Piccolo
#pragma once

