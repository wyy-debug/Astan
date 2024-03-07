#include "MainCameraPass.h"

void Astan::MainCameraPass::Initialize()
{
    // 设置附件
	SetupAttachments();
    // 设置渲染pass
    SetupRenderPass();
    // 设置描述符布局
    SetupDescriptorSetLayout();
}

void Astan::MainCameraPass::Draw()
{
}

void Astan::MainCameraPass::SetupParticlePass()
{
}

void Astan::MainCameraPass::SetupAttachments()
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

void Astan::MainCameraPass::SetupRenderPass()
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

void Astan::MainCameraPass::SetupDescriptorSetLayout()
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

    // Mesh Global Bind Layout = 1 bind 0-8
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

    {

    }

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
