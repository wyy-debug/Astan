#pragma once
#include "Platform/Vulkan/VulkanRendererAPI.h"
namespace Astan
{ 
	enum
	{
		_main_camera_pass_gbuffer_a = 0,
		_main_camera_pass_gbuffer_b = 1,
		_main_camera_pass_gbuffer_c = 2,
		_main_camera_pass_backup_buffer_odd = 3,
		_main_camera_pass_backup_buffer_even = 4,
		_main_camera_pass_post_process_buffer_odd = 5,
		_main_camera_pass_post_process_buffer_even = 6,
		_main_camera_pass_depth = 7,
		_main_camera_pass_swap_chain_image = 8,
		_main_camera_pass_custom_attachment_count = 5,
		_main_camera_pass_post_process_attachment_count = 2,
		_main_camera_pass_attachment_count = 9,
	};

	enum
	{
		_main_camera_subpass_basepass = 0,
		_main_camera_subpass_deferred_lighting,
		_main_camera_subpass_forward_lighting,
		_main_camera_subpass_tone_mapping,
		_main_camera_subpass_color_grading,
		_main_camera_subpass_fxaa,
		_main_camera_subpass_ui,
		_main_camera_subpass_combine_ui,
		_main_camera_subpass_count
	};

	class RenderPass
	{

	public:
		struct FrameBufferAttachment
		{
			RHIImage* image;
			RHIDeviceMemory* mem;
			RHIImageView* view;
			RHIFormat       format;
		};

		struct Framebuffer
		{
			int           width;
			int           height;
			RHIFramebuffer* framebuffer;
			RHIRenderPass* render_pass;

			std::vector<FrameBufferAttachment> attachments;
		};

		struct Descriptor
		{
			RHIDescriptorSetLayout* layout;
			RHIDescriptorSet* descriptor_set;
		};

		struct RenderPipelineBase
		{
			RHIPipelineLayout* layout;
			RHIPipeline* pipeline;
		};
		virtual void Initialize();
		virtual void Draw();
		virtual void SetCommonInfo(Ref<VulkanRendererAPI> renderCommand);

	public:
		Framebuffer m_FrameBuffer;
		std::vector<Descriptor>  m_DescriptorInfos;
		std::vector<RenderPipelineBase> m_RenderPipelines;
	
	protected:
		Ref<VulkanRendererAPI> m_RenderCommand;
	};
}
