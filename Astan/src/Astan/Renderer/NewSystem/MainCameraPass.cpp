#include "aspch.h"
#include "MainCameraPass.h"
#include "shader/mesh_vert.h"
#include "shader/mesh_gbuffer_frag.h"
#include "RenderMesh.h"
#include "RenderConfig.h"
#include "shader/deferred_lighting_frag.h"
#include "shader/deferred_lighting_vert.h"
#include "shader/mesh_frag.h"
#include "shader/skybox_vert.h"
#include "shader/skybox_frag.h"
#include "shader/axis_vert.h"
#include "shader/axis_frag.h"
#include "ColorGradingPass.h"
#include "FxaaPass.h"
#include "ToneMappingPass.h"
#include "UIPass.h"
#include "CombineUIPass.h"
#include "ParticlePass.h"
#include <Astan/Scene/Scene.h>

namespace Astan
{
    void MainCameraPass::Initialize()
    {
        // 设置附件
	    SetupAttachments();
        // 设置渲染pass
        SetupRenderPass();
        // 设置描述符布局
        SetupDescriptorSetLayout();
        // 设置渲染管线
        SetupPipelines();
        // 设置描述符
        SetupDescriptorSet();
        // Framebuffer 描述符
        SetupFramebufferDescriptorSet();
        // 交换设置
        SetupSwapchainFramebuffers();
    }

    void MainCameraPass::PreparePassData(Ref<RenderSource> source)
    {
        m_mesh_perframe_storage_buffer_object = source->m_MeshPerframeStorageBufferObject;
        m_axis_storage_buffer_object = source->m_AxisStorageBufferObject;
    }

    void MainCameraPass::Draw(ColorGradingPass& color_grading_pass,
        FXAAPass& fxaa_pass,
        ToneMappingPass& tone_mapping_pass,
        UIPass& ui_pass,
        CombineUIPass& combine_ui_pass,
        ParticlePass& particle_pass,
        uint32_t          current_swapchain_image_index)
    {
        {
            RHIRenderPassBeginInfo renderpass_begin_info{};
            renderpass_begin_info.sType = RHI_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderpass_begin_info.renderPass = m_FrameBuffer.render_pass;
            renderpass_begin_info.framebuffer = m_SwapchainFramebuffers[current_swapchain_image_index];
            renderpass_begin_info.renderArea.offset = { 0, 0 };
            renderpass_begin_info.renderArea.extent = m_RenderCommand->GetSwapchainInfo().extent;

            RHIClearValue clear_values[_main_camera_pass_attachment_count];
            clear_values[_main_camera_pass_gbuffer_a].color = { {0.0f, 0.0f, 0.0f, 0.0f} };
            clear_values[_main_camera_pass_gbuffer_b].color = { {0.0f, 0.0f, 0.0f, 0.0f} };
            clear_values[_main_camera_pass_gbuffer_c].color = { {0.0f, 0.0f, 0.0f, 0.0f} };
            clear_values[_main_camera_pass_backup_buffer_odd].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
            clear_values[_main_camera_pass_backup_buffer_even].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
            clear_values[_main_camera_pass_post_process_buffer_odd].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
            clear_values[_main_camera_pass_post_process_buffer_even].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
            clear_values[_main_camera_pass_depth].depthStencil = { 1.0f, 0 };
            clear_values[_main_camera_pass_swap_chain_image].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
            renderpass_begin_info.clearValueCount = (sizeof(clear_values) / sizeof(clear_values[0]));
            renderpass_begin_info.pClearValues = clear_values;

            m_RenderCommand->CmdBeginRenderPassPFN(m_RenderCommand->GetCurrentCommandBuffer(), &renderpass_begin_info, RHI_SUBPASS_CONTENTS_INLINE);
        }

        float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        m_RenderCommand->PushEvent(m_RenderCommand->GetCurrentCommandBuffer(), "BasePass", color);

        DrawMeshGbuffer();

        m_RenderCommand->PopEvent(m_RenderCommand->GetCurrentCommandBuffer());

        m_RenderCommand->CmdNextSubpassPFN(m_RenderCommand->GetCurrentCommandBuffer(), RHI_SUBPASS_CONTENTS_INLINE);

        m_RenderCommand->PushEvent(m_RenderCommand->GetCurrentCommandBuffer(), "Deferred Lighting", color);

        DrawDeferredLighting();

        m_RenderCommand->PopEvent(m_RenderCommand->GetCurrentCommandBuffer());

        m_RenderCommand->CmdNextSubpassPFN(m_RenderCommand->GetCurrentCommandBuffer(), RHI_SUBPASS_CONTENTS_INLINE);

        m_RenderCommand->PushEvent(m_RenderCommand->GetCurrentCommandBuffer(), "Forward Lighting", color);

        particle_pass.Draw();

        m_RenderCommand->PopEvent(m_RenderCommand->GetCurrentCommandBuffer());

        m_RenderCommand->CmdNextSubpassPFN(m_RenderCommand->GetCurrentCommandBuffer(), RHI_SUBPASS_CONTENTS_INLINE);

        tone_mapping_pass.Draw();

        m_RenderCommand->CmdNextSubpassPFN(m_RenderCommand->GetCurrentCommandBuffer(), RHI_SUBPASS_CONTENTS_INLINE);

        color_grading_pass.Draw();

        m_RenderCommand->CmdNextSubpassPFN(m_RenderCommand->GetCurrentCommandBuffer(), RHI_SUBPASS_CONTENTS_INLINE);

        if (m_enable_fxaa)
            fxaa_pass.Draw();

        m_RenderCommand->CmdNextSubpassPFN(m_RenderCommand->GetCurrentCommandBuffer(), RHI_SUBPASS_CONTENTS_INLINE);

        RHIClearAttachment clear_attachments[1];
        clear_attachments[0].aspectMask = RHI_IMAGE_ASPECT_COLOR_BIT;
        clear_attachments[0].colorAttachment = 0;
        clear_attachments[0].clearValue.color.float32[0] = 0.0;
        clear_attachments[0].clearValue.color.float32[1] = 0.0;
        clear_attachments[0].clearValue.color.float32[2] = 0.0;
        clear_attachments[0].clearValue.color.float32[3] = 0.0;
        RHIClearRect clear_rects[1];
        clear_rects[0].baseArrayLayer = 0;
        clear_rects[0].layerCount = 1;
        clear_rects[0].rect.offset.x = 0;
        clear_rects[0].rect.offset.y = 0;
        clear_rects[0].rect.extent.width = m_RenderCommand->GetSwapchainInfo().extent.width;
        clear_rects[0].rect.extent.height = m_RenderCommand->GetSwapchainInfo().extent.height;
        m_RenderCommand->CmdClearAttachmentsPFN(m_RenderCommand->GetCurrentCommandBuffer(),
            sizeof(clear_attachments) / sizeof(clear_attachments[0]),
            clear_attachments,
            sizeof(clear_rects) / sizeof(clear_rects[0]),
            clear_rects);

        DrawAxis();

        ui_pass.Draw();

        m_RenderCommand->CmdNextSubpassPFN(m_RenderCommand->GetCurrentCommandBuffer(), RHI_SUBPASS_CONTENTS_INLINE);

        combine_ui_pass.Draw();

        m_RenderCommand->CmdEndRenderPassPFN(m_RenderCommand->GetCurrentCommandBuffer());
    }

    void MainCameraPass::DrawForward(ColorGradingPass& color_grading_pass, FXAAPass& fxaa_pass, ToneMappingPass& tone_mapping_pass, UIPass& ui_pass, CombineUIPass& combine_ui_pass, ParticlePass& particle_pass, uint32_t current_swapchain_image_index)
    {
        {
            RHIRenderPassBeginInfo renderpass_begin_info{};
            renderpass_begin_info.sType = RHI_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderpass_begin_info.renderPass = m_FrameBuffer.render_pass;
            renderpass_begin_info.framebuffer = m_SwapchainFramebuffers[current_swapchain_image_index];
            renderpass_begin_info.renderArea.offset = { 0, 0 };
            renderpass_begin_info.renderArea.extent = m_RenderCommand->GetSwapchainInfo().extent;

            RHIClearValue clear_values[_main_camera_pass_attachment_count];
            clear_values[_main_camera_pass_gbuffer_a].color = { {0.0f, 0.0f, 0.0f, 0.0f} };
            clear_values[_main_camera_pass_gbuffer_b].color = { {0.0f, 0.0f, 0.0f, 0.0f} };
            clear_values[_main_camera_pass_gbuffer_c].color = { {0.0f, 0.0f, 0.0f, 0.0f} };
            clear_values[_main_camera_pass_backup_buffer_odd].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
            clear_values[_main_camera_pass_backup_buffer_even].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
            clear_values[_main_camera_pass_depth].depthStencil = { 1.0f, 0 };
            clear_values[_main_camera_pass_swap_chain_image].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
            renderpass_begin_info.clearValueCount = (sizeof(clear_values) / sizeof(clear_values[0]));
            renderpass_begin_info.pClearValues = clear_values;

            m_RenderCommand->CmdBeginRenderPassPFN(m_RenderCommand->GetCurrentCommandBuffer(), &renderpass_begin_info, RHI_SUBPASS_CONTENTS_INLINE);
        }

        m_RenderCommand->CmdNextSubpassPFN(m_RenderCommand->GetCurrentCommandBuffer(), RHI_SUBPASS_CONTENTS_INLINE);

        m_RenderCommand->CmdNextSubpassPFN(m_RenderCommand->GetCurrentCommandBuffer(), RHI_SUBPASS_CONTENTS_INLINE);

        float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        m_RenderCommand->PushEvent(m_RenderCommand->GetCurrentCommandBuffer(), "Forward Lighting", color);

        DrawMeshLighting();
        DrawSkybox();
        particle_pass.Draw();

        m_RenderCommand->PopEvent(m_RenderCommand->GetCurrentCommandBuffer());

        m_RenderCommand->CmdNextSubpassPFN(m_RenderCommand->GetCurrentCommandBuffer(), RHI_SUBPASS_CONTENTS_INLINE);

        tone_mapping_pass.Draw();

        m_RenderCommand->CmdNextSubpassPFN(m_RenderCommand->GetCurrentCommandBuffer(), RHI_SUBPASS_CONTENTS_INLINE);

        color_grading_pass.Draw();

        m_RenderCommand->CmdNextSubpassPFN(m_RenderCommand->GetCurrentCommandBuffer(), RHI_SUBPASS_CONTENTS_INLINE);

        if (m_enable_fxaa)
            fxaa_pass.Draw();

        m_RenderCommand->CmdNextSubpassPFN(m_RenderCommand->GetCurrentCommandBuffer(), RHI_SUBPASS_CONTENTS_INLINE);

        RHIClearAttachment clear_attachments[1];
        clear_attachments[0].aspectMask = RHI_IMAGE_ASPECT_COLOR_BIT;
        clear_attachments[0].colorAttachment = 0;
        clear_attachments[0].clearValue.color.float32[0] = 0.0;
        clear_attachments[0].clearValue.color.float32[1] = 0.0;
        clear_attachments[0].clearValue.color.float32[2] = 0.0;
        clear_attachments[0].clearValue.color.float32[3] = 0.0;
        RHIClearRect clear_rects[1];
        clear_rects[0].baseArrayLayer = 0;
        clear_rects[0].layerCount = 1;
        clear_rects[0].rect.offset.x = 0;
        clear_rects[0].rect.offset.y = 0;
        clear_rects[0].rect.extent.width = m_RenderCommand->GetSwapchainInfo().extent.width;
        clear_rects[0].rect.extent.height = m_RenderCommand->GetSwapchainInfo().extent.height;
        m_RenderCommand->CmdClearAttachmentsPFN(m_RenderCommand->GetCurrentCommandBuffer(),
            sizeof(clear_attachments) / sizeof(clear_attachments[0]),
            clear_attachments,
            sizeof(clear_rects) / sizeof(clear_rects[0]),
            clear_rects);

        DrawAxis();

        ui_pass.Draw();

        m_RenderCommand->CmdNextSubpassPFN(m_RenderCommand->GetCurrentCommandBuffer(), RHI_SUBPASS_CONTENTS_INLINE);

        combine_ui_pass.Draw();

        m_RenderCommand->CmdEndRenderPassPFN(m_RenderCommand->GetCurrentCommandBuffer());

    }

    void MainCameraPass::SetupParticlePass()
    {
    }

    void MainCameraPass::SetupAttachments()
    {
	    // 创建Framebuffer
	    m_FrameBuffer.attachments.resize(_main_camera_pass_custom_attachment_count + _main_camera_pass_post_process_attachment_count);

	    m_FrameBuffer.attachments[_main_camera_pass_gbuffer_a].format = RHI_FORMAT_R8G8B8A8_UNORM;
	    m_FrameBuffer.attachments[_main_camera_pass_gbuffer_b].format = RHI_FORMAT_R8G8B8A8_UNORM;
	    m_FrameBuffer.attachments[_main_camera_pass_gbuffer_c].format = RHI_FORMAT_R8G8B8A8_SRGB;
	    m_FrameBuffer.attachments[_main_camera_pass_backup_buffer_odd].format = RHI_FORMAT_R16G16B16A16_SFLOAT;
	    m_FrameBuffer.attachments[_main_camera_pass_backup_buffer_even].format = RHI_FORMAT_R16G16B16A16_SFLOAT;

	    // _main_camera_pass_gbuffer_a = 0,
	    // _main_camera_pass_gbuffer_b = 1,
	    // _main_camera_pass_gbuffer_c = 2,
	    // _main_camera_pass_backup_buffer_odd = 3,
	    // _main_camera_pass_backup_buffer_even = 4,
	    for (int bufferIndex = 0; bufferIndex < _main_camera_pass_custom_attachment_count; ++bufferIndex)
	    {
		    if (bufferIndex == _main_camera_pass_gbuffer_a)
		    {
			    m_RenderCommand->CreateImage(
				    m_RenderCommand->GetSwapchainInfo().extent.width,
				    m_RenderCommand->GetSwapchainInfo().extent.height,
				    m_FrameBuffer.attachments[_main_camera_pass_gbuffer_a].format,
				    RHI_IMAGE_TILING_OPTIMAL,
				    RHI_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | RHI_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | RHI_IMAGE_USAGE_TRANSFER_SRC_BIT,
				    RHI_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				    m_FrameBuffer.attachments[_main_camera_pass_gbuffer_a].image,
				    m_FrameBuffer.attachments[_main_camera_pass_gbuffer_a].mem,
				    0,
				    1,
				    1
			    );
		    }
		    else
		    {
			    m_RenderCommand->CreateImage(
				    m_RenderCommand->GetSwapchainInfo().extent.width,
				    m_RenderCommand->GetSwapchainInfo().extent.height,
				    m_FrameBuffer.attachments[bufferIndex].format,
				    RHI_IMAGE_TILING_OPTIMAL,
				    RHI_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | RHI_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | RHI_IMAGE_USAGE_TRANSFER_SRC_BIT,
				    RHI_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				    m_FrameBuffer.attachments[bufferIndex].image,
				    m_FrameBuffer.attachments[bufferIndex].mem,
				    0,
				    1,
				    1
			    );
		    }

		    m_RenderCommand->CreateImageView(
			    m_FrameBuffer.attachments[bufferIndex].image,
			    m_FrameBuffer.attachments[bufferIndex].format,
			    RHI_IMAGE_ASPECT_COLOR_BIT,
			    RHI_IMAGE_VIEW_TYPE_2D,
			    1,
			    1,
			    m_FrameBuffer.attachments[bufferIndex].view
		    );
	    }

	    // _main_camera_pass_post_process_buffer_odd = 5,
	    // _main_camera_pass_post_process_buffer_even = 6,
	    m_FrameBuffer.attachments[_main_camera_pass_post_process_buffer_odd].format = RHI_FORMAT_R16G16B16A16_SFLOAT;
	    m_FrameBuffer.attachments[_main_camera_pass_post_process_buffer_even].format = RHI_FORMAT_R16G16B16A16_SFLOAT;
	    for (int attachmentIndex = _main_camera_pass_custom_attachment_count;
		    attachmentIndex <
		    _main_camera_pass_custom_attachment_count + _main_camera_pass_post_process_attachment_count;
		    ++attachmentIndex)
	    {
		    m_RenderCommand->CreateImage(
			    m_RenderCommand->GetSwapchainInfo().extent.width,
			    m_RenderCommand->GetSwapchainInfo().extent.height,
			    m_FrameBuffer.attachments[attachmentIndex].format,
			    RHI_IMAGE_TILING_OPTIMAL,
			    RHI_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | RHI_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | RHI_IMAGE_USAGE_TRANSFER_SRC_BIT,
			    RHI_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			    m_FrameBuffer.attachments[attachmentIndex].image,
			    m_FrameBuffer.attachments[attachmentIndex].mem,
			    0,
			    1,
			    1
		    );

		    m_RenderCommand->CreateImageView(
			    m_FrameBuffer.attachments[attachmentIndex].image,
			    m_FrameBuffer.attachments[attachmentIndex].format,
			    RHI_IMAGE_ASPECT_COLOR_BIT,
			    RHI_IMAGE_VIEW_TYPE_2D,
			    1,
			    1,
			    m_FrameBuffer.attachments[attachmentIndex].view
		    );
	    }
    }

    void MainCameraPass::SetupRenderPass()
    {
	    RHIAttachmentDescription attachments[_main_camera_pass_attachment_count] = {};

	    RHIAttachmentDescription& gbufferNormalAttachmentDescription = attachments[_main_camera_pass_gbuffer_a];
	    gbufferNormalAttachmentDescription.format = m_FrameBuffer.attachments[_main_camera_pass_gbuffer_a].format;
	    gbufferNormalAttachmentDescription.samples = RHI_SAMPLE_COUNT_1_BIT;
	    gbufferNormalAttachmentDescription.loadOp = RHI_ATTACHMENT_LOAD_OP_CLEAR;
	    gbufferNormalAttachmentDescription.storeOp = RHI_ATTACHMENT_STORE_OP_DONT_CARE;
	    gbufferNormalAttachmentDescription.stencilLoadOp = RHI_ATTACHMENT_LOAD_OP_DONT_CARE;
	    gbufferNormalAttachmentDescription.stencilStoreOp = RHI_ATTACHMENT_STORE_OP_DONT_CARE;
	    gbufferNormalAttachmentDescription.initialLayout = RHI_IMAGE_LAYOUT_UNDEFINED;
	    gbufferNormalAttachmentDescription.finalLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	    RHIAttachmentDescription& gbufferMetallicRoughessShadingmodeidAttachmentDescription =
		    attachments[_main_camera_pass_gbuffer_b];
	    gbufferMetallicRoughessShadingmodeidAttachmentDescription.format = m_FrameBuffer.attachments[_main_camera_pass_gbuffer_b].format;
	    gbufferMetallicRoughessShadingmodeidAttachmentDescription.samples = RHI_SAMPLE_COUNT_1_BIT;
	    gbufferMetallicRoughessShadingmodeidAttachmentDescription.loadOp = RHI_ATTACHMENT_LOAD_OP_CLEAR;
	    gbufferMetallicRoughessShadingmodeidAttachmentDescription.storeOp = RHI_ATTACHMENT_STORE_OP_DONT_CARE;
	    gbufferMetallicRoughessShadingmodeidAttachmentDescription.stencilLoadOp = RHI_ATTACHMENT_LOAD_OP_DONT_CARE;
	    gbufferMetallicRoughessShadingmodeidAttachmentDescription.stencilStoreOp = RHI_ATTACHMENT_STORE_OP_DONT_CARE;
	    gbufferMetallicRoughessShadingmodeidAttachmentDescription.initialLayout = RHI_IMAGE_LAYOUT_UNDEFINED;
	    gbufferMetallicRoughessShadingmodeidAttachmentDescription.finalLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	    RHIAttachmentDescription& gbufferAlbedoAttachmentDescription = attachments[_main_camera_pass_gbuffer_c];
	    gbufferAlbedoAttachmentDescription.format = m_FrameBuffer.attachments[_main_camera_pass_gbuffer_c].format;
	    gbufferAlbedoAttachmentDescription.samples = RHI_SAMPLE_COUNT_1_BIT;
	    gbufferAlbedoAttachmentDescription.loadOp = RHI_ATTACHMENT_LOAD_OP_CLEAR;
	    gbufferAlbedoAttachmentDescription.storeOp = RHI_ATTACHMENT_STORE_OP_DONT_CARE;
	    gbufferAlbedoAttachmentDescription.stencilLoadOp = RHI_ATTACHMENT_LOAD_OP_DONT_CARE;
	    gbufferAlbedoAttachmentDescription.stencilStoreOp = RHI_ATTACHMENT_STORE_OP_DONT_CARE;
	    gbufferAlbedoAttachmentDescription.initialLayout = RHI_IMAGE_LAYOUT_UNDEFINED;
	    gbufferAlbedoAttachmentDescription.finalLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	
	    RHIAttachmentDescription& backupOddColorAttachmentDescription = attachments[_main_camera_pass_backup_buffer_odd];
	    backupOddColorAttachmentDescription.format = m_FrameBuffer.attachments[_main_camera_pass_backup_buffer_odd].format;
	    backupOddColorAttachmentDescription.samples = RHI_SAMPLE_COUNT_1_BIT;
	    backupOddColorAttachmentDescription.loadOp = RHI_ATTACHMENT_LOAD_OP_CLEAR;
	    backupOddColorAttachmentDescription.storeOp = RHI_ATTACHMENT_STORE_OP_DONT_CARE;
	    backupOddColorAttachmentDescription.stencilLoadOp = RHI_ATTACHMENT_LOAD_OP_DONT_CARE;
	    backupOddColorAttachmentDescription.stencilStoreOp = RHI_ATTACHMENT_STORE_OP_DONT_CARE;
	    backupOddColorAttachmentDescription.initialLayout = RHI_IMAGE_LAYOUT_UNDEFINED;
	    backupOddColorAttachmentDescription.finalLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	    RHIAttachmentDescription& backupEvenColorAttachmentDescription =
		    attachments[_main_camera_pass_backup_buffer_even];
        backupEvenColorAttachmentDescription.format =
		    m_FrameBuffer.attachments[_main_camera_pass_backup_buffer_even].format;
	    backupEvenColorAttachmentDescription.samples = RHI_SAMPLE_COUNT_1_BIT;
	    backupEvenColorAttachmentDescription.loadOp = RHI_ATTACHMENT_LOAD_OP_CLEAR;
	    backupEvenColorAttachmentDescription.storeOp = RHI_ATTACHMENT_STORE_OP_DONT_CARE;
	    backupEvenColorAttachmentDescription.stencilLoadOp = RHI_ATTACHMENT_LOAD_OP_DONT_CARE;
	    backupEvenColorAttachmentDescription.stencilStoreOp = RHI_ATTACHMENT_STORE_OP_DONT_CARE;
	    backupEvenColorAttachmentDescription.initialLayout = RHI_IMAGE_LAYOUT_UNDEFINED;
	    backupEvenColorAttachmentDescription.finalLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	    RHIAttachmentDescription& postProcessOldColorAttachmentDescription =
		    attachments[_main_camera_pass_post_process_buffer_odd];
	    postProcessOldColorAttachmentDescription.format =
		    m_FrameBuffer.attachments[_main_camera_pass_post_process_buffer_odd].format;
	    postProcessOldColorAttachmentDescription.samples = RHI_SAMPLE_COUNT_1_BIT;
	    postProcessOldColorAttachmentDescription.loadOp = RHI_ATTACHMENT_LOAD_OP_CLEAR;
	    postProcessOldColorAttachmentDescription.storeOp = RHI_ATTACHMENT_STORE_OP_DONT_CARE;
	    postProcessOldColorAttachmentDescription.stencilLoadOp = RHI_ATTACHMENT_LOAD_OP_DONT_CARE;
	    postProcessOldColorAttachmentDescription.stencilStoreOp = RHI_ATTACHMENT_STORE_OP_DONT_CARE;
	    postProcessOldColorAttachmentDescription.initialLayout = RHI_IMAGE_LAYOUT_UNDEFINED;
	    postProcessOldColorAttachmentDescription.finalLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	    RHIAttachmentDescription& postProcessEvenColorAttachmentDescription =
		    attachments[_main_camera_pass_post_process_buffer_even];
	    postProcessEvenColorAttachmentDescription.format =
		    m_FrameBuffer.attachments[_main_camera_pass_post_process_buffer_even].format;
	    postProcessEvenColorAttachmentDescription.samples = RHI_SAMPLE_COUNT_1_BIT;
	    postProcessEvenColorAttachmentDescription.loadOp = RHI_ATTACHMENT_LOAD_OP_CLEAR;
	    postProcessEvenColorAttachmentDescription.storeOp = RHI_ATTACHMENT_STORE_OP_DONT_CARE;
	    postProcessEvenColorAttachmentDescription.stencilLoadOp = RHI_ATTACHMENT_LOAD_OP_DONT_CARE;
	    postProcessEvenColorAttachmentDescription.stencilStoreOp = RHI_ATTACHMENT_STORE_OP_DONT_CARE;
	    postProcessEvenColorAttachmentDescription.initialLayout = RHI_IMAGE_LAYOUT_UNDEFINED;
	    postProcessEvenColorAttachmentDescription.finalLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	    RHIAttachmentDescription& depthAttachmentDescription = attachments[_main_camera_pass_depth];
	    depthAttachmentDescription.format = m_RenderCommand->GetDepthImageInfo().depth_image_format;
	    depthAttachmentDescription.samples = RHI_SAMPLE_COUNT_1_BIT;
	    depthAttachmentDescription.loadOp = RHI_ATTACHMENT_LOAD_OP_CLEAR;
	    depthAttachmentDescription.storeOp = RHI_ATTACHMENT_STORE_OP_STORE;
	    depthAttachmentDescription.stencilLoadOp = RHI_ATTACHMENT_LOAD_OP_DONT_CARE;
	    depthAttachmentDescription.stencilStoreOp = RHI_ATTACHMENT_STORE_OP_DONT_CARE;
	    depthAttachmentDescription.initialLayout = RHI_IMAGE_LAYOUT_UNDEFINED;
	    depthAttachmentDescription.finalLayout = RHI_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	    RHIAttachmentDescription& swapchainImageAttachmentDescription =
		    attachments[_main_camera_pass_swap_chain_image];
	    swapchainImageAttachmentDescription.format = m_RenderCommand->GetSwapchainInfo().image_format;
	    swapchainImageAttachmentDescription.samples = RHI_SAMPLE_COUNT_1_BIT;
	    swapchainImageAttachmentDescription.loadOp = RHI_ATTACHMENT_LOAD_OP_CLEAR;
	    swapchainImageAttachmentDescription.storeOp = RHI_ATTACHMENT_STORE_OP_STORE;
	    swapchainImageAttachmentDescription.stencilLoadOp = RHI_ATTACHMENT_LOAD_OP_DONT_CARE;
	    swapchainImageAttachmentDescription.stencilStoreOp = RHI_ATTACHMENT_STORE_OP_DONT_CARE;
	    swapchainImageAttachmentDescription.initialLayout = RHI_IMAGE_LAYOUT_UNDEFINED;
	    swapchainImageAttachmentDescription.finalLayout = RHI_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	    RHISubpassDescription subpasses[_main_camera_subpass_count] = {};

	    // 基础颜色附件
	    RHIAttachmentReference bassPassColorAttachmentsRefence[3] = {};
	    bassPassColorAttachmentsRefence[0].attachment = &gbufferNormalAttachmentDescription - attachments;
	    bassPassColorAttachmentsRefence[0].layout = RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	    bassPassColorAttachmentsRefence[1].attachment =
		    &gbufferMetallicRoughessShadingmodeidAttachmentDescription - attachments;
	    bassPassColorAttachmentsRefence[1].layout = RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	    bassPassColorAttachmentsRefence[2].attachment = &gbufferAlbedoAttachmentDescription - attachments;
	    bassPassColorAttachmentsRefence[2].layout = RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	    RHIAttachmentReference bassPassDepthAttachmentReference{};
	    bassPassDepthAttachmentReference.attachment = &depthAttachmentDescription - attachments;
	    bassPassDepthAttachmentReference.layout = RHI_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	    // 基础颜色subpass
	    RHISubpassDescription& basePass = subpasses[_main_camera_subpass_basepass];
	    basePass.pipelineBindPoint = RHI_PIPELINE_BIND_POINT_GRAPHICS;
	    basePass.colorAttachmentCount =
		    sizeof(bassPassColorAttachmentsRefence) / sizeof(bassPassColorAttachmentsRefence[0]);
	    basePass.pColorAttachments = &bassPassColorAttachmentsRefence[0];
	    basePass.pDepthStencilAttachment = &bassPassDepthAttachmentReference;
	    basePass.preserveAttachmentCount = 0;
	    basePass.pPreserveAttachments = NULL;


        // 延迟渲染光照附件
        RHIAttachmentReference deferredLightingPassInputAttachmentsReference[4] = {};
        deferredLightingPassInputAttachmentsReference[0].attachment =
            &gbufferNormalAttachmentDescription - attachments;
        deferredLightingPassInputAttachmentsReference[0].layout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        deferredLightingPassInputAttachmentsReference[1].attachment =
            &gbufferMetallicRoughessShadingmodeidAttachmentDescription - attachments;
        deferredLightingPassInputAttachmentsReference[1].layout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        deferredLightingPassInputAttachmentsReference[2].attachment =
            &gbufferAlbedoAttachmentDescription - attachments;
        deferredLightingPassInputAttachmentsReference[2].layout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        deferredLightingPassInputAttachmentsReference[3].attachment = &depthAttachmentDescription - attachments;
        deferredLightingPassInputAttachmentsReference[3].layout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // 延迟渲染光照附件 backup 上一帧？
        RHIAttachmentReference deferredLightingPassColorAttachmentsReference[1] = {};
        deferredLightingPassColorAttachmentsReference[0].attachment =
            &backupOddColorAttachmentDescription - attachments;
        deferredLightingPassColorAttachmentsReference[0].layout = RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // 延迟渲染subpass
        RHISubpassDescription& deferredLightingPass = subpasses[_main_camera_subpass_deferred_lighting];
        deferredLightingPass.pipelineBindPoint = RHI_PIPELINE_BIND_POINT_GRAPHICS;
        deferredLightingPass.inputAttachmentCount = sizeof(deferredLightingPassInputAttachmentsReference) /
            sizeof(deferredLightingPassInputAttachmentsReference[0]);
        deferredLightingPass.pInputAttachments = &deferredLightingPassInputAttachmentsReference[0];
        deferredLightingPass.colorAttachmentCount = sizeof(deferredLightingPassColorAttachmentsReference) /
            sizeof(deferredLightingPassColorAttachmentsReference[0]);
        deferredLightingPass.pColorAttachments = &deferredLightingPassColorAttachmentsReference[0];
        deferredLightingPass.pDepthStencilAttachment = NULL;
        deferredLightingPass.preserveAttachmentCount = 0;
        deferredLightingPass.pPreserveAttachments = NULL;

        // 前向渲染颜色附件
        RHIAttachmentReference forwardLightingPassColorAttachmentsReference[1] = {};
        forwardLightingPassColorAttachmentsReference[0].attachment =
            &backupOddColorAttachmentDescription - attachments;
        forwardLightingPassColorAttachmentsReference[0].layout = RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // 前向渲染深度附件
        RHIAttachmentReference forwardLightingPassDepthAttachmentReference{};
        forwardLightingPassDepthAttachmentReference.attachment = &depthAttachmentDescription - attachments;
        forwardLightingPassDepthAttachmentReference.layout = RHI_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // 前向渲染subpass
        RHISubpassDescription& forwardLightingPass = subpasses[_main_camera_subpass_forward_lighting];
        forwardLightingPass.pipelineBindPoint = RHI_PIPELINE_BIND_POINT_GRAPHICS;
        forwardLightingPass.inputAttachmentCount = 0U;
        forwardLightingPass.pInputAttachments = NULL;
        forwardLightingPass.colorAttachmentCount = sizeof(forwardLightingPassColorAttachmentsReference) /
            sizeof(forwardLightingPassColorAttachmentsReference[0]);
        forwardLightingPass.pColorAttachments = &forwardLightingPassColorAttachmentsReference[0];
        forwardLightingPass.pDepthStencilAttachment = &forwardLightingPassDepthAttachmentReference;
        forwardLightingPass.preserveAttachmentCount = 0;
        forwardLightingPass.pPreserveAttachments = NULL;

        // toneMapping 附件
        RHIAttachmentReference toneMappingPassInputAttachmentReference{};
        toneMappingPassInputAttachmentReference.attachment =
            &backupOddColorAttachmentDescription - attachments;
        toneMappingPassInputAttachmentReference.layout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // toneMapping 颜色附件
        RHIAttachmentReference toneMappingPassColorAttachmentReference{};
        toneMappingPassColorAttachmentReference.attachment =
            &backupEvenColorAttachmentDescription - attachments;
        toneMappingPassColorAttachmentReference.layout = RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // toneMapping subpass
        RHISubpassDescription& toneMappingPass = subpasses[_main_camera_subpass_tone_mapping];
        toneMappingPass.pipelineBindPoint = RHI_PIPELINE_BIND_POINT_GRAPHICS;
        toneMappingPass.inputAttachmentCount = 1;
        toneMappingPass.pInputAttachments = &toneMappingPassInputAttachmentReference;
        toneMappingPass.colorAttachmentCount = 1;
        toneMappingPass.pColorAttachments = &toneMappingPassColorAttachmentReference;
        toneMappingPass.pDepthStencilAttachment = NULL;
        toneMappingPass.preserveAttachmentCount = 0;
        toneMappingPass.pPreserveAttachments = NULL;

        // color grading 颜色附件
        RHIAttachmentReference colorGradingPssInputAttachmentReference{};
        colorGradingPssInputAttachmentReference.attachment =
            &backupEvenColorAttachmentDescription - attachments;
        colorGradingPssInputAttachmentReference.layout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // color grading 颜色附件 TODO FXAA
        RHIAttachmentReference colorGradingPassColorAttachmentReference{};
        colorGradingPassColorAttachmentReference.attachment =
            &backupOddColorAttachmentDescription - attachments;
        colorGradingPassColorAttachmentReference.layout = RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // color grading subpass
        RHISubpassDescription& colorGradingPass = subpasses[_main_camera_subpass_color_grading];
        colorGradingPass.pipelineBindPoint = RHI_PIPELINE_BIND_POINT_GRAPHICS;
        colorGradingPass.inputAttachmentCount = 1;
        colorGradingPass.pInputAttachments = &colorGradingPssInputAttachmentReference;
        colorGradingPass.colorAttachmentCount = 1;
        colorGradingPass.pColorAttachments = &colorGradingPassColorAttachmentReference;
        colorGradingPass.pDepthStencilAttachment = NULL;
        colorGradingPass.preserveAttachmentCount = 0;
        colorGradingPass.pPreserveAttachments = NULL;

        // fxaa todo 
        RHIAttachmentReference fxaaPassInputAttachmentReference{};
        fxaaPassInputAttachmentReference.attachment = &backupEvenColorAttachmentDescription - attachments;
        fxaaPassInputAttachmentReference.layout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        RHIAttachmentReference fxaaPassColorAttachmentReference{};
        fxaaPassColorAttachmentReference.attachment = &backupOddColorAttachmentDescription - attachments;
        fxaaPassColorAttachmentReference.layout = RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        RHISubpassDescription& fxaaPass = subpasses[_main_camera_subpass_fxaa];
        fxaaPass.pipelineBindPoint = RHI_PIPELINE_BIND_POINT_GRAPHICS;
        fxaaPass.inputAttachmentCount = 1;
        fxaaPass.pInputAttachments = &fxaaPassInputAttachmentReference;
        fxaaPass.colorAttachmentCount = 1;
        fxaaPass.pColorAttachments = &fxaaPassColorAttachmentReference;
        fxaaPass.pDepthStencilAttachment = NULL;
        fxaaPass.preserveAttachmentCount = 0;
        fxaaPass.pPreserveAttachments = NULL;

        // UI pass
        RHIAttachmentReference uiPassColorAttachmentReference{};
        uiPassColorAttachmentReference.attachment = &backupEvenColorAttachmentDescription - attachments;
        uiPassColorAttachmentReference.layout = RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        uint32_t uiPassPreserveAttachment = &backupOddColorAttachmentDescription - attachments;

        RHISubpassDescription& uiPass = subpasses[_main_camera_subpass_ui];
        uiPass.pipelineBindPoint = RHI_PIPELINE_BIND_POINT_GRAPHICS;
        uiPass.inputAttachmentCount = 0;
        uiPass.pInputAttachments = NULL;
        uiPass.colorAttachmentCount = 1;
        uiPass.pColorAttachments = &uiPassColorAttachmentReference;
        uiPass.pDepthStencilAttachment = NULL;
        uiPass.preserveAttachmentCount = 1;
        uiPass.pPreserveAttachments = &uiPassPreserveAttachment;

        // combime 附件
        RHIAttachmentReference combineuiPassInputAttachmentsReference[2] = {};
        combineuiPassInputAttachmentsReference[0].attachment =
            &backupOddColorAttachmentDescription - attachments;
        combineuiPassInputAttachmentsReference[0].layout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        combineuiPassInputAttachmentsReference[1].attachment =
            &backupEvenColorAttachmentDescription - attachments;
        combineuiPassInputAttachmentsReference[1].layout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        RHIAttachmentReference combineUiPassColorAttachmentReference{};
        combineUiPassColorAttachmentReference.attachment = &swapchainImageAttachmentDescription - attachments;
        combineUiPassColorAttachmentReference.layout = RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        RHISubpassDescription& combineUiPass = subpasses[_main_camera_subpass_combine_ui];
        combineUiPass.pipelineBindPoint = RHI_PIPELINE_BIND_POINT_GRAPHICS;
        combineUiPass.inputAttachmentCount = sizeof(combineuiPassInputAttachmentsReference) /
            sizeof(combineuiPassInputAttachmentsReference[0]);
        combineUiPass.pInputAttachments = combineuiPassInputAttachmentsReference;
        combineUiPass.colorAttachmentCount = 1;
        combineUiPass.pColorAttachments = &combineUiPassColorAttachmentReference;
        combineUiPass.pDepthStencilAttachment = NULL;
        combineUiPass.preserveAttachmentCount = 0;
        combineUiPass.pPreserveAttachments = NULL;

        RHISubpassDependency dependencies[8] = {};

        RHISubpassDependency& deferredLigtingPassDependOnShadowMapPass = dependencies[0];
        deferredLigtingPassDependOnShadowMapPass.srcSubpass = RHI_SUBPASS_EXTERNAL;
        deferredLigtingPassDependOnShadowMapPass.dstSubpass = _main_camera_subpass_deferred_lighting;
        deferredLigtingPassDependOnShadowMapPass.srcStageMask = RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        deferredLigtingPassDependOnShadowMapPass.dstStageMask = RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        deferredLigtingPassDependOnShadowMapPass.srcAccessMask = RHI_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        deferredLigtingPassDependOnShadowMapPass.dstAccessMask = RHI_ACCESS_SHADER_READ_BIT;
        deferredLigtingPassDependOnShadowMapPass.dependencyFlags = 0; // NOT BY REGION

        RHISubpassDependency& deferredLightingPassDependOnBassPass = dependencies[1];
        deferredLightingPassDependOnBassPass.srcSubpass = _main_camera_subpass_basepass;
        deferredLightingPassDependOnBassPass.dstSubpass = _main_camera_subpass_deferred_lighting;
        deferredLightingPassDependOnBassPass.srcStageMask =
            RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        deferredLightingPassDependOnBassPass.dstStageMask =
            RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        deferredLightingPassDependOnBassPass.srcAccessMask =
            RHI_ACCESS_SHADER_WRITE_BIT | RHI_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        deferredLightingPassDependOnBassPass.dstAccessMask =
            RHI_ACCESS_SHADER_READ_BIT | RHI_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        deferredLightingPassDependOnBassPass.dependencyFlags = RHI_DEPENDENCY_BY_REGION_BIT;

        RHISubpassDependency& forwardLightingPassDependOnDeferredLightingPass = dependencies[2];
        forwardLightingPassDependOnDeferredLightingPass.srcSubpass = _main_camera_subpass_deferred_lighting;
        forwardLightingPassDependOnDeferredLightingPass.dstSubpass = _main_camera_subpass_forward_lighting;
        forwardLightingPassDependOnDeferredLightingPass.srcStageMask =
            RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        forwardLightingPassDependOnDeferredLightingPass.dstStageMask =
            RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        forwardLightingPassDependOnDeferredLightingPass.srcAccessMask =
            RHI_ACCESS_SHADER_WRITE_BIT | RHI_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        forwardLightingPassDependOnDeferredLightingPass.dstAccessMask =
            RHI_ACCESS_SHADER_READ_BIT | RHI_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        forwardLightingPassDependOnDeferredLightingPass.dependencyFlags = RHI_DEPENDENCY_BY_REGION_BIT;

        RHISubpassDependency& toneMappingPassDependOnLightingPass = dependencies[3];
        toneMappingPassDependOnLightingPass.srcSubpass = _main_camera_subpass_forward_lighting;
        toneMappingPassDependOnLightingPass.dstSubpass = _main_camera_subpass_tone_mapping;
        toneMappingPassDependOnLightingPass.srcStageMask =
            RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        toneMappingPassDependOnLightingPass.dstStageMask =
            RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        toneMappingPassDependOnLightingPass.srcAccessMask =
            RHI_ACCESS_SHADER_WRITE_BIT | RHI_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        toneMappingPassDependOnLightingPass.dstAccessMask =
            RHI_ACCESS_SHADER_READ_BIT | RHI_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        toneMappingPassDependOnLightingPass.dependencyFlags = RHI_DEPENDENCY_BY_REGION_BIT;

        RHISubpassDependency& colorGradingPassDependOnToneMappingPass = dependencies[4];
        colorGradingPassDependOnToneMappingPass.srcSubpass = _main_camera_subpass_tone_mapping;
        colorGradingPassDependOnToneMappingPass.dstSubpass = _main_camera_subpass_color_grading;
        colorGradingPassDependOnToneMappingPass.srcStageMask =
            RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        colorGradingPassDependOnToneMappingPass.dstStageMask =
            RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        colorGradingPassDependOnToneMappingPass.srcAccessMask =
            RHI_ACCESS_SHADER_WRITE_BIT | RHI_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        colorGradingPassDependOnToneMappingPass.dstAccessMask =
            RHI_ACCESS_SHADER_READ_BIT | RHI_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        colorGradingPassDependOnToneMappingPass.dependencyFlags = RHI_DEPENDENCY_BY_REGION_BIT;

        RHISubpassDependency& fxaaPassDependOnColorGradingPass = dependencies[5];
        fxaaPassDependOnColorGradingPass.srcSubpass = _main_camera_subpass_color_grading;
        fxaaPassDependOnColorGradingPass.dstSubpass = _main_camera_subpass_fxaa;
        fxaaPassDependOnColorGradingPass.srcStageMask =
            RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        fxaaPassDependOnColorGradingPass.dstStageMask =
            RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        fxaaPassDependOnColorGradingPass.srcAccessMask =
            RHI_ACCESS_SHADER_WRITE_BIT | RHI_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        fxaaPassDependOnColorGradingPass.dstAccessMask =
            RHI_ACCESS_SHADER_READ_BIT | RHI_ACCESS_COLOR_ATTACHMENT_READ_BIT;

        RHISubpassDependency& uiPassDependOnFxaaPass = dependencies[6];
        uiPassDependOnFxaaPass.srcSubpass = _main_camera_subpass_fxaa;
        uiPassDependOnFxaaPass.dstSubpass = _main_camera_subpass_ui;
        uiPassDependOnFxaaPass.srcStageMask =
            RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        uiPassDependOnFxaaPass.dstStageMask =
            RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        uiPassDependOnFxaaPass.srcAccessMask = RHI_ACCESS_SHADER_WRITE_BIT | RHI_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        uiPassDependOnFxaaPass.dstAccessMask = RHI_ACCESS_SHADER_READ_BIT | RHI_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        uiPassDependOnFxaaPass.dependencyFlags = RHI_DEPENDENCY_BY_REGION_BIT;

        RHISubpassDependency& combineUiPassDependOnUiPass = dependencies[7];
        combineUiPassDependOnUiPass.srcSubpass = _main_camera_subpass_ui;
        combineUiPassDependOnUiPass.dstSubpass = _main_camera_subpass_combine_ui;
        combineUiPassDependOnUiPass.srcStageMask =
            RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        combineUiPassDependOnUiPass.dstStageMask =
            RHI_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | RHI_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        combineUiPassDependOnUiPass.srcAccessMask =
            RHI_ACCESS_SHADER_WRITE_BIT | RHI_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        combineUiPassDependOnUiPass.dstAccessMask =
            RHI_ACCESS_SHADER_READ_BIT | RHI_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        combineUiPassDependOnUiPass.dependencyFlags = RHI_DEPENDENCY_BY_REGION_BIT;

        RHIRenderPassCreateInfo renderpassCreateInfo{};
        renderpassCreateInfo.sType = RHI_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderpassCreateInfo.attachmentCount = (sizeof(attachments) / sizeof(attachments[0]));
        renderpassCreateInfo.pAttachments = attachments;
        renderpassCreateInfo.subpassCount = (sizeof(subpasses) / sizeof(subpasses[0]));
        renderpassCreateInfo.pSubpasses = subpasses;
        renderpassCreateInfo.dependencyCount = (sizeof(dependencies) / sizeof(dependencies[0]));
        renderpassCreateInfo.pDependencies = dependencies;

        if (m_RenderCommand->CreateRenderPass(&renderpassCreateInfo, m_FrameBuffer.render_pass) != RHI_SUCCESS)
        {
            throw std::runtime_error("failed to create render pass");
        }
    }

    void MainCameraPass::SetupDescriptorSetLayout()
    {
        m_DescriptorInfos.resize(_layout_type_count);

        // MeshLayout Bind layout = 0 bind = 0
        {
            RHIDescriptorSetLayoutBinding meshMeshLayoutBindings[1];

            // Vertex
            RHIDescriptorSetLayoutBinding& meshMeshLayoutUniformBufferBinding = meshMeshLayoutBindings[0];
            meshMeshLayoutUniformBufferBinding.binding = 0; 
            meshMeshLayoutUniformBufferBinding.descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            meshMeshLayoutUniformBufferBinding.descriptorCount = 1;
            meshMeshLayoutUniformBufferBinding.stageFlags = RHI_SHADER_STAGE_VERTEX_BIT;
            meshMeshLayoutUniformBufferBinding.pImmutableSamplers = NULL;

            RHIDescriptorSetLayoutCreateInfo meshMeshLayoutCreateInfo{};
            meshMeshLayoutCreateInfo.sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            meshMeshLayoutCreateInfo.bindingCount = 1;
            meshMeshLayoutCreateInfo.pBindings = meshMeshLayoutBindings;

            if (m_RenderCommand->CreateDescriptorSetLayout(&meshMeshLayoutCreateInfo, m_DescriptorInfos[_per_mesh].layout) != RHI_SUCCESS)
            {
                throw std::runtime_error("create mesh mesh layout");
            }
        }

        // Mesh Global Bind Layout = 1 bind 0-7
        {
            RHIDescriptorSetLayoutBinding meshGlobalLayoutBindings[8];

            // Vertex Fragment
            RHIDescriptorSetLayoutBinding& meshGlobalLayoutPerframeStorageBufferBinding = meshGlobalLayoutBindings[0];
            meshGlobalLayoutPerframeStorageBufferBinding.binding = 0;
            meshGlobalLayoutPerframeStorageBufferBinding.descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            meshGlobalLayoutPerframeStorageBufferBinding.descriptorCount = 1;
            meshGlobalLayoutPerframeStorageBufferBinding.stageFlags = RHI_SHADER_STAGE_VERTEX_BIT | RHI_SHADER_STAGE_FRAGMENT_BIT;
            meshGlobalLayoutPerframeStorageBufferBinding.pImmutableSamplers = NULL;

            // Vertex 
            RHIDescriptorSetLayoutBinding& meshGlobalLayoutPerdrawcallStorageBufferBinding = meshGlobalLayoutBindings[1];
            meshGlobalLayoutPerdrawcallStorageBufferBinding.binding = 1;
            meshGlobalLayoutPerdrawcallStorageBufferBinding.descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            meshGlobalLayoutPerdrawcallStorageBufferBinding.descriptorCount = 1;
            meshGlobalLayoutPerdrawcallStorageBufferBinding.stageFlags = RHI_SHADER_STAGE_VERTEX_BIT;
            meshGlobalLayoutPerdrawcallStorageBufferBinding.pImmutableSamplers = NULL;

            // Vertex
            RHIDescriptorSetLayoutBinding& meshGlobalLayoutPerDrawcallVertexBlendingStorageBufferBinding = meshGlobalLayoutBindings[2];
            meshGlobalLayoutPerDrawcallVertexBlendingStorageBufferBinding.binding = 2;
            meshGlobalLayoutPerDrawcallVertexBlendingStorageBufferBinding.descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            meshGlobalLayoutPerDrawcallVertexBlendingStorageBufferBinding.descriptorCount = 1;
            meshGlobalLayoutPerDrawcallVertexBlendingStorageBufferBinding.stageFlags = RHI_SHADER_STAGE_VERTEX_BIT;
            meshGlobalLayoutPerDrawcallVertexBlendingStorageBufferBinding.pImmutableSamplers = NULL;

            // Fragment
            RHIDescriptorSetLayoutBinding& meshGlobalLayoutBrdfLUTTextureBinding = meshGlobalLayoutBindings[3];
            meshGlobalLayoutBrdfLUTTextureBinding.binding = 3;
            meshGlobalLayoutBrdfLUTTextureBinding.descriptorType = RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            meshGlobalLayoutBrdfLUTTextureBinding.descriptorCount = 1;
            meshGlobalLayoutBrdfLUTTextureBinding.stageFlags = RHI_SHADER_STAGE_FRAGMENT_BIT;
            meshGlobalLayoutBrdfLUTTextureBinding.pImmutableSamplers = NULL;
        
            // Fragment
            RHIDescriptorSetLayoutBinding& meshGlobalLayoutIrradianceTextureBinding = meshGlobalLayoutBindings[4];
            meshGlobalLayoutIrradianceTextureBinding = meshGlobalLayoutBrdfLUTTextureBinding;
            meshGlobalLayoutIrradianceTextureBinding.binding = 4;

            // Fragment
            RHIDescriptorSetLayoutBinding& meshGlobalLayoutSpecularTextureBinding = meshGlobalLayoutBindings[5];
            meshGlobalLayoutSpecularTextureBinding = meshGlobalLayoutBrdfLUTTextureBinding;
            meshGlobalLayoutSpecularTextureBinding.binding = 5;

            // Fragment
            RHIDescriptorSetLayoutBinding& meshGlobalLayoutPointLightShadowTextureBinding = meshGlobalLayoutBindings[6];
            meshGlobalLayoutPointLightShadowTextureBinding = meshGlobalLayoutBrdfLUTTextureBinding;
            meshGlobalLayoutPointLightShadowTextureBinding.binding = 6;

            // Fragment
            RHIDescriptorSetLayoutBinding& meshGlobalLayoutDirectionalLightShadowTextureBinding = meshGlobalLayoutBindings[7];
            meshGlobalLayoutDirectionalLightShadowTextureBinding = meshGlobalLayoutBrdfLUTTextureBinding;
            meshGlobalLayoutDirectionalLightShadowTextureBinding.binding = 7;

            RHIDescriptorSetLayoutCreateInfo meshGlobalLayoutCreateInfo;
            meshGlobalLayoutCreateInfo.sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            meshGlobalLayoutCreateInfo.pNext = NULL;
            meshGlobalLayoutCreateInfo.flags = 0;
            meshGlobalLayoutCreateInfo.bindingCount = (sizeof(meshGlobalLayoutBindings) / sizeof(meshGlobalLayoutBindings[0]));
            meshGlobalLayoutCreateInfo.pBindings = meshGlobalLayoutBindings;

            if (RHI_SUCCESS != m_RenderCommand->CreateDescriptorSetLayout(&meshGlobalLayoutCreateInfo, m_DescriptorInfos[_mesh_global].layout));
            {
                throw std::runtime_error("create mesh global layout");
            }
        }

        // Mesh Material Bind Layout = 2 bing 0-5
        {
            RHIDescriptorSetLayoutBinding meshMaterialLayoutBindings[6];
        
            RHIDescriptorSetLayoutBinding& meshMaterialLayoutUniformBufferBinding = meshMaterialLayoutBindings[0];
            meshMaterialLayoutUniformBufferBinding.binding = 0;
            meshMaterialLayoutUniformBufferBinding.descriptorType = RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            meshMaterialLayoutUniformBufferBinding.descriptorCount = 1;
            meshMaterialLayoutUniformBufferBinding.stageFlags = RHI_SHADER_STAGE_FRAGMENT_BIT;
            meshMaterialLayoutUniformBufferBinding.pImmutableSamplers = nullptr;

            RHIDescriptorSetLayoutBinding& meshMaterialLayoutBaseColorTextureBinding = meshMaterialLayoutBindings[1];
            meshMaterialLayoutBaseColorTextureBinding.binding = 1;
            meshMaterialLayoutBaseColorTextureBinding.descriptorType = RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            meshMaterialLayoutBaseColorTextureBinding.descriptorCount = 1;
            meshMaterialLayoutBaseColorTextureBinding.stageFlags = RHI_SHADER_STAGE_FRAGMENT_BIT;
            meshMaterialLayoutBaseColorTextureBinding.pImmutableSamplers = nullptr;

            RHIDescriptorSetLayoutBinding& meshMaterialLayoutMetallicRoughessTextureBindings = meshMaterialLayoutBindings[2];
            meshMaterialLayoutMetallicRoughessTextureBindings = meshMaterialLayoutBaseColorTextureBinding;
            meshMaterialLayoutMetallicRoughessTextureBindings.binding = 2;

            RHIDescriptorSetLayoutBinding& meshMaterialLayoutNoramlRoughessTextureBindings = meshMaterialLayoutBindings[3];
            meshMaterialLayoutNoramlRoughessTextureBindings = meshMaterialLayoutBaseColorTextureBinding;
            meshMaterialLayoutNoramlRoughessTextureBindings.binding = 3;

            RHIDescriptorSetLayoutBinding& meshMaterialLayoutOcclusionTextureBinding = meshMaterialLayoutBindings[4];
            meshMaterialLayoutOcclusionTextureBinding = meshMaterialLayoutBaseColorTextureBinding;
            meshMaterialLayoutOcclusionTextureBinding.binding = 4;

            RHIDescriptorSetLayoutBinding& meshMaterialLayoutEmissiveTextureBinding = meshMaterialLayoutBindings[5];
            meshMaterialLayoutEmissiveTextureBinding = meshMaterialLayoutBaseColorTextureBinding;
            meshMaterialLayoutEmissiveTextureBinding.binding = 5;

            RHIDescriptorSetLayoutCreateInfo meshMaterialLayoutCreateInfo;
            meshMaterialLayoutCreateInfo.sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            meshMaterialLayoutCreateInfo.pNext = NULL;
            meshMaterialLayoutCreateInfo.flags = 0;
            meshMaterialLayoutCreateInfo.bindingCount = 6;
            meshMaterialLayoutCreateInfo.pBindings = meshMaterialLayoutBindings;

            if (m_RenderCommand->CreateDescriptorSetLayout(&meshMaterialLayoutCreateInfo, m_DescriptorInfos[_mesh_per_material].layout) != RHI_SUCCESS)
            {
                throw std::runtime_error("create mesh material layout");
            }
        }

        // sky box
        {
            RHIDescriptorSetLayoutBinding skyboxLayoutBindings[2];

            RHIDescriptorSetLayoutBinding& skyboxLayoutPerframeStorageBufferBinding = skyboxLayoutBindings[0];
            skyboxLayoutPerframeStorageBufferBinding.binding = 0;
            skyboxLayoutPerframeStorageBufferBinding.descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            skyboxLayoutPerframeStorageBufferBinding.descriptorCount = 1;
            skyboxLayoutPerframeStorageBufferBinding.stageFlags = RHI_SHADER_STAGE_VERTEX_BIT;
            skyboxLayoutPerframeStorageBufferBinding.pImmutableSamplers = NULL;

            RHIDescriptorSetLayoutBinding& skyboxLayoutSpecularTextureBinding = skyboxLayoutBindings[1];
            skyboxLayoutSpecularTextureBinding.binding = 1;
            skyboxLayoutSpecularTextureBinding.descriptorType = RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            skyboxLayoutSpecularTextureBinding.descriptorCount = 1;
            skyboxLayoutSpecularTextureBinding.stageFlags = RHI_SHADER_STAGE_FRAGMENT_BIT;
            skyboxLayoutSpecularTextureBinding.pImmutableSamplers = NULL;
        
            RHIDescriptorSetLayoutCreateInfo skyboxLayoutCreateInfo{};
            skyboxLayoutCreateInfo.sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            skyboxLayoutCreateInfo.bindingCount = 2;
            skyboxLayoutCreateInfo.pBindings = skyboxLayoutBindings;

            if (RHI_SUCCESS != m_RenderCommand->CreateDescriptorSetLayout(&skyboxLayoutCreateInfo, m_DescriptorInfos[_skybox].layout))
            {
                throw std::runtime_error("create skybox layout");
            }
        }

        // axis
        {
            RHIDescriptorSetLayoutBinding axisLayoutBindings[2];

            RHIDescriptorSetLayoutBinding& axisLayoutPerframeStorageBufferBinding = axisLayoutBindings[0];
            axisLayoutPerframeStorageBufferBinding.binding = 0;
            axisLayoutPerframeStorageBufferBinding.descriptorCount = 1;
            axisLayoutPerframeStorageBufferBinding.descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            axisLayoutPerframeStorageBufferBinding.stageFlags = RHI_SHADER_STAGE_VERTEX_BIT;
            axisLayoutPerframeStorageBufferBinding.pImmutableSamplers = NULL;

            RHIDescriptorSetLayoutBinding& axisLayoutStorageBufferBinding = axisLayoutBindings[1];
            axisLayoutStorageBufferBinding.binding = 1;
            axisLayoutStorageBufferBinding.descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            axisLayoutStorageBufferBinding.descriptorCount = 1;
            axisLayoutStorageBufferBinding.stageFlags = RHI_SHADER_STAGE_VERTEX_BIT;
            axisLayoutStorageBufferBinding.pImmutableSamplers = NULL;

            RHIDescriptorSetLayoutCreateInfo axisLayoutCreateInfo{};
            axisLayoutCreateInfo.sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            axisLayoutCreateInfo.bindingCount = 2;
            axisLayoutCreateInfo.pBindings = axisLayoutBindings;
        
            if (RHI_SUCCESS != m_RenderCommand->CreateDescriptorSetLayout(&axisLayoutCreateInfo, m_DescriptorInfos[_axis].layout))
            {
                throw std::runtime_error("create axis layout");
            }
        }
    
        // gbuffer
        {
            RHIDescriptorSetLayoutBinding gbufferLightingGlobalLayoutBindings[4];

            RHIDescriptorSetLayoutBinding& gbufferNormalGlobalLayoutInputAttachmentBinding = gbufferLightingGlobalLayoutBindings[0];
            gbufferNormalGlobalLayoutInputAttachmentBinding.binding = 0;
            gbufferNormalGlobalLayoutInputAttachmentBinding.descriptorType = RHI_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            gbufferNormalGlobalLayoutInputAttachmentBinding.descriptorCount = 1;
            gbufferNormalGlobalLayoutInputAttachmentBinding.stageFlags = RHI_SHADER_STAGE_FRAGMENT_BIT;

            RHIDescriptorSetLayoutBinding&
                gbufferMetallicRoughnessShadingmodeidGlobalLayoutInputAttachmentBinding =
                gbufferLightingGlobalLayoutBindings[1];
            gbufferMetallicRoughnessShadingmodeidGlobalLayoutInputAttachmentBinding.binding = 1;
            gbufferMetallicRoughnessShadingmodeidGlobalLayoutInputAttachmentBinding.descriptorType = RHI_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            gbufferMetallicRoughnessShadingmodeidGlobalLayoutInputAttachmentBinding.descriptorCount = 1;
            gbufferMetallicRoughnessShadingmodeidGlobalLayoutInputAttachmentBinding.stageFlags = RHI_SHADER_STAGE_FRAGMENT_BIT;
        
            RHIDescriptorSetLayoutBinding& gbufferAlbedoGlobalLayoutInputAttachmentBinding =
                gbufferLightingGlobalLayoutBindings[2];
            gbufferAlbedoGlobalLayoutInputAttachmentBinding.binding = 2;
            gbufferAlbedoGlobalLayoutInputAttachmentBinding.descriptorType = RHI_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            gbufferAlbedoGlobalLayoutInputAttachmentBinding.descriptorCount = 1;
            gbufferAlbedoGlobalLayoutInputAttachmentBinding.stageFlags = RHI_SHADER_STAGE_FRAGMENT_BIT;
        
            RHIDescriptorSetLayoutBinding& gbufferDepthGlobalLayoutInputAttachmentBinding =
                gbufferLightingGlobalLayoutBindings[3];
            gbufferDepthGlobalLayoutInputAttachmentBinding.binding = 3;
            gbufferDepthGlobalLayoutInputAttachmentBinding.descriptorType = RHI_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            gbufferDepthGlobalLayoutInputAttachmentBinding.descriptorCount = 1;
            gbufferDepthGlobalLayoutInputAttachmentBinding.stageFlags = RHI_SHADER_STAGE_FRAGMENT_BIT;

            RHIDescriptorSetLayoutCreateInfo gbufferLightingGlobalLayoutCreateInfo;
            gbufferLightingGlobalLayoutCreateInfo.sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            gbufferLightingGlobalLayoutCreateInfo.pNext = NULL;
            gbufferLightingGlobalLayoutCreateInfo.flags = 0;
            gbufferLightingGlobalLayoutCreateInfo.bindingCount =
                sizeof(gbufferLightingGlobalLayoutBindings) / sizeof(gbufferLightingGlobalLayoutBindings[0]);
            gbufferLightingGlobalLayoutCreateInfo.pBindings = gbufferLightingGlobalLayoutBindings;

            if (RHI_SUCCESS != m_RenderCommand->CreateDescriptorSetLayout(&gbufferLightingGlobalLayoutCreateInfo, m_DescriptorInfos[_deferred_lighting].layout))
            {
                throw std::runtime_error("create deferred lighting global layout");
            }

        }

    }

    void MainCameraPass::SetupPipelines()
    {
        m_RenderPipelines.resize(_render_pipeline_type_count);

        // mesh gbuffer
        {
            RHIDescriptorSetLayout* descriptorLayouts[3] = { m_DescriptorInfos[_mesh_global].layout,
                                                            m_DescriptorInfos[_per_mesh].layout,
                                                            m_DescriptorInfos[_mesh_per_material].layout };

            RHIPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
            pipelineLayoutCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutCreateInfo.setLayoutCount = 3;
            pipelineLayoutCreateInfo.pSetLayouts = descriptorLayouts;

            if (RHI_SUCCESS != m_RenderCommand->CreatePipelineLayout(&pipelineLayoutCreateInfo, m_RenderPipelines[_render_pipeline_type_mesh_gbuffer].layout))
            {
                throw std::runtime_error("create mesh gbuffer pipeline layout");
            }

            RHIShader* vertShaderModule = m_RenderCommand->CreateShaderModule(MESH_VERT);
            RHIShader* fragShaderModule = m_RenderCommand->CreateShaderModule(MESH_GBUFFER_FRAG);

            RHIPipelineShaderStageCreateInfo vertPipelineShaderStageCreateInfo{};
            vertPipelineShaderStageCreateInfo.sType = RHI_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            vertPipelineShaderStageCreateInfo.stage = RHI_SHADER_STAGE_VERTEX_BIT;
            vertPipelineShaderStageCreateInfo.module = vertShaderModule;
            vertPipelineShaderStageCreateInfo.pName = "main";

            RHIPipelineShaderStageCreateInfo fragPipelineShaderStageCreateInfo{};
            fragPipelineShaderStageCreateInfo.sType = RHI_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            fragPipelineShaderStageCreateInfo.stage = RHI_SHADER_STAGE_FRAGMENT_BIT;
            fragPipelineShaderStageCreateInfo.module = fragShaderModule;
            fragPipelineShaderStageCreateInfo.pName = "main";

            RHIPipelineShaderStageCreateInfo shaderStages[] = { vertPipelineShaderStageCreateInfo ,fragPipelineShaderStageCreateInfo };

            auto vertexBindingDescriptions = MeshVertex::GetBindingDescriptions();
            auto vertexAttributeDescriptions = MeshVertex::GetAttributeDescriptions();

            RHIPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
            vertexInputStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputStateCreateInfo.vertexBindingDescriptionCount = vertexBindingDescriptions.size();
            vertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexBindingDescriptions[0];
            vertexInputStateCreateInfo.vertexAttributeDescriptionCount = vertexAttributeDescriptions.size();
            vertexInputStateCreateInfo.pVertexAttributeDescriptions = &vertexAttributeDescriptions[0];

            RHIPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
            inputAssemblyCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssemblyCreateInfo.topology = RHI_PRIMITIVE_TOPOLOGY_LINE_LIST;
            inputAssemblyCreateInfo.primitiveRestartEnable = RHI_FALSE;

            RHIPipelineViewportStateCreateInfo viewportStateCreateInfo{};
            viewportStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportStateCreateInfo.viewportCount = 1;
            viewportStateCreateInfo.pViewports = m_RenderCommand->GetSwapchainInfo().viewport;
            viewportStateCreateInfo.scissorCount = 1;
            viewportStateCreateInfo.pScissors = m_RenderCommand->GetSwapchainInfo().scissor;

            RHIPipelineRasterizationStateCreateInfo rasteriazationStateCreateInfo{};
            rasteriazationStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasteriazationStateCreateInfo.depthClampEnable = RHI_FALSE;
            rasteriazationStateCreateInfo.rasterizerDiscardEnable = RHI_FALSE;
            rasteriazationStateCreateInfo.polygonMode = RHI_POLYGON_MODE_FILL;
            rasteriazationStateCreateInfo.lineWidth = 1.0f;
            rasteriazationStateCreateInfo.cullMode = RHI_CULL_MODE_BACK_BIT;
            rasteriazationStateCreateInfo.frontFace = RHI_FRONT_FACE_COUNTER_CLOCKWISE;
            rasteriazationStateCreateInfo.depthBiasEnable = FALSE;
            rasteriazationStateCreateInfo.depthBiasConstantFactor = 0.0f;
            rasteriazationStateCreateInfo.depthBiasClamp = 0.0f;
            rasteriazationStateCreateInfo.depthBiasSlopeFactor = 0.0f;

            RHIPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
            multisampleStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampleStateCreateInfo.sampleShadingEnable = RHI_FALSE;
            multisampleStateCreateInfo.rasterizationSamples = RHI_SAMPLE_COUNT_1_BIT;

            RHIPipelineColorBlendAttachmentState colorBlendAttachments[3] = {};
            colorBlendAttachments[0].colorWriteMask = RHI_COLOR_COMPONENT_R_BIT | RHI_COLOR_COMPONENT_G_BIT |
                                                      RHI_COLOR_COMPONENT_B_BIT | RHI_COLOR_COMPONENT_A_BIT;
            colorBlendAttachments[0].blendEnable = RHI_FALSE;
            colorBlendAttachments[0].srcColorBlendFactor = RHI_BLEND_FACTOR_ONE;
            colorBlendAttachments[0].dstColorBlendFactor = RHI_BLEND_FACTOR_ZERO;
            colorBlendAttachments[0].colorBlendOp = RHI_BLEND_OP_ADD;
            colorBlendAttachments[0].srcAlphaBlendFactor = RHI_BLEND_FACTOR_ONE;
            colorBlendAttachments[0].dstAlphaBlendFactor = RHI_BLEND_FACTOR_ZERO;
            colorBlendAttachments[0].alphaBlendOp = RHI_BLEND_OP_ADD;

            colorBlendAttachments[1].colorWriteMask = RHI_COLOR_COMPONENT_R_BIT | RHI_COLOR_COMPONENT_G_BIT |
                                                      RHI_COLOR_COMPONENT_B_BIT | RHI_COLOR_COMPONENT_A_BIT;
            colorBlendAttachments[1].blendEnable = RHI_FALSE;
            colorBlendAttachments[1].srcColorBlendFactor = RHI_BLEND_FACTOR_ONE;
            colorBlendAttachments[1].dstColorBlendFactor = RHI_BLEND_FACTOR_ZERO;
            colorBlendAttachments[1].colorBlendOp = RHI_BLEND_OP_ADD;
            colorBlendAttachments[1].srcAlphaBlendFactor = RHI_BLEND_FACTOR_ONE;
            colorBlendAttachments[1].dstAlphaBlendFactor = RHI_BLEND_FACTOR_ZERO;
            colorBlendAttachments[1].alphaBlendOp = RHI_BLEND_OP_ADD;

            colorBlendAttachments[2].colorWriteMask = RHI_COLOR_COMPONENT_R_BIT | RHI_COLOR_COMPONENT_G_BIT |
                RHI_COLOR_COMPONENT_B_BIT | RHI_COLOR_COMPONENT_A_BIT;
            colorBlendAttachments[2].blendEnable = RHI_FALSE;
            colorBlendAttachments[2].srcColorBlendFactor = RHI_BLEND_FACTOR_ONE;
            colorBlendAttachments[2].dstColorBlendFactor = RHI_BLEND_FACTOR_ZERO;
            colorBlendAttachments[2].colorBlendOp = RHI_BLEND_OP_ADD;
            colorBlendAttachments[2].srcAlphaBlendFactor = RHI_BLEND_FACTOR_ONE;
            colorBlendAttachments[2].dstAlphaBlendFactor = RHI_BLEND_FACTOR_ZERO;
            colorBlendAttachments[2].alphaBlendOp = RHI_BLEND_OP_ADD;

            RHIPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
            colorBlendStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlendStateCreateInfo.logicOpEnable = RHI_FALSE;
            colorBlendStateCreateInfo.logicOp = RHI_LOGIC_OP_COPY;
            colorBlendStateCreateInfo.attachmentCount = sizeof(colorBlendAttachments) / sizeof(colorBlendAttachments[0]);
            colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
            colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
            colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
            colorBlendStateCreateInfo.blendConstants[3] = 0.0f;

            RHIPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
            depthStencilCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencilCreateInfo.depthTestEnable = RHI_TRUE;
            depthStencilCreateInfo.depthWriteEnable = RHI_TRUE;
            depthStencilCreateInfo.depthCompareOp = RHI_COMPARE_OP_LESS;
            depthStencilCreateInfo.depthBoundsTestEnable = RHI_FALSE;
            depthStencilCreateInfo.stencilTestEnable = RHI_FALSE;

            RHIDynamicState     dynamicStates[] = { RHI_DYNAMIC_STATE_VIEWPORT,RHI_DYNAMIC_STATE_SCISSOR };
            RHIPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
            dynamicStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicStateCreateInfo.dynamicStateCount = 2;
            dynamicStateCreateInfo.pDynamicStates = dynamicStates;

            RHIGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = RHI_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = 2;
            pipelineInfo.pStages = shaderStages;
            pipelineInfo.pVertexInputState = &vertexInputStateCreateInfo;
            pipelineInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
            pipelineInfo.pViewportState = &viewportStateCreateInfo;
            pipelineInfo.pRasterizationState = &rasteriazationStateCreateInfo;
            pipelineInfo.pMultisampleState = &multisampleStateCreateInfo;
            pipelineInfo.pColorBlendState = &colorBlendStateCreateInfo;
            pipelineInfo.pDepthStencilState = &depthStencilCreateInfo;
            pipelineInfo.layout = m_RenderPipelines[_render_pipeline_type_mesh_gbuffer].layout;
            pipelineInfo.renderPass = m_FrameBuffer.render_pass;
            pipelineInfo.subpass = _main_camera_subpass_basepass;
            pipelineInfo.basePipelineHandle = RHI_NULL_HANDLE;
            pipelineInfo.pDynamicState = &dynamicStateCreateInfo;

            if (RHI_SUCCESS != m_RenderCommand->CreateGraphicsPipelines(RHI_NULL_HANDLE,
                1,
                &pipelineInfo,
                m_RenderPipelines[_render_pipeline_type_mesh_gbuffer].pipeline))
            {
                throw std::runtime_error("create mesh gbuffer graphics pipeline");
            }

            m_RenderCommand->DestroyShaderModule(vertShaderModule);
            m_RenderCommand->DestroyShaderModule(fragShaderModule);
        }

        // deferred lighting
        {
            RHIDescriptorSetLayout* descriptorsetLayouts[3] = { m_DescriptorInfos[_mesh_global].layout,m_DescriptorInfos[_deferred_lighting].layout,m_DescriptorInfos[_skybox].layout };

            RHIPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
            pipelineLayoutCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutCreateInfo.setLayoutCount = sizeof(descriptorsetLayouts) / sizeof(descriptorsetLayouts[0]);
            pipelineLayoutCreateInfo.pSetLayouts = descriptorsetLayouts;

            if (RHI_SUCCESS != m_RenderCommand->CreatePipelineLayout(&pipelineLayoutCreateInfo, m_RenderPipelines[_render_pipeline_type_deferred_lighting].layout));
            {
                throw std::runtime_error("create deferred lighting pipeline layout");
            }

            RHIShader* vertShaderModule = m_RenderCommand->CreateShaderModule(DEFERRED_LIGHTING_VERT);
            RHIShader* fragShaderModule = m_RenderCommand->CreateShaderModule(DEFERRED_LIGHTING_FRAG);

            RHIPipelineShaderStageCreateInfo vertPipelineShaderStageCreateInfo{};
            vertPipelineShaderStageCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertPipelineShaderStageCreateInfo.stage = RHI_SHADER_STAGE_VERTEX_BIT;
            vertPipelineShaderStageCreateInfo.module = vertShaderModule;
            vertPipelineShaderStageCreateInfo.pName = "main";

            RHIPipelineShaderStageCreateInfo fragPipelineShaderStageCreateInfo{};
            fragPipelineShaderStageCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragPipelineShaderStageCreateInfo.stage = RHI_SHADER_STAGE_FRAGMENT_BIT;
            fragPipelineShaderStageCreateInfo.module = fragShaderModule;
            fragPipelineShaderStageCreateInfo.pName = "main";

            RHIPipelineShaderStageCreateInfo shaderStages[] =
            {
                vertPipelineShaderStageCreateInfo,
                fragPipelineShaderStageCreateInfo
            };

            auto vertexBindingDescriptions = MeshVertex::GetBindingDescriptions();
            auto vertexAttributeDescriptions = MeshVertex::GetAttributeDescriptions();

            RHIPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
            vertexInputStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
            vertexInputStateCreateInfo.pVertexBindingDescriptions = NULL;
            vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
            vertexInputStateCreateInfo.pVertexAttributeDescriptions = NULL;

            RHIPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
            inputAssemblyCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssemblyCreateInfo.topology = RHI_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssemblyCreateInfo.primitiveRestartEnable = RHI_FALSE;

            RHIPipelineViewportStateCreateInfo viewprotStateCreateInfo{};
            viewprotStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewprotStateCreateInfo.viewportCount = 1;
            viewprotStateCreateInfo.pViewports = m_RenderCommand->GetSwapchainInfo().viewport;
            viewprotStateCreateInfo.scissorCount = 1;
            viewprotStateCreateInfo.pScissors = m_RenderCommand->GetSwapchainInfo().scissor;

            RHIPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
            rasterizationStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizationStateCreateInfo.depthClampEnable = RHI_FALSE;
            rasterizationStateCreateInfo.rasterizerDiscardEnable = RHI_FALSE;
            rasterizationStateCreateInfo.polygonMode = RHI_POLYGON_MODE_FILL;
            rasterizationStateCreateInfo.lineWidth = 1.0f;
            rasterizationStateCreateInfo.cullMode = RHI_CULL_MODE_BACK_BIT;
            rasterizationStateCreateInfo.frontFace = RHI_FRONT_FACE_CLOCKWISE;
            rasterizationStateCreateInfo.depthBiasEnable = RHI_FALSE;
            rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
            rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
            rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;

            RHIPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
            multisampleStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampleStateCreateInfo.sampleShadingEnable = RHI_FALSE;
            multisampleStateCreateInfo.rasterizationSamples = RHI_SAMPLE_COUNT_1_BIT;

            RHIPipelineColorBlendAttachmentState colorBlendAttachments[1] = {};
            colorBlendAttachments[0].colorWriteMask = RHI_COLOR_COMPONENT_R_BIT | RHI_COLOR_COMPONENT_G_BIT |
                RHI_COLOR_COMPONENT_B_BIT | RHI_COLOR_COMPONENT_A_BIT;
            colorBlendAttachments[0].blendEnable = RHI_FALSE;
            colorBlendAttachments[0].srcColorBlendFactor = RHI_BLEND_FACTOR_ONE;
            colorBlendAttachments[0].dstColorBlendFactor = RHI_BLEND_FACTOR_ONE;
            colorBlendAttachments[0].colorBlendOp = RHI_BLEND_OP_ADD;
            colorBlendAttachments[0].srcAlphaBlendFactor = RHI_BLEND_FACTOR_ONE;
            colorBlendAttachments[0].dstAlphaBlendFactor = RHI_BLEND_FACTOR_ONE;
            colorBlendAttachments[0].alphaBlendOp = RHI_BLEND_OP_ADD;

            RHIPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
            colorBlendStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlendStateCreateInfo.logicOpEnable = RHI_FALSE;
            colorBlendStateCreateInfo.logicOp = RHI_LOGIC_OP_COPY;
            colorBlendStateCreateInfo.attachmentCount =
                sizeof(colorBlendAttachments) / sizeof(colorBlendAttachments[0]);
            colorBlendStateCreateInfo.pAttachments = &colorBlendAttachments[0];
            colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
            colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
            colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
            colorBlendStateCreateInfo.blendConstants[3] = 0.0f;

            RHIPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
            depthStencilCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencilCreateInfo.depthTestEnable = RHI_FALSE;
            depthStencilCreateInfo.depthWriteEnable = RHI_FALSE;
            depthStencilCreateInfo.depthCompareOp = RHI_COMPARE_OP_ALWAYS;
            depthStencilCreateInfo.depthBoundsTestEnable = RHI_FALSE;
            depthStencilCreateInfo.stencilTestEnable = RHI_FALSE;

            RHIDynamicState                   dynamicStates[] = { RHI_DYNAMIC_STATE_VIEWPORT, RHI_DYNAMIC_STATE_SCISSOR };
            RHIPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
            dynamicStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicStateCreateInfo.dynamicStateCount = 2;
            dynamicStateCreateInfo.pDynamicStates = dynamicStates;

            RHIGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = RHI_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = 2;
            pipelineInfo.pStages = shaderStages;
            pipelineInfo.pVertexInputState = &vertexInputStateCreateInfo;
            pipelineInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
            pipelineInfo.pViewportState = &viewprotStateCreateInfo;
            pipelineInfo.pRasterizationState = &rasterizationStateCreateInfo;
            pipelineInfo.pMultisampleState = &multisampleStateCreateInfo;
            pipelineInfo.pColorBlendState = &colorBlendStateCreateInfo;
            pipelineInfo.pDepthStencilState = &depthStencilCreateInfo;
            pipelineInfo.layout = m_RenderPipelines[_render_pipeline_type_deferred_lighting].layout;
            pipelineInfo.renderPass = m_FrameBuffer.render_pass;
            pipelineInfo.subpass = _main_camera_subpass_deferred_lighting;
            pipelineInfo.basePipelineHandle = RHI_NULL_HANDLE;
            pipelineInfo.pDynamicState = &dynamicStateCreateInfo;

            if (RHI_SUCCESS != m_RenderCommand->CreateGraphicsPipelines(RHI_NULL_HANDLE,
                1,
                &pipelineInfo,
                m_RenderPipelines[_render_pipeline_type_deferred_lighting].pipeline))
            {
                throw std::runtime_error("create deferred lighting graphics pipeline");
            }

            m_RenderCommand->DestroyShaderModule(vertShaderModule);
            m_RenderCommand->DestroyShaderModule(fragShaderModule);
        }

        // mesh lighting
        {
            RHIDescriptorSetLayout* descriptorset_layouts[3] = { m_DescriptorInfos[_mesh_global].layout,
                                                                         m_DescriptorInfos[_per_mesh].layout,
                                                                         m_DescriptorInfos[_mesh_per_material].layout };
            RHIPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
            pipelineLayoutCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutCreateInfo.setLayoutCount = 3;
            pipelineLayoutCreateInfo.pSetLayouts = descriptorset_layouts;

            if (m_RenderCommand->CreatePipelineLayout(&pipelineLayoutCreateInfo, m_RenderPipelines[_render_pipeline_type_mesh_lighting].layout) != RHI_SUCCESS)
            {
                throw std::runtime_error("create mesh lighting pipeline layout");
            }

            RHIShader* vertShaderModule = m_RenderCommand->CreateShaderModule(MESH_VERT);
            RHIShader* fragShaderModule = m_RenderCommand->CreateShaderModule(MESH_FRAG);

            RHIPipelineShaderStageCreateInfo vertPipelineShaderStageCreateInfo{};
            vertPipelineShaderStageCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertPipelineShaderStageCreateInfo.stage = RHI_SHADER_STAGE_VERTEX_BIT;
            vertPipelineShaderStageCreateInfo.module = vertShaderModule;
            vertPipelineShaderStageCreateInfo.pName = "main";

            RHIPipelineShaderStageCreateInfo fragPipelineShaderStageCreateInfo{};
            fragPipelineShaderStageCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragPipelineShaderStageCreateInfo.stage = RHI_SHADER_STAGE_FRAGMENT_BIT;
            fragPipelineShaderStageCreateInfo.module = fragShaderModule;
            fragPipelineShaderStageCreateInfo.pName = "main";

            RHIPipelineShaderStageCreateInfo shaderStages[] = { vertPipelineShaderStageCreateInfo,
                                                               fragPipelineShaderStageCreateInfo };

            auto                                 vertexBindingDescriptions = MeshVertex::GetBindingDescriptions();
            auto                                 vertexAttributeDescriptions = MeshVertex::GetAttributeDescriptions();
            RHIPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
            vertexInputStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputStateCreateInfo.vertexBindingDescriptionCount = vertexBindingDescriptions.size();
            vertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexBindingDescriptions[0];
            vertexInputStateCreateInfo.vertexAttributeDescriptionCount = vertexAttributeDescriptions.size();
            vertexInputStateCreateInfo.pVertexAttributeDescriptions = &vertexAttributeDescriptions[0];

            RHIPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
            inputAssemblyCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssemblyCreateInfo.topology = RHI_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssemblyCreateInfo.primitiveRestartEnable = RHI_FALSE;

            RHIPipelineViewportStateCreateInfo viewportStateCreateInfo{};
            viewportStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportStateCreateInfo.viewportCount = 1;
            viewportStateCreateInfo.pViewports = m_RenderCommand->GetSwapchainInfo().viewport;
            viewportStateCreateInfo.scissorCount = 1;
            viewportStateCreateInfo.pScissors = m_RenderCommand->GetSwapchainInfo().scissor;

            RHIPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
            rasterizationStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizationStateCreateInfo.depthClampEnable = RHI_FALSE;
            rasterizationStateCreateInfo.rasterizerDiscardEnable = RHI_FALSE;
            rasterizationStateCreateInfo.polygonMode = RHI_POLYGON_MODE_FILL;
            rasterizationStateCreateInfo.lineWidth = 1.0f;
            rasterizationStateCreateInfo.cullMode = RHI_CULL_MODE_BACK_BIT;
            rasterizationStateCreateInfo.frontFace = RHI_FRONT_FACE_COUNTER_CLOCKWISE;
            rasterizationStateCreateInfo.depthBiasEnable = RHI_FALSE;
            rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
            rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
            rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;

            RHIPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
            multisampleStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampleStateCreateInfo.sampleShadingEnable = RHI_FALSE;
            multisampleStateCreateInfo.rasterizationSamples = RHI_SAMPLE_COUNT_1_BIT;

            RHIPipelineColorBlendAttachmentState colorBlendAttachments[1] = {};
            colorBlendAttachments[0].colorWriteMask = RHI_COLOR_COMPONENT_R_BIT | RHI_COLOR_COMPONENT_G_BIT |
                RHI_COLOR_COMPONENT_B_BIT | RHI_COLOR_COMPONENT_A_BIT;
            colorBlendAttachments[0].blendEnable = RHI_FALSE;
            colorBlendAttachments[0].srcColorBlendFactor = RHI_BLEND_FACTOR_ONE;
            colorBlendAttachments[0].dstColorBlendFactor = RHI_BLEND_FACTOR_ONE;
            colorBlendAttachments[0].colorBlendOp = RHI_BLEND_OP_ADD;
            colorBlendAttachments[0].srcAlphaBlendFactor = RHI_BLEND_FACTOR_ONE;
            colorBlendAttachments[0].dstAlphaBlendFactor = RHI_BLEND_FACTOR_ONE;
            colorBlendAttachments[0].alphaBlendOp = RHI_BLEND_OP_ADD;

            RHIPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
            colorBlendStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlendStateCreateInfo.logicOpEnable = RHI_FALSE;
            colorBlendStateCreateInfo.logicOp = RHI_LOGIC_OP_COPY;
            colorBlendStateCreateInfo.attachmentCount =
                sizeof(colorBlendAttachments) / sizeof(colorBlendAttachments[0]);
            colorBlendStateCreateInfo.pAttachments = &colorBlendAttachments[0];
            colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
            colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
            colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
            colorBlendStateCreateInfo.blendConstants[3] = 0.0f;

            RHIPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{};
            depthStencilStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencilStateCreateInfo.depthTestEnable = RHI_TRUE;
            depthStencilStateCreateInfo.depthWriteEnable = RHI_TRUE;
            depthStencilStateCreateInfo.depthCompareOp = RHI_COMPARE_OP_LESS;
            depthStencilStateCreateInfo.depthBoundsTestEnable = RHI_FALSE;
            depthStencilStateCreateInfo.stencilTestEnable = RHI_FALSE;

            RHIDynamicState                   dynamicStates[] = { RHI_DYNAMIC_STATE_VIEWPORT, RHI_DYNAMIC_STATE_SCISSOR };
            RHIPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
            dynamicStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicStateCreateInfo.dynamicStateCount = 2;
            dynamicStateCreateInfo.pDynamicStates = dynamicStates;

            RHIGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = RHI_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = 2;
            pipelineInfo.pStages = shaderStages;
            pipelineInfo.pVertexInputState = &vertexInputStateCreateInfo;
            pipelineInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
            pipelineInfo.pViewportState = &viewportStateCreateInfo;
            pipelineInfo.pRasterizationState = &rasterizationStateCreateInfo;
            pipelineInfo.pMultisampleState = &multisampleStateCreateInfo;
            pipelineInfo.pColorBlendState = &colorBlendStateCreateInfo;
            pipelineInfo.pDepthStencilState = &depthStencilStateCreateInfo;
            pipelineInfo.layout = m_RenderPipelines[_render_pipeline_type_mesh_lighting].layout;
            pipelineInfo.renderPass = m_FrameBuffer.render_pass;
            pipelineInfo.subpass = _main_camera_subpass_forward_lighting;
            pipelineInfo.basePipelineHandle = RHI_NULL_HANDLE;
            pipelineInfo.pDynamicState = &dynamicStateCreateInfo;

            if (m_RenderCommand->CreateGraphicsPipelines(RHI_NULL_HANDLE,
                1,
                &pipelineInfo,
                m_RenderPipelines[_render_pipeline_type_mesh_lighting].pipeline) !=
                RHI_SUCCESS)
            {
                throw std::runtime_error("create mesh lighting graphics pipeline");
            }

            m_RenderCommand->DestroyShaderModule(vertShaderModule);
            m_RenderCommand->DestroyShaderModule(fragShaderModule);
        }

        // skybox
        {
            RHIDescriptorSetLayout* descriptorset_layouts[1] = { m_DescriptorInfos[_skybox].layout };
            RHIPipelineLayoutCreateInfo pipeline_layout_create_info{};
            pipeline_layout_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_create_info.setLayoutCount = 1;
            pipeline_layout_create_info.pSetLayouts = descriptorset_layouts;

            if (m_RenderCommand->CreatePipelineLayout(&pipeline_layout_create_info, m_RenderPipelines[_render_pipeline_type_skybox].layout) != RHI_SUCCESS)
            {
                throw std::runtime_error("create skybox pipeline layout");
            }

            RHIShader* vertShaderModule = m_RenderCommand->CreateShaderModule(SKYBOX_VERT);
            RHIShader* fragShaderModule = m_RenderCommand->CreateShaderModule(SKYBOX_FRAG);

            RHIPipelineShaderStageCreateInfo vertPipelineShaderStageCreateInfo{};
            vertPipelineShaderStageCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertPipelineShaderStageCreateInfo.stage = RHI_SHADER_STAGE_VERTEX_BIT;
            vertPipelineShaderStageCreateInfo.module = vertShaderModule;
            vertPipelineShaderStageCreateInfo.pName = "main";
            // vertPipelineShaderStageCreateInfo.pSpecializationInfo

            RHIPipelineShaderStageCreateInfo fragPipelineShaderStageCreateInfo{};
            fragPipelineShaderStageCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragPipelineShaderStageCreateInfo.stage = RHI_SHADER_STAGE_FRAGMENT_BIT;
            fragPipelineShaderStageCreateInfo.module = fragShaderModule;
            fragPipelineShaderStageCreateInfo.pName = "main";

            RHIPipelineShaderStageCreateInfo shader_stages[] = { vertPipelineShaderStageCreateInfo,
                                                               fragPipelineShaderStageCreateInfo };

            auto                                 vertexBindingDescriptions = MeshVertex::GetBindingDescriptions();
            auto                                 vertexAttributeDescriptions = MeshVertex::GetAttributeDescriptions();
            RHIPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo{};
            VertexInputStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            VertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
            VertexInputStateCreateInfo.pVertexBindingDescriptions = NULL;
            VertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
            VertexInputStateCreateInfo.pVertexAttributeDescriptions = NULL;

            RHIPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
            inputAssemblyCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssemblyCreateInfo.topology = RHI_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssemblyCreateInfo.primitiveRestartEnable = RHI_FALSE;

            RHIPipelineViewportStateCreateInfo viewportStateCreateInfo{};
            viewportStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportStateCreateInfo.viewportCount = 1;
            viewportStateCreateInfo.pViewports = m_RenderCommand->GetSwapchainInfo().viewport;
            viewportStateCreateInfo.scissorCount = 1;
            viewportStateCreateInfo.pScissors = m_RenderCommand->GetSwapchainInfo().scissor;

            RHIPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
            rasterizationStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizationStateCreateInfo.depthClampEnable = RHI_FALSE;
            rasterizationStateCreateInfo.rasterizerDiscardEnable = RHI_FALSE;
            rasterizationStateCreateInfo.polygonMode = RHI_POLYGON_MODE_FILL;
            rasterizationStateCreateInfo.lineWidth = 1.0f;
            rasterizationStateCreateInfo.cullMode = RHI_CULL_MODE_BACK_BIT;
            rasterizationStateCreateInfo.frontFace = RHI_FRONT_FACE_COUNTER_CLOCKWISE;
            rasterizationStateCreateInfo.depthBiasEnable = RHI_FALSE;
            rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
            rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
            rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;

            RHIPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
            multisampleStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampleStateCreateInfo.sampleShadingEnable = RHI_FALSE;
            multisampleStateCreateInfo.rasterizationSamples = RHI_SAMPLE_COUNT_1_BIT;

            RHIPipelineColorBlendAttachmentState colorBlendAttachments[1] = {};
            colorBlendAttachments[0].colorWriteMask = RHI_COLOR_COMPONENT_R_BIT | RHI_COLOR_COMPONENT_G_BIT |
                RHI_COLOR_COMPONENT_B_BIT | RHI_COLOR_COMPONENT_A_BIT;
            colorBlendAttachments[0].blendEnable = RHI_FALSE;
            colorBlendAttachments[0].srcColorBlendFactor = RHI_BLEND_FACTOR_ONE;
            colorBlendAttachments[0].dstColorBlendFactor = RHI_BLEND_FACTOR_ZERO;
            colorBlendAttachments[0].colorBlendOp = RHI_BLEND_OP_ADD;
            colorBlendAttachments[0].srcAlphaBlendFactor = RHI_BLEND_FACTOR_ONE;
            colorBlendAttachments[0].dstAlphaBlendFactor = RHI_BLEND_FACTOR_ZERO;
            colorBlendAttachments[0].alphaBlendOp = RHI_BLEND_OP_ADD;

            RHIPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
            colorBlendStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlendStateCreateInfo.logicOpEnable = RHI_FALSE;
            colorBlendStateCreateInfo.logicOp = RHI_LOGIC_OP_COPY;
            colorBlendStateCreateInfo.attachmentCount =
                sizeof(colorBlendAttachments) / sizeof(colorBlendAttachments[0]);
            colorBlendStateCreateInfo.pAttachments = &colorBlendAttachments[0];
            colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
            colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
            colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
            colorBlendStateCreateInfo.blendConstants[3] = 0.0f;

            RHIPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
            depthStencilCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencilCreateInfo.depthTestEnable = RHI_TRUE;
            depthStencilCreateInfo.depthWriteEnable = RHI_TRUE;
            depthStencilCreateInfo.depthCompareOp = RHI_COMPARE_OP_LESS;
            depthStencilCreateInfo.depthBoundsTestEnable = RHI_FALSE;
            depthStencilCreateInfo.stencilTestEnable = RHI_FALSE;

            RHIDynamicState                   dynamicStates[] = { RHI_DYNAMIC_STATE_VIEWPORT, RHI_DYNAMIC_STATE_SCISSOR };
            RHIPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
            dynamicStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicStateCreateInfo.dynamicStateCount = 2;
            dynamicStateCreateInfo.pDynamicStates = dynamicStates;

            RHIGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = RHI_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = 2;
            pipelineInfo.pStages = shader_stages;
            pipelineInfo.pVertexInputState = &VertexInputStateCreateInfo;
            pipelineInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
            pipelineInfo.pViewportState = &viewportStateCreateInfo;
            pipelineInfo.pRasterizationState = &rasterizationStateCreateInfo;
            pipelineInfo.pMultisampleState = &multisampleStateCreateInfo;
            pipelineInfo.pColorBlendState = &colorBlendStateCreateInfo;
            pipelineInfo.pDepthStencilState = &depthStencilCreateInfo;
            pipelineInfo.layout = m_RenderPipelines[_render_pipeline_type_skybox].layout;
            pipelineInfo.renderPass = m_FrameBuffer.render_pass;
            pipelineInfo.subpass = _main_camera_subpass_forward_lighting;
            pipelineInfo.basePipelineHandle = RHI_NULL_HANDLE;
            pipelineInfo.pDynamicState = &dynamicStateCreateInfo;

            if (RHI_SUCCESS != m_RenderCommand->CreateGraphicsPipelines(RHI_NULL_HANDLE,
                1,
                &pipelineInfo,
                m_RenderPipelines[_render_pipeline_type_skybox].pipeline))
            {
                throw std::runtime_error("create skybox graphics pipeline");
            }

            m_RenderCommand->DestroyShaderModule(vertShaderModule);
            m_RenderCommand->DestroyShaderModule(fragShaderModule);
            }

        // draw axis
        {
            RHIDescriptorSetLayout* descriptorset_layouts[1] = { m_DescriptorInfos[_axis].layout };
            RHIPipelineLayoutCreateInfo pipeline_layout_create_info{};
            pipeline_layout_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_create_info.setLayoutCount = 1;
            pipeline_layout_create_info.pSetLayouts = descriptorset_layouts;

            if (m_RenderCommand->CreatePipelineLayout(&pipeline_layout_create_info, m_RenderPipelines[_render_pipeline_type_axis].layout) != RHI_SUCCESS)
            {
                throw std::runtime_error("create axis pipeline layout");
            }

            RHIShader* vertShaderModule = m_RenderCommand->CreateShaderModule(AXIS_VERT);
            RHIShader* fragShaderModule = m_RenderCommand->CreateShaderModule(AXIS_FRAG);

            RHIPipelineShaderStageCreateInfo vertPipelineShaderStageCreateInfo{};
            vertPipelineShaderStageCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertPipelineShaderStageCreateInfo.stage = RHI_SHADER_STAGE_VERTEX_BIT;
            vertPipelineShaderStageCreateInfo.module = vertShaderModule;
            vertPipelineShaderStageCreateInfo.pName = "main";
            // vertPipelineShaderStageCreateInfo.pSpecializationInfo

            RHIPipelineShaderStageCreateInfo fragPipelineShaderStageCreateInfo{};
            fragPipelineShaderStageCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragPipelineShaderStageCreateInfo.stage = RHI_SHADER_STAGE_FRAGMENT_BIT;
            fragPipelineShaderStageCreateInfo.module = fragShaderModule;
            fragPipelineShaderStageCreateInfo.pName = "main";

            RHIPipelineShaderStageCreateInfo shader_stages[] = { vertPipelineShaderStageCreateInfo,
                                                               fragPipelineShaderStageCreateInfo };

            auto                                 vertexBindingDescriptions = MeshVertex::GetBindingDescriptions();
            auto                                 vertexAttributeDescriptions = MeshVertex::GetAttributeDescriptions();
            RHIPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo{};
            VertexInputStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            VertexInputStateCreateInfo.vertexBindingDescriptionCount = vertexBindingDescriptions.size();
            VertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexBindingDescriptions[0];
            VertexInputStateCreateInfo.vertexAttributeDescriptionCount = vertexAttributeDescriptions.size();
            VertexInputStateCreateInfo.pVertexAttributeDescriptions = &vertexAttributeDescriptions[0];

            RHIPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
            inputAssemblyCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssemblyCreateInfo.topology = RHI_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssemblyCreateInfo.primitiveRestartEnable = RHI_FALSE;

            RHIPipelineViewportStateCreateInfo viewportStateCreateInfo{};
            viewportStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportStateCreateInfo.viewportCount = 1;
            viewportStateCreateInfo.pViewports = m_RenderCommand->GetSwapchainInfo().viewport;
            viewportStateCreateInfo.scissorCount = 1;
            viewportStateCreateInfo.pScissors = m_RenderCommand->GetSwapchainInfo().scissor;

            RHIPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
            rasterizationStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizationStateCreateInfo.depthClampEnable = RHI_FALSE;
            rasterizationStateCreateInfo.rasterizerDiscardEnable = RHI_FALSE;
            rasterizationStateCreateInfo.polygonMode = RHI_POLYGON_MODE_FILL;
            rasterizationStateCreateInfo.lineWidth = 1.0f;
            rasterizationStateCreateInfo.cullMode = RHI_CULL_MODE_NONE;
            rasterizationStateCreateInfo.frontFace = RHI_FRONT_FACE_COUNTER_CLOCKWISE;
            rasterizationStateCreateInfo.depthBiasEnable = RHI_FALSE;
            rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
            rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
            rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;

            RHIPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
            multisampleStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampleStateCreateInfo.sampleShadingEnable = RHI_FALSE;
            multisampleStateCreateInfo.rasterizationSamples = RHI_SAMPLE_COUNT_1_BIT;

            RHIPipelineColorBlendAttachmentState color_blend_attachment_state{};
            color_blend_attachment_state.colorWriteMask = RHI_COLOR_COMPONENT_R_BIT | RHI_COLOR_COMPONENT_G_BIT |
                RHI_COLOR_COMPONENT_B_BIT | RHI_COLOR_COMPONENT_A_BIT;
            color_blend_attachment_state.blendEnable = RHI_FALSE;
            color_blend_attachment_state.srcColorBlendFactor = RHI_BLEND_FACTOR_ONE;
            color_blend_attachment_state.dstColorBlendFactor = RHI_BLEND_FACTOR_ZERO;
            color_blend_attachment_state.colorBlendOp = RHI_BLEND_OP_ADD;
            color_blend_attachment_state.srcAlphaBlendFactor = RHI_BLEND_FACTOR_ONE;
            color_blend_attachment_state.dstAlphaBlendFactor = RHI_BLEND_FACTOR_ZERO;
            color_blend_attachment_state.alphaBlendOp = RHI_BLEND_OP_ADD;

            RHIPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
            colorBlendStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlendStateCreateInfo.logicOpEnable = RHI_FALSE;
            colorBlendStateCreateInfo.logicOp = RHI_LOGIC_OP_COPY;
            colorBlendStateCreateInfo.attachmentCount = 1;
            colorBlendStateCreateInfo.pAttachments = &color_blend_attachment_state;
            colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
            colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
            colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
            colorBlendStateCreateInfo.blendConstants[3] = 0.0f;

            RHIPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
            depthStencilCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencilCreateInfo.depthTestEnable = RHI_FALSE;
            depthStencilCreateInfo.depthWriteEnable = RHI_FALSE;
            depthStencilCreateInfo.depthCompareOp = RHI_COMPARE_OP_LESS;
            depthStencilCreateInfo.depthBoundsTestEnable = RHI_FALSE;
            depthStencilCreateInfo.stencilTestEnable = RHI_FALSE;

            RHIDynamicState                   dynamicStates[] = { RHI_DYNAMIC_STATE_VIEWPORT, RHI_DYNAMIC_STATE_SCISSOR };
            RHIPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
            dynamicStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicStateCreateInfo.dynamicStateCount = 2;
            dynamicStateCreateInfo.pDynamicStates = dynamicStates;

            RHIGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = RHI_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = 2;
            pipelineInfo.pStages = shader_stages;
            pipelineInfo.pVertexInputState = &VertexInputStateCreateInfo;
            pipelineInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
            pipelineInfo.pViewportState = &viewportStateCreateInfo;
            pipelineInfo.pRasterizationState = &rasterizationStateCreateInfo;
            pipelineInfo.pMultisampleState = &multisampleStateCreateInfo;
            pipelineInfo.pColorBlendState = &colorBlendStateCreateInfo;
            pipelineInfo.pDepthStencilState = &depthStencilCreateInfo;
            pipelineInfo.layout = m_RenderPipelines[_render_pipeline_type_axis].layout;
            pipelineInfo.renderPass = m_FrameBuffer.render_pass;
            pipelineInfo.subpass = _main_camera_subpass_ui;
            pipelineInfo.basePipelineHandle = RHI_NULL_HANDLE;
            pipelineInfo.pDynamicState = &dynamicStateCreateInfo;

            if (RHI_SUCCESS != m_RenderCommand->CreateGraphicsPipelines(RHI_NULL_HANDLE,
                1,
                &pipelineInfo,
                m_RenderPipelines[_render_pipeline_type_axis].pipeline))
            {
                throw std::runtime_error("create axis graphics pipeline");
            }

            m_RenderCommand->DestroyShaderModule(vertShaderModule);
            m_RenderCommand->DestroyShaderModule(fragShaderModule);
        }

    }

    void MainCameraPass::SetupDescriptorSet()
    {
        SetupModelGlobalDescriptorSet();
        SetupSkyboxDescriptorSet();
        SetupAxisDescriptorSet();
        SetupGbufferLightingDescriptorSet();
    }

    void MainCameraPass::SetupFramebufferDescriptorSet()
    {
        RHIDescriptorImageInfo gbuffer_normal_input_attachment_info = {};
        gbuffer_normal_input_attachment_info.sampler = m_RenderCommand->GetOrCreateDefaultSampler(Default_Sampler_Nearest);
        gbuffer_normal_input_attachment_info.imageView = m_FrameBuffer.attachments[_main_camera_pass_gbuffer_a].view;
        gbuffer_normal_input_attachment_info.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        RHIDescriptorImageInfo gbuffer_metallic_roughness_shadingmodeid_input_attachment_info = {};
        gbuffer_metallic_roughness_shadingmodeid_input_attachment_info.sampler = m_RenderCommand->GetOrCreateDefaultSampler(Default_Sampler_Nearest);
        gbuffer_metallic_roughness_shadingmodeid_input_attachment_info.imageView =
            m_FrameBuffer.attachments[_main_camera_pass_gbuffer_b].view;
        gbuffer_metallic_roughness_shadingmodeid_input_attachment_info.imageLayout =
            RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        RHIDescriptorImageInfo gbuffer_albedo_input_attachment_info = {};
        gbuffer_albedo_input_attachment_info.sampler = m_RenderCommand->GetOrCreateDefaultSampler(Default_Sampler_Nearest);
        gbuffer_albedo_input_attachment_info.imageView = m_FrameBuffer.attachments[_main_camera_pass_gbuffer_c].view;
        gbuffer_albedo_input_attachment_info.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        RHIDescriptorImageInfo depth_input_attachment_info = {};
        depth_input_attachment_info.sampler = m_RenderCommand->GetOrCreateDefaultSampler(Default_Sampler_Nearest);
        depth_input_attachment_info.imageView = m_RenderCommand->GetDepthImageInfo().depth_image_view;
        depth_input_attachment_info.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        RHIWriteDescriptorSet deferred_lighting_descriptor_writes_info[4];

        RHIWriteDescriptorSet& gbuffer_normal_descriptor_input_attachment_write_info =
            deferred_lighting_descriptor_writes_info[0];
        gbuffer_normal_descriptor_input_attachment_write_info.sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        gbuffer_normal_descriptor_input_attachment_write_info.pNext = NULL;
        gbuffer_normal_descriptor_input_attachment_write_info.dstSet =
            m_DescriptorInfos[_deferred_lighting].descriptor_set;
        gbuffer_normal_descriptor_input_attachment_write_info.dstBinding = 0;
        gbuffer_normal_descriptor_input_attachment_write_info.dstArrayElement = 0;
        gbuffer_normal_descriptor_input_attachment_write_info.descriptorType = RHI_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        gbuffer_normal_descriptor_input_attachment_write_info.descriptorCount = 1;
        gbuffer_normal_descriptor_input_attachment_write_info.pImageInfo = &gbuffer_normal_input_attachment_info;

        RHIWriteDescriptorSet& gbuffer_metallic_roughness_shadingmodeid_descriptor_input_attachment_write_info =
            deferred_lighting_descriptor_writes_info[1];
        gbuffer_metallic_roughness_shadingmodeid_descriptor_input_attachment_write_info.sType =
            RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        gbuffer_metallic_roughness_shadingmodeid_descriptor_input_attachment_write_info.pNext = NULL;
        gbuffer_metallic_roughness_shadingmodeid_descriptor_input_attachment_write_info.dstSet =
            m_DescriptorInfos[_deferred_lighting].descriptor_set;
        gbuffer_metallic_roughness_shadingmodeid_descriptor_input_attachment_write_info.dstBinding = 1;
        gbuffer_metallic_roughness_shadingmodeid_descriptor_input_attachment_write_info.dstArrayElement = 0;
        gbuffer_metallic_roughness_shadingmodeid_descriptor_input_attachment_write_info.descriptorType =
            RHI_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        gbuffer_metallic_roughness_shadingmodeid_descriptor_input_attachment_write_info.descriptorCount = 1;
        gbuffer_metallic_roughness_shadingmodeid_descriptor_input_attachment_write_info.pImageInfo =
            &gbuffer_metallic_roughness_shadingmodeid_input_attachment_info;

        RHIWriteDescriptorSet& gbuffer_albedo_descriptor_input_attachment_write_info =
            deferred_lighting_descriptor_writes_info[2];
        gbuffer_albedo_descriptor_input_attachment_write_info.sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        gbuffer_albedo_descriptor_input_attachment_write_info.pNext = NULL;
        gbuffer_albedo_descriptor_input_attachment_write_info.dstSet =
            m_DescriptorInfos[_deferred_lighting].descriptor_set;
        gbuffer_albedo_descriptor_input_attachment_write_info.dstBinding = 2;
        gbuffer_albedo_descriptor_input_attachment_write_info.dstArrayElement = 0;
        gbuffer_albedo_descriptor_input_attachment_write_info.descriptorType = RHI_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        gbuffer_albedo_descriptor_input_attachment_write_info.descriptorCount = 1;
        gbuffer_albedo_descriptor_input_attachment_write_info.pImageInfo = &gbuffer_albedo_input_attachment_info;

        RHIWriteDescriptorSet& depth_descriptor_input_attachment_write_info =
            deferred_lighting_descriptor_writes_info[3];
        depth_descriptor_input_attachment_write_info.sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        depth_descriptor_input_attachment_write_info.pNext = NULL;
        depth_descriptor_input_attachment_write_info.dstSet = m_DescriptorInfos[_deferred_lighting].descriptor_set;
        depth_descriptor_input_attachment_write_info.dstBinding = 3;
        depth_descriptor_input_attachment_write_info.dstArrayElement = 0;
        depth_descriptor_input_attachment_write_info.descriptorType = RHI_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        depth_descriptor_input_attachment_write_info.descriptorCount = 1;
        depth_descriptor_input_attachment_write_info.pImageInfo = &depth_input_attachment_info;

        m_RenderCommand->UpdateDescriptorSets(sizeof(deferred_lighting_descriptor_writes_info) /
            sizeof(deferred_lighting_descriptor_writes_info[0]),
            deferred_lighting_descriptor_writes_info,
            0,
            NULL);
    }

    void MainCameraPass::SetupSwapchainFramebuffers()
    {
        m_SwapchainFramebuffers.resize(m_RenderCommand->GetSwapchainInfo().imageViews.size());

        // create frame buffer for every imageview
        for (size_t i = 0; i < m_RenderCommand->GetSwapchainInfo().imageViews.size(); i++)
        {
            RHIImageView* framebuffer_attachments_for_image_view[_main_camera_pass_attachment_count] = {
                m_FrameBuffer.attachments[_main_camera_pass_gbuffer_a].view,
                m_FrameBuffer.attachments[_main_camera_pass_gbuffer_b].view,
                m_FrameBuffer.attachments[_main_camera_pass_gbuffer_c].view,
                m_FrameBuffer.attachments[_main_camera_pass_backup_buffer_odd].view,
                m_FrameBuffer.attachments[_main_camera_pass_backup_buffer_even].view,
                m_FrameBuffer.attachments[_main_camera_pass_post_process_buffer_odd].view,
                m_FrameBuffer.attachments[_main_camera_pass_post_process_buffer_even].view,
                m_RenderCommand->GetDepthImageInfo().depth_image_view,
                m_RenderCommand->GetSwapchainInfo().imageViews[i] };

            RHIFramebufferCreateInfo framebuffer_create_info{};
            framebuffer_create_info.sType = RHI_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_create_info.flags = 0U;
            framebuffer_create_info.renderPass = m_FrameBuffer.render_pass;
            framebuffer_create_info.attachmentCount =
                (sizeof(framebuffer_attachments_for_image_view) / sizeof(framebuffer_attachments_for_image_view[0]));
            framebuffer_create_info.pAttachments = framebuffer_attachments_for_image_view;
            framebuffer_create_info.width = m_RenderCommand->GetSwapchainInfo().extent.width;
            framebuffer_create_info.height = m_RenderCommand->GetSwapchainInfo().extent.height;
            framebuffer_create_info.layers = 1;

            m_SwapchainFramebuffers[i] = new VulkanFramebuffer();
            if (RHI_SUCCESS != m_RenderCommand->CreateFramebuffer(&framebuffer_create_info, m_SwapchainFramebuffers[i]))
            {
                throw std::runtime_error("create main camera framebuffer");
            }
        }
    }

    void MainCameraPass::SetupModelGlobalDescriptorSet()
    {
        // update common model's global descriptor set
        RHIDescriptorSetAllocateInfo meshGlobalDescriptorSetAllocInfo;
        meshGlobalDescriptorSetAllocInfo.sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        meshGlobalDescriptorSetAllocInfo.pNext = NULL;
        meshGlobalDescriptorSetAllocInfo.descriptorPool = m_RenderCommand->GetDescriptorPoor();
        meshGlobalDescriptorSetAllocInfo.descriptorSetCount = 1;
        meshGlobalDescriptorSetAllocInfo.pSetLayouts = &m_DescriptorInfos[_mesh_global].layout;

        if (RHI_SUCCESS != m_RenderCommand->AllocateDescriptorSets(&meshGlobalDescriptorSetAllocInfo, m_DescriptorInfos[_mesh_global].descriptor_set))
        {
            throw std::runtime_error("allocate mesh global descriptor set");
        }

        RHIDescriptorBufferInfo meshPerframeStorageBufferInfo = {};
        // this offset plus dynamic_offset should not be greater than the size of the buffer
        meshPerframeStorageBufferInfo.offset = 0;
        // the range means the size actually used by the shader per draw call
        meshPerframeStorageBufferInfo.range = sizeof(MeshPerframeStorageBufferObject);
        meshPerframeStorageBufferInfo.buffer = m_global_render_resource->_storage_buffer._global_upload_ringbuffer;
        assert(meshPerframeStorageBufferInfo.range <
            m_global_render_resource->_storage_buffer._max_storage_buffer_range);

        RHIDescriptorBufferInfo meshPerdrawcallStorageBufferInfo = {};
        meshPerdrawcallStorageBufferInfo.offset = 0;
        meshPerdrawcallStorageBufferInfo.range = sizeof(MeshPerdrawcallStorageBufferObject);
        meshPerdrawcallStorageBufferInfo.buffer =
            m_global_render_resource->_storage_buffer._global_upload_ringbuffer;
        assert(meshPerdrawcallStorageBufferInfo.range <
            m_global_render_resource->_storage_buffer._max_storage_buffer_range);

        RHIDescriptorBufferInfo meshPerDrawcallVertexBlendingStorageBufferInfo = {};
        meshPerDrawcallVertexBlendingStorageBufferInfo.offset = 0;
        meshPerDrawcallVertexBlendingStorageBufferInfo.range =
            sizeof(MeshPerdrawcallVertexBlendingStorageBufferObject);
        meshPerDrawcallVertexBlendingStorageBufferInfo.buffer =
            m_global_render_resource->_storage_buffer._global_upload_ringbuffer;
        assert(meshPerDrawcallVertexBlendingStorageBufferInfo.range <
            m_global_render_resource->_storage_buffer._max_storage_buffer_range);

        RHIDescriptorImageInfo brdfTextureImageInfo = {};
        brdfTextureImageInfo.sampler = m_global_render_resource->_ibl_resource._brdfLUT_texture_sampler;
        brdfTextureImageInfo.imageView = m_global_render_resource->_ibl_resource._brdfLUT_texture_image_view;
        brdfTextureImageInfo.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        RHIDescriptorImageInfo irradianceTextureImageInfo = {};
        irradianceTextureImageInfo.sampler = m_global_render_resource->_ibl_resource._irradiance_texture_sampler;
        irradianceTextureImageInfo.imageView =
            m_global_render_resource->_ibl_resource._irradiance_texture_image_view;
        irradianceTextureImageInfo.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        RHIDescriptorImageInfo specularTextureImageInfo{};
        specularTextureImageInfo.sampler = m_global_render_resource->_ibl_resource._specular_texture_sampler;
        specularTextureImageInfo.imageView = m_global_render_resource->_ibl_resource._specular_texture_image_view;
        specularTextureImageInfo.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        RHIDescriptorImageInfo pointLightShadowTextureImageInfo{};
        pointLightShadowTextureImageInfo.sampler =
            m_RenderCommand->GetOrCreateDefaultSampler(Default_Sampler_Nearest);
        pointLightShadowTextureImageInfo.imageView = m_point_light_shadow_color_image_view;
        pointLightShadowTextureImageInfo.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        RHIDescriptorImageInfo directionalLightShadowTextureImageInfo{};
        directionalLightShadowTextureImageInfo.sampler =
            m_RenderCommand->GetOrCreateDefaultSampler(Default_Sampler_Nearest);
        directionalLightShadowTextureImageInfo.imageView = m_directional_light_shadow_color_image_view;
        directionalLightShadowTextureImageInfo.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        RHIWriteDescriptorSet meshDescriptorWritesInfo[8];

        meshDescriptorWritesInfo[0].sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        meshDescriptorWritesInfo[0].pNext = NULL;
        meshDescriptorWritesInfo[0].dstSet = m_DescriptorInfos[_mesh_global].descriptor_set;
        meshDescriptorWritesInfo[0].dstBinding = 0;
        meshDescriptorWritesInfo[0].dstArrayElement = 0;
        meshDescriptorWritesInfo[0].descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        meshDescriptorWritesInfo[0].descriptorCount = 1;
        meshDescriptorWritesInfo[0].pBufferInfo = &meshPerframeStorageBufferInfo;

        meshDescriptorWritesInfo[1].sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        meshDescriptorWritesInfo[1].pNext = NULL;
        meshDescriptorWritesInfo[1].dstSet = m_DescriptorInfos[_mesh_global].descriptor_set;
        meshDescriptorWritesInfo[1].dstBinding = 1;
        meshDescriptorWritesInfo[1].dstArrayElement = 0;
        meshDescriptorWritesInfo[1].descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        meshDescriptorWritesInfo[1].descriptorCount = 1;
        meshDescriptorWritesInfo[1].pBufferInfo = &meshPerdrawcallStorageBufferInfo;

        meshDescriptorWritesInfo[2].sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        meshDescriptorWritesInfo[2].pNext = NULL;
        meshDescriptorWritesInfo[2].dstSet = m_DescriptorInfos[_mesh_global].descriptor_set;
        meshDescriptorWritesInfo[2].dstBinding = 2;
        meshDescriptorWritesInfo[2].dstArrayElement = 0;
        meshDescriptorWritesInfo[2].descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        meshDescriptorWritesInfo[2].descriptorCount = 1;
        meshDescriptorWritesInfo[2].pBufferInfo = &meshPerDrawcallVertexBlendingStorageBufferInfo;

        meshDescriptorWritesInfo[3].sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        meshDescriptorWritesInfo[3].pNext = NULL;
        meshDescriptorWritesInfo[3].dstSet = m_DescriptorInfos[_mesh_global].descriptor_set;
        meshDescriptorWritesInfo[3].dstBinding = 3;
        meshDescriptorWritesInfo[3].dstArrayElement = 0;
        meshDescriptorWritesInfo[3].descriptorType = RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        meshDescriptorWritesInfo[3].descriptorCount = 1;
        meshDescriptorWritesInfo[3].pImageInfo = &brdfTextureImageInfo;

        meshDescriptorWritesInfo[4] = meshDescriptorWritesInfo[3];
        meshDescriptorWritesInfo[4].dstBinding = 4;
        meshDescriptorWritesInfo[4].pImageInfo = &irradianceTextureImageInfo;

        meshDescriptorWritesInfo[5] = meshDescriptorWritesInfo[3];
        meshDescriptorWritesInfo[5].dstBinding = 5;
        meshDescriptorWritesInfo[5].pImageInfo = &specularTextureImageInfo;

        meshDescriptorWritesInfo[6] = meshDescriptorWritesInfo[3];
        meshDescriptorWritesInfo[6].dstBinding = 6;
        meshDescriptorWritesInfo[6].pImageInfo = &pointLightShadowTextureImageInfo;

        meshDescriptorWritesInfo[7] = meshDescriptorWritesInfo[3];
        meshDescriptorWritesInfo[7].dstBinding = 7;
        meshDescriptorWritesInfo[7].pImageInfo = &directionalLightShadowTextureImageInfo;

        m_RenderCommand->UpdateDescriptorSets(sizeof(meshDescriptorWritesInfo) / sizeof(meshDescriptorWritesInfo[0]),
            meshDescriptorWritesInfo,
            0,
            NULL);
    }

    void MainCameraPass::SetupSkyboxDescriptorSet()
    {
        RHIDescriptorSetAllocateInfo skybox_descriptor_set_alloc_info;
        skybox_descriptor_set_alloc_info.sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        skybox_descriptor_set_alloc_info.pNext = NULL;
        skybox_descriptor_set_alloc_info.descriptorPool = m_RenderCommand->GetDescriptorPoor();
        skybox_descriptor_set_alloc_info.descriptorSetCount = 1;
        skybox_descriptor_set_alloc_info.pSetLayouts = &m_DescriptorInfos[_skybox].layout;

        if (RHI_SUCCESS != m_RenderCommand->AllocateDescriptorSets(&skybox_descriptor_set_alloc_info, m_DescriptorInfos[_skybox].descriptor_set))
        {
            throw std::runtime_error("allocate skybox descriptor set");
        }

        RHIDescriptorBufferInfo mesh_perframe_storage_buffer_info = {};
        mesh_perframe_storage_buffer_info.offset = 0;
        mesh_perframe_storage_buffer_info.range = sizeof(MeshPerframeStorageBufferObject);
        mesh_perframe_storage_buffer_info.buffer = m_global_render_resource->_storage_buffer._global_upload_ringbuffer;
        assert(mesh_perframe_storage_buffer_info.range <
            m_global_render_resource->_storage_buffer._max_storage_buffer_range);

        RHIDescriptorImageInfo specular_texture_image_info = {};
        specular_texture_image_info.sampler = m_global_render_resource->_ibl_resource._specular_texture_sampler;
        specular_texture_image_info.imageView = m_global_render_resource->_ibl_resource._specular_texture_image_view;
        specular_texture_image_info.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        RHIWriteDescriptorSet skybox_descriptor_writes_info[2];

        skybox_descriptor_writes_info[0].sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        skybox_descriptor_writes_info[0].pNext = NULL;
        skybox_descriptor_writes_info[0].dstSet = m_DescriptorInfos[_skybox].descriptor_set;
        skybox_descriptor_writes_info[0].dstBinding = 0;
        skybox_descriptor_writes_info[0].dstArrayElement = 0;
        skybox_descriptor_writes_info[0].descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        skybox_descriptor_writes_info[0].descriptorCount = 1;
        skybox_descriptor_writes_info[0].pBufferInfo = &mesh_perframe_storage_buffer_info;

        skybox_descriptor_writes_info[1].sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        skybox_descriptor_writes_info[1].pNext = NULL;
        skybox_descriptor_writes_info[1].dstSet = m_DescriptorInfos[_skybox].descriptor_set;
        skybox_descriptor_writes_info[1].dstBinding = 1;
        skybox_descriptor_writes_info[1].dstArrayElement = 0;
        skybox_descriptor_writes_info[1].descriptorType = RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        skybox_descriptor_writes_info[1].descriptorCount = 1;
        skybox_descriptor_writes_info[1].pImageInfo = &specular_texture_image_info;

        m_RenderCommand->UpdateDescriptorSets(2, skybox_descriptor_writes_info, 0, NULL);
    }

    void MainCameraPass::SetupAxisDescriptorSet()
    {
        RHIDescriptorSetAllocateInfo axis_descriptor_set_alloc_info;
        axis_descriptor_set_alloc_info.sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        axis_descriptor_set_alloc_info.pNext = NULL;
        axis_descriptor_set_alloc_info.descriptorPool = m_RenderCommand->GetDescriptorPoor();
        axis_descriptor_set_alloc_info.descriptorSetCount = 1;
        axis_descriptor_set_alloc_info.pSetLayouts = &m_DescriptorInfos[_axis].layout;

        if (RHI_SUCCESS != m_RenderCommand->AllocateDescriptorSets(&axis_descriptor_set_alloc_info, m_DescriptorInfos[_axis].descriptor_set))
        {
            throw std::runtime_error("allocate axis descriptor set");
        }

        RHIDescriptorBufferInfo mesh_perframe_storage_buffer_info = {};
        mesh_perframe_storage_buffer_info.offset = 0;
        mesh_perframe_storage_buffer_info.range = sizeof(MeshPerframeStorageBufferObject);
        mesh_perframe_storage_buffer_info.buffer = m_global_render_resource->_storage_buffer._global_upload_ringbuffer;
        assert(mesh_perframe_storage_buffer_info.range <
            m_global_render_resource->_storage_buffer._max_storage_buffer_range);

        RHIDescriptorBufferInfo axis_storage_buffer_info = {};
        axis_storage_buffer_info.offset = 0;
        axis_storage_buffer_info.range = sizeof(AxisStorageBufferObject);
        axis_storage_buffer_info.buffer = m_global_render_resource->_storage_buffer._axis_inefficient_storage_buffer;

        RHIWriteDescriptorSet axis_descriptor_writes_info[2];

        axis_descriptor_writes_info[0].sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        axis_descriptor_writes_info[0].pNext = NULL;
        axis_descriptor_writes_info[0].dstSet = m_DescriptorInfos[_axis].descriptor_set;
        axis_descriptor_writes_info[0].dstBinding = 0;
        axis_descriptor_writes_info[0].dstArrayElement = 0;
        axis_descriptor_writes_info[0].descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        axis_descriptor_writes_info[0].descriptorCount = 1;
        axis_descriptor_writes_info[0].pBufferInfo = &mesh_perframe_storage_buffer_info;

        axis_descriptor_writes_info[1].sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        axis_descriptor_writes_info[1].pNext = NULL;
        axis_descriptor_writes_info[1].dstSet = m_DescriptorInfos[_axis].descriptor_set;
        axis_descriptor_writes_info[1].dstBinding = 1;
        axis_descriptor_writes_info[1].dstArrayElement = 0;
        axis_descriptor_writes_info[1].descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        axis_descriptor_writes_info[1].descriptorCount = 1;
        axis_descriptor_writes_info[1].pBufferInfo = &axis_storage_buffer_info;

        m_RenderCommand->UpdateDescriptorSets((uint32_t)(sizeof(axis_descriptor_writes_info) / sizeof(axis_descriptor_writes_info[0])),
            axis_descriptor_writes_info,
            0,
            NULL);
    }


    void MainCameraPass::SetupGbufferLightingDescriptorSet()
    {
        RHIDescriptorSetAllocateInfo axis_descriptor_set_alloc_info;
        axis_descriptor_set_alloc_info.sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        axis_descriptor_set_alloc_info.pNext = NULL;
        axis_descriptor_set_alloc_info.descriptorPool = m_RenderCommand->GetDescriptorPoor();
        axis_descriptor_set_alloc_info.descriptorSetCount = 1;
        axis_descriptor_set_alloc_info.pSetLayouts = &m_DescriptorInfos[_axis].layout;

        if (RHI_SUCCESS != m_RenderCommand->AllocateDescriptorSets(&axis_descriptor_set_alloc_info, m_DescriptorInfos[_axis].descriptor_set))
        {
            throw std::runtime_error("allocate axis descriptor set");
        }

        RHIDescriptorBufferInfo mesh_perframe_storage_buffer_info = {};
        mesh_perframe_storage_buffer_info.offset = 0;
        mesh_perframe_storage_buffer_info.range = sizeof(MeshPerframeStorageBufferObject);
        mesh_perframe_storage_buffer_info.buffer = m_global_render_resource->_storage_buffer._global_upload_ringbuffer;
        assert(mesh_perframe_storage_buffer_info.range <
            m_global_render_resource->_storage_buffer._max_storage_buffer_range);

        RHIDescriptorBufferInfo axis_storage_buffer_info = {};
        axis_storage_buffer_info.offset = 0;
        axis_storage_buffer_info.range = sizeof(AxisStorageBufferObject);
        axis_storage_buffer_info.buffer = m_global_render_resource->_storage_buffer._axis_inefficient_storage_buffer;

        RHIWriteDescriptorSet axis_descriptor_writes_info[2];

        axis_descriptor_writes_info[0].sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        axis_descriptor_writes_info[0].pNext = NULL;
        axis_descriptor_writes_info[0].dstSet = m_DescriptorInfos[_axis].descriptor_set;
        axis_descriptor_writes_info[0].dstBinding = 0;
        axis_descriptor_writes_info[0].dstArrayElement = 0;
        axis_descriptor_writes_info[0].descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        axis_descriptor_writes_info[0].descriptorCount = 1;
        axis_descriptor_writes_info[0].pBufferInfo = &mesh_perframe_storage_buffer_info;

        axis_descriptor_writes_info[1].sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        axis_descriptor_writes_info[1].pNext = NULL;
        axis_descriptor_writes_info[1].dstSet = m_DescriptorInfos[_axis].descriptor_set;
        axis_descriptor_writes_info[1].dstBinding = 1;
        axis_descriptor_writes_info[1].dstArrayElement = 0;
        axis_descriptor_writes_info[1].descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        axis_descriptor_writes_info[1].descriptorCount = 1;
        axis_descriptor_writes_info[1].pBufferInfo = &axis_storage_buffer_info;

        m_RenderCommand->UpdateDescriptorSets((uint32_t)(sizeof(axis_descriptor_writes_info) / sizeof(axis_descriptor_writes_info[0])),
            axis_descriptor_writes_info,
            0,
            NULL);
    }

    void MainCameraPass::DrawMeshGbuffer()
    {
    }

    void MainCameraPass::DrawDeferredLighting()
    {
    }

    void MainCameraPass::DrawMeshLighting()
    {
    }

    void MainCameraPass::DrawSkybox()
    {
    }

    void MainCameraPass::DrawAxis()
    {
    }

    RHICommandBuffer* MainCameraPass::GetRenderCommandBuffer() { return m_RenderCommand->GetCurrentCommandBuffer(); }

    void MainCameraPass::UpdateAfterFramebufferRecreate()
    {
        for (size_t i = 0; i < m_FrameBuffer.attachments.size(); i++)
        {
            m_RenderCommand->DestroyImage(m_FrameBuffer.attachments[i].image);
            m_RenderCommand->DestroyImageView(m_FrameBuffer.attachments[i].view);
            m_RenderCommand->FreeMemory(m_FrameBuffer.attachments[i].mem);
        }

        for (auto framebuffer : m_SwapchainFramebuffers)
        {
            m_RenderCommand->DestroyFramebuffer(framebuffer);
        }

        SetupAttachments();

        SetupFramebufferDescriptorSet();

        SetupSwapchainFramebuffers();

    }
}