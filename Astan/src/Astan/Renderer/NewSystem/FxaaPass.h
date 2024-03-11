#pragma once
#include "RenderPass.h"

namespace Astan
{
    class WindowUI;

    struct FXAAPassInitInfo : RenderPass::RenderPassInitInfo
    {
        RHIRenderPass* render_pass;
        RHIImageView* input_attachment;
    };

    class FXAAPass : public RenderPass
    {
    public:
        void Initialize(const RenderPassInitInfo* init_info) override final;
        void Draw() override final;

        void updateAfterFramebufferRecreate(RHIImageView* input_attachment);

    private:
        void setupDescriptorSetLayout();
        void setupPipelines();
        void setupDescriptorSet();
    };
} // namespace Piccolo
