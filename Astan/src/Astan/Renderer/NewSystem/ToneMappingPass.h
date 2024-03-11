#pragma once

#include "RenderPass.h"

namespace Astan
{
    struct ToneMappingPassInitInfo : RenderPass::RenderPassInitInfo
    {
        RHIRenderPass* render_pass;
        RHIImageView* input_attachment;
    };

    class ToneMappingPass : public RenderPass
    {
    public:
        void Initialize(const RenderPassInitInfo* init_info) override final;
        void Draw() override final;

        void UpdateAfterFramebufferRecreate(RHIImageView* input_attachment);

    private:
        void SetupDescriptorSetLayout();
        void SetupPipelines();
        void SetupDescriptorSet();
    };
} // namespace Piccolo
