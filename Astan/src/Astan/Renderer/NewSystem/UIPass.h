#pragma once

#include "RenderPass.h"

namespace Astan
{
    class WindowUI;

    struct UIPassInitInfo : RenderPass::RenderPassInitInfo
    {
        RHIRenderPass* render_pass;
    };

    class UIPass : public RenderPass
    {
    public:
        void Initialize(const RenderPassInitInfo* init_info) override final;
        void InitializeUIRenderBackend(WindowUI* window_ui);
        void Draw() override final;

    private:
        void UploadFonts();

    private:
        WindowUI* m_window_ui;
    };
} // namespace Piccolo
