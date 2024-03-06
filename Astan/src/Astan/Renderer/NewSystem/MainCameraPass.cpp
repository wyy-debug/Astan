#include "MainCameraPass.h"

void Astan::MainCameraPass::Initialize()
{
	SetupAttachments();
}

void Astan::MainCameraPass::Draw()
{
}

void Astan::MainCameraPass::SetupParticlePass()
{
}

void Astan::MainCameraPass::SetupAttachments()
{
	// ´´½¨Framebuffer
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

void Astan::MainCameraPass::SetupRenderPass()
{
}

void Astan::MainCameraPass::SetupDescriptorSetLayout()
{
}

void Astan::MainCameraPass::SetupPipelines()
{
}

void Astan::MainCameraPass::SetupDescriptorSet()
{
}

void Astan::MainCameraPass::SetupFramebufferDescriptorSet()
{
}

void Astan::MainCameraPass::SetupSwapchainFramebuffers()
{
}

void Astan::MainCameraPass::SetupModelGlobalDescriptorSet()
{
}

void Astan::MainCameraPass::SetupSkyboxDescriptorSet()
{
}

void Astan::MainCameraPass::SetupAxisDescriptorSet()
{
}

void Astan::MainCameraPass::SetupParticleDescriptorSet()
{
}

void Astan::MainCameraPass::SetupGbufferLightingDescriptorSet()
{
}

void Astan::MainCameraPass::DrawMeshGbuffer()
{
}

void Astan::MainCameraPass::DrawDeferredLighting()
{
}

void Astan::MainCameraPass::DrawMeshLighting()
{
}

void Astan::MainCameraPass::DrawSkybox()
{
}

void Astan::MainCameraPass::DrawAxis()
{
}
