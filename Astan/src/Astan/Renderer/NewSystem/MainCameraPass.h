#pragma once
#include "RenderPass.h"

namespace Astan
{
	class MainCameraPass : public RenderPass
	{
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