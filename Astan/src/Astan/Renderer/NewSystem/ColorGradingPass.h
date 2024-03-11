#pragma once
#include <Platform/Vulkan/VulkanRHIResource.h>
#include "RenderPass.h"

namespace Astan
{
    struct ColorGradingPassInitInfo : RenderPass::RenderPassInitInfo
    {
        RHIRenderPass* render_pass;
        RHIImageView* input_attachment;
    };

    class ColorGradingPass : public RenderPass
    {
    public:
        void Initialize(const RenderPassInitInfo* init_info);
        void Draw();

        void UpdateAfterFramebufferRecreate(RHIImageView* input_attachment);

    private:
        void SetupDescriptorSetLayout();
        void SetupPipelines();
        void SetupDescriptorSet();
    };
}
