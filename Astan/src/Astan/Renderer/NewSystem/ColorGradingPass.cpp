#include "aspch.h"
#include "ColorGradingPass.h"
#include <stdexcept>

namespace Astan
{

    void ColorGradingPass::Initialize(const RenderPassInitInfo* init_info)
    {
        RenderPass::Initialize(nullptr);

        const ColorGradingPassInitInfo* _init_info = static_cast<const ColorGradingPassInitInfo*>(init_info);
        m_FrameBuffer.render_pass = _init_info->render_pass;

        SetupDescriptorSetLayout();
        SetupPipelines();
        SetupDescriptorSet();
        UpdateAfterFramebufferRecreate(_init_info->input_attachment);
    }

    void ColorGradingPass::SetupDescriptorSetLayout()
    {
        m_DescriptorInfos.resize(1);

        RHIDescriptorSetLayoutBinding post_process_global_layout_bindings[2] = {};

        RHIDescriptorSetLayoutBinding& post_process_global_layout_input_attachment_binding =
            post_process_global_layout_bindings[0];
        post_process_global_layout_input_attachment_binding.binding = 0;
        post_process_global_layout_input_attachment_binding.descriptorType = RHI_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        post_process_global_layout_input_attachment_binding.descriptorCount = 1;
        post_process_global_layout_input_attachment_binding.stageFlags = RHI_SHADER_STAGE_FRAGMENT_BIT;

        RHIDescriptorSetLayoutBinding& post_process_global_layout_LUT_binding = post_process_global_layout_bindings[1];
        post_process_global_layout_LUT_binding.binding = 1;
        post_process_global_layout_LUT_binding.descriptorType = RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        post_process_global_layout_LUT_binding.descriptorCount = 1;
        post_process_global_layout_LUT_binding.stageFlags = RHI_SHADER_STAGE_FRAGMENT_BIT;

        RHIDescriptorSetLayoutCreateInfo post_process_global_layout_create_info;
        post_process_global_layout_create_info.sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        post_process_global_layout_create_info.pNext = NULL;
        post_process_global_layout_create_info.flags = 0;
        post_process_global_layout_create_info.bindingCount =
            sizeof(post_process_global_layout_bindings) / sizeof(post_process_global_layout_bindings[0]);
        post_process_global_layout_create_info.pBindings = post_process_global_layout_bindings;

        if (RHI_SUCCESS != m_RenderCommand->CreateDescriptorSetLayout(&post_process_global_layout_create_info, m_DescriptorInfos[0].layout))
        {
            throw std::runtime_error("create post process global layout");
        }
    }

    void ColorGradingPass::SetupPipelines()
    {
        m_RenderPipelines.resize(1);

        RHIDescriptorSetLayout* descriptorset_layouts[1] = { m_DescriptorInfos[0].layout };
        RHIPipelineLayoutCreateInfo pipeline_layout_create_info{};
        pipeline_layout_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_create_info.setLayoutCount = 1;
        pipeline_layout_create_info.pSetLayouts = descriptorset_layouts;

        if (m_RenderCommand->CreatePipelineLayout(&pipeline_layout_create_info, m_RenderPipelines[0].layout) != RHI_SUCCESS)
        {
            throw std::runtime_error("create post process pipeline layout");
        }

        RHIShader* vert_shader_module = m_RenderCommand->CreateShaderModule(POST_PROCESS_VERT);
        RHIShader* frag_shader_module = m_RenderCommand->CreateShaderModule(COLOR_GRADING_FRAG);

        RHIPipelineShaderStageCreateInfo vertPipelineShaderStageCreateInfo{};
        vertPipelineShaderStageCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertPipelineShaderStageCreateInfo.stage = RHI_SHADER_STAGE_VERTEX_BIT;
        vertPipelineShaderStageCreateInfo.module = vert_shader_module;
        vertPipelineShaderStageCreateInfo.pName = "main";

        RHIPipelineShaderStageCreateInfo fragPipelineShaderStageCreateInfo{};
        fragPipelineShaderStageCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragPipelineShaderStageCreateInfo.stage = RHI_SHADER_STAGE_FRAGMENT_BIT;
        fragPipelineShaderStageCreateInfo.module = frag_shader_module;
        fragPipelineShaderStageCreateInfo.pName = "main";

        RHIPipelineShaderStageCreateInfo shader_stages[] = { vertPipelineShaderStageCreateInfo,
                                                           fragPipelineShaderStageCreateInfo };

        RHIPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
        vertex_input_state_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_state_create_info.vertexBindingDescriptionCount = 0;
        vertex_input_state_create_info.pVertexBindingDescriptions = NULL;
        vertex_input_state_create_info.vertexAttributeDescriptionCount = 0;
        vertex_input_state_create_info.pVertexAttributeDescriptions = NULL;

        RHIPipelineInputAssemblyStateCreateInfo input_assembly_create_info{};
        input_assembly_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_create_info.topology = RHI_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        input_assembly_create_info.primitiveRestartEnable = RHI_FALSE;

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
        rasterizationStateCreateInfo.frontFace = RHI_FRONT_FACE_CLOCKWISE;
        rasterizationStateCreateInfo.depthBiasEnable = RHI_FALSE;
        rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
        rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
        rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;

        RHIPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
        multisampleStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleStateCreateInfo.sampleShadingEnable = RHI_FALSE;
        multisampleStateCreateInfo.rasterizationSamples = RHI_SAMPLE_COUNT_1_BIT;

        RHIPipelineColorBlendAttachmentState colorBlendAttachmentState{};
        colorBlendAttachmentState.colorWriteMask =
            RHI_COLOR_COMPONENT_R_BIT | RHI_COLOR_COMPONENT_G_BIT | RHI_COLOR_COMPONENT_B_BIT | RHI_COLOR_COMPONENT_A_BIT;
        colorBlendAttachmentState.blendEnable = RHI_FALSE;
        colorBlendAttachmentState.srcColorBlendFactor = RHI_BLEND_FACTOR_ONE;
        colorBlendAttachmentState.dstColorBlendFactor = RHI_BLEND_FACTOR_ZERO;
        colorBlendAttachmentState.colorBlendOp = RHI_BLEND_OP_ADD;
        colorBlendAttachmentState.srcAlphaBlendFactor = RHI_BLEND_FACTOR_ONE;
        colorBlendAttachmentState.dstAlphaBlendFactor = RHI_BLEND_FACTOR_ZERO;
        colorBlendAttachmentState.alphaBlendOp = RHI_BLEND_OP_ADD;

        RHIPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
        colorBlendStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateCreateInfo.logicOpEnable = RHI_FALSE;
        colorBlendStateCreateInfo.logicOp = RHI_LOGIC_OP_COPY;
        colorBlendStateCreateInfo.attachmentCount = 1;
        colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;
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

        RHIDynamicState dynamic_states[] = { RHI_DYNAMIC_STATE_VIEWPORT, RHI_DYNAMIC_STATE_SCISSOR };

        RHIPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
        dynamicStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCreateInfo.dynamicStateCount = 2;
        dynamicStateCreateInfo.pDynamicStates = dynamic_states;

        RHIGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = RHI_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shader_stages;
        pipelineInfo.pVertexInputState = &vertex_input_state_create_info;
        pipelineInfo.pInputAssemblyState = &input_assembly_create_info;
        pipelineInfo.pViewportState = &viewportStateCreateInfo;
        pipelineInfo.pRasterizationState = &rasterizationStateCreateInfo;
        pipelineInfo.pMultisampleState = &multisampleStateCreateInfo;
        pipelineInfo.pColorBlendState = &colorBlendStateCreateInfo;
        pipelineInfo.pDepthStencilState = &depthStencilCreateInfo;
        pipelineInfo.layout = m_RenderPipelines[0].layout;
        pipelineInfo.renderPass = m_FrameBuffer.render_pass;
        pipelineInfo.subpass = _main_camera_subpass_color_grading;
        pipelineInfo.basePipelineHandle = RHI_NULL_HANDLE;
        pipelineInfo.pDynamicState = &dynamicStateCreateInfo;

        if (RHI_SUCCESS != m_RenderCommand->CreateGraphicsPipelines(RHI_NULL_HANDLE, 1, &pipelineInfo, m_RenderPipelines[0].pipeline))
        {
            throw std::runtime_error("create post process graphics pipeline");
        }

        m_RenderCommand->DestroyShaderModule(vert_shader_module);
        m_RenderCommand->DestroyShaderModule(frag_shader_module);
    }

    void ColorGradingPass::SetupDescriptorSet()
    {
        RHIDescriptorSetAllocateInfo post_process_global_descriptor_set_alloc_info;
        post_process_global_descriptor_set_alloc_info.sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        post_process_global_descriptor_set_alloc_info.pNext = NULL;
        post_process_global_descriptor_set_alloc_info.descriptorPool = m_RenderCommand->GetDescriptorPoor();
        post_process_global_descriptor_set_alloc_info.descriptorSetCount = 1;
        post_process_global_descriptor_set_alloc_info.pSetLayouts = &m_DescriptorInfos[0].layout;

        if (RHI_SUCCESS != m_RenderCommand->AllocateDescriptorSets(&post_process_global_descriptor_set_alloc_info, m_DescriptorInfos[0].descriptor_set))
        {
            throw std::runtime_error("allocate post process global descriptor set");
        }
    }

    void ColorGradingPass::UpdateAfterFramebufferRecreate(RHIImageView* input_attachment)
    {
        RHIDescriptorImageInfo post_process_per_frame_input_attachment_info = {};
        post_process_per_frame_input_attachment_info.sampler =
            m_RenderCommand->GetOrCreateDefaultSampler(Default_Sampler_Nearest);
        post_process_per_frame_input_attachment_info.imageView = input_attachment;
        post_process_per_frame_input_attachment_info.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        RHIDescriptorImageInfo color_grading_LUT_image_info = {};
        color_grading_LUT_image_info.sampler = m_RenderCommand->GetOrCreateDefaultSampler(Default_Sampler_Linear);
        color_grading_LUT_image_info.imageView =
            m_global_render_resource->_color_grading_resource._color_grading_LUT_texture_image_view;
        color_grading_LUT_image_info.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        RHIWriteDescriptorSet postProcessDescriptorWritesInfo[2];

        RHIWriteDescriptorSet& postProcessDescriptorInputAttachmentWriteInfo =
            postProcessDescriptorWritesInfo[0];
        postProcessDescriptorInputAttachmentWriteInfo.sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        postProcessDescriptorInputAttachmentWriteInfo.pNext = NULL;
        postProcessDescriptorInputAttachmentWriteInfo.dstSet = m_DescriptorInfos[0].descriptor_set;
        postProcessDescriptorInputAttachmentWriteInfo.dstBinding = 0;
        postProcessDescriptorInputAttachmentWriteInfo.dstArrayElement = 0;
        postProcessDescriptorInputAttachmentWriteInfo.descriptorType = RHI_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        postProcessDescriptorInputAttachmentWriteInfo.descriptorCount = 1;
        postProcessDescriptorInputAttachmentWriteInfo.pImageInfo = &post_process_per_frame_input_attachment_info;

        RHIWriteDescriptorSet& postProcessDescriptorLUTWriteInfo = postProcessDescriptorWritesInfo[1];
        postProcessDescriptorLUTWriteInfo.sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        postProcessDescriptorLUTWriteInfo.pNext = NULL;
        postProcessDescriptorLUTWriteInfo.dstSet = m_DescriptorInfos[0].descriptor_set;
        postProcessDescriptorLUTWriteInfo.dstBinding = 1;
        postProcessDescriptorLUTWriteInfo.dstArrayElement = 0;
        postProcessDescriptorLUTWriteInfo.descriptorType = RHI_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        postProcessDescriptorLUTWriteInfo.descriptorCount = 1;
        postProcessDescriptorLUTWriteInfo.pImageInfo = &color_grading_LUT_image_info;

        m_RenderCommand->UpdateDescriptorSets(sizeof(postProcessDescriptorWritesInfo) /
            sizeof(postProcessDescriptorWritesInfo[0]),
            postProcessDescriptorWritesInfo,
            0,
            NULL);
    }
    void ColorGradingPass::Draw()
    {
        float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        m_RenderCommand->PushEvent(m_RenderCommand->GetCurrentCommandBuffer(), "Color Grading", color);

        m_RenderCommand->CmdBindPipelinePFN(m_RenderCommand->GetCurrentCommandBuffer(), RHI_PIPELINE_BIND_POINT_GRAPHICS, m_RenderPipelines[0].pipeline);
        m_RenderCommand->CmdSetViewportPFN(m_RenderCommand->GetCurrentCommandBuffer(), 0, 1, m_RenderCommand->GetSwapchainInfo().viewport);
        m_RenderCommand->CmdSetScissorPFN(m_RenderCommand->GetCurrentCommandBuffer(), 0, 1, m_RenderCommand->GetSwapchainInfo().scissor);
        m_RenderCommand->CmdBindDescriptorSetsPFN(m_RenderCommand->GetCurrentCommandBuffer(),
            RHI_PIPELINE_BIND_POINT_GRAPHICS,
            m_RenderPipelines[0].layout,
            0,
            1,
            &m_DescriptorInfos[0].descriptor_set,
            0,
            NULL);

        m_RenderCommand->CmdDraw(m_RenderCommand->GetCurrentCommandBuffer(), 3, 1, 0, 0);

        m_RenderCommand->PopEvent(m_RenderCommand->GetCurrentCommandBuffer());
    }
} // namespace Piccolo
