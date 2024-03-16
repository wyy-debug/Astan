#include "aspch.h"
#include "ToneMappingPass.h"

#include <stdexcept>
#include <Astan/Renderer/NewSystem/shader/mesh_frag.h>

namespace Astan
{
    void ToneMappingPass::Initialize(const RenderPassInitInfo* init_info)
    {
        RenderPass::Initialize(nullptr);

        const ToneMappingPassInitInfo* _init_info = static_cast<const ToneMappingPassInitInfo*>(init_info);
        m_FrameBuffer.render_pass = _init_info->render_pass;

        SetupDescriptorSetLayout();
        SetupPipelines();
        SetupDescriptorSet();
        UpdateAfterFramebufferRecreate(_init_info->input_attachment);
    }

    void ToneMappingPass::SetupDescriptorSetLayout()
    {
        m_DescriptorInfos.resize(1);

        RHIDescriptorSetLayoutBinding postProcessGlobalLayoutBindings[1] = {};

        RHIDescriptorSetLayoutBinding& postProcessGlobalLayoutInputAttachmentBinding = postProcessGlobalLayoutBindings[0];
        postProcessGlobalLayoutInputAttachmentBinding.binding = 0;
        postProcessGlobalLayoutInputAttachmentBinding.descriptorType = RHI_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        postProcessGlobalLayoutInputAttachmentBinding.descriptorCount = 1;
        postProcessGlobalLayoutInputAttachmentBinding.stageFlags = RHI_SHADER_STAGE_FRAGMENT_BIT;

        RHIDescriptorSetLayoutCreateInfo postProcessGlobalLayoutCreateInfo;
        postProcessGlobalLayoutCreateInfo.sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        postProcessGlobalLayoutCreateInfo.pNext = NULL;
        postProcessGlobalLayoutCreateInfo.flags = 0;
        postProcessGlobalLayoutCreateInfo.bindingCount = sizeof(postProcessGlobalLayoutBindings) / sizeof(postProcessGlobalLayoutBindings[0]);
        postProcessGlobalLayoutCreateInfo.pBindings = postProcessGlobalLayoutBindings;

        if (RHI_SUCCESS != m_RenderCommand->CreateDescriptorSetLayout(&postProcessGlobalLayoutCreateInfo, m_DescriptorInfos[0].layout))
        {
            throw std::runtime_error("create post process global layout");
        }
    }
    void ToneMappingPass::SetupPipelines()
    {
        m_RenderPipelines.resize(1);

        RHIDescriptorSetLayout* descriptorsetLayouts[1] = { m_DescriptorInfos[0].layout };
        RHIPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = descriptorsetLayouts;

        if (m_RenderCommand->CreatePipelineLayout(&pipelineLayoutCreateInfo, m_RenderPipelines[0].layout) != RHI_SUCCESS)
        {
            throw std::runtime_error("create post process pipeline layout");
        }

        RHIShader* vertShaderModule = m_RenderCommand->CreateShaderModule(MESH_FRAG);
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

        RHIPipelineShaderStageCreateInfo shader_stages[] = { vertPipelineShaderStageCreateInfo,
                                                           fragPipelineShaderStageCreateInfo };

        RHIPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
        vertexInputStateCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
        vertexInputStateCreateInfo.pVertexBindingDescriptions = NULL;
        vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
        vertexInputStateCreateInfo.pVertexAttributeDescriptions = NULL;

        RHIPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
        inputAssemblyCreateInfo.sType = RHI_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyCreateInfo.topology = RHI_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
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
        colorBlendAttachmentState.colorWriteMask = RHI_COLOR_COMPONENT_R_BIT |
            RHI_COLOR_COMPONENT_G_BIT |
            RHI_COLOR_COMPONENT_B_BIT |
            RHI_COLOR_COMPONENT_A_BIT;
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

        RHIPipelineDynamicStateCreateInfo dynamic_state_create_info{};
        dynamic_state_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_create_info.dynamicStateCount = 2;
        dynamic_state_create_info.pDynamicStates = dynamic_states;

        RHIGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = RHI_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shader_stages;
        pipelineInfo.pVertexInputState = &vertexInputStateCreateInfo;
        pipelineInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
        pipelineInfo.pViewportState = &viewportStateCreateInfo;
        pipelineInfo.pRasterizationState = &rasterizationStateCreateInfo;
        pipelineInfo.pMultisampleState = &multisampleStateCreateInfo;
        pipelineInfo.pColorBlendState = &colorBlendStateCreateInfo;
        pipelineInfo.pDepthStencilState = &depthStencilCreateInfo;
        pipelineInfo.layout = m_RenderPipelines[0].layout;
        pipelineInfo.renderPass = m_FrameBuffer.render_pass;
        pipelineInfo.subpass = _main_camera_subpass_tone_mapping;
        pipelineInfo.basePipelineHandle = RHI_NULL_HANDLE;
        pipelineInfo.pDynamicState = &dynamic_state_create_info;

        if (RHI_SUCCESS != m_RenderCommand->CreateGraphicsPipelines(RHI_NULL_HANDLE, 1, &pipelineInfo, m_RenderPipelines[0].pipeline))
        {
            throw std::runtime_error("create post process graphics pipeline");
        }

        m_RenderCommand->DestroyShaderModule(vertShaderModule);
        m_RenderCommand->DestroyShaderModule(fragShaderModule);
    }
    void ToneMappingPass::SetupDescriptorSet()
    {
        RHIDescriptorSetAllocateInfo post_process_global_descriptor_set_alloc_info;
        post_process_global_descriptor_set_alloc_info.sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        post_process_global_descriptor_set_alloc_info.pNext = NULL;
        post_process_global_descriptor_set_alloc_info.descriptorPool = m_RenderCommand->GetDescriptorPoor();
        post_process_global_descriptor_set_alloc_info.descriptorSetCount = 1;
        post_process_global_descriptor_set_alloc_info.pSetLayouts = &m_DescriptorInfos[0].layout;

        if (RHI_SUCCESS != m_RenderCommand->AllocateDescriptorSets(&post_process_global_descriptor_set_alloc_info,
            m_DescriptorInfos[0].descriptor_set))
        {
            throw std::runtime_error("allocate post process global descriptor set");
        }
    }

    void ToneMappingPass::UpdateAfterFramebufferRecreate(RHIImageView* input_attachment)
    {
        RHIDescriptorImageInfo post_process_per_frame_input_attachment_info = {};
        post_process_per_frame_input_attachment_info.sampler = m_RenderCommand->GetOrCreateDefaultSampler(Default_Sampler_Nearest);
        post_process_per_frame_input_attachment_info.imageView = input_attachment;
        post_process_per_frame_input_attachment_info.imageLayout = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        RHIWriteDescriptorSet post_process_descriptor_writes_info[1];

        RHIWriteDescriptorSet& post_process_descriptor_input_attachment_write_info = post_process_descriptor_writes_info[0];
        post_process_descriptor_input_attachment_write_info.sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        post_process_descriptor_input_attachment_write_info.pNext = NULL;
        post_process_descriptor_input_attachment_write_info.dstSet = m_DescriptorInfos[0].descriptor_set;
        post_process_descriptor_input_attachment_write_info.dstBinding = 0;
        post_process_descriptor_input_attachment_write_info.dstArrayElement = 0;
        post_process_descriptor_input_attachment_write_info.descriptorType = RHI_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        post_process_descriptor_input_attachment_write_info.descriptorCount = 1;
        post_process_descriptor_input_attachment_write_info.pImageInfo = &post_process_per_frame_input_attachment_info;

        m_RenderCommand->UpdateDescriptorSets(sizeof(post_process_descriptor_writes_info) /
            sizeof(post_process_descriptor_writes_info[0]),
            post_process_descriptor_writes_info,
            0,
            NULL);
    }

    void ToneMappingPass::Draw()
    {
        float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        m_RenderCommand->PushEvent(m_RenderCommand->GetCurrentCommandBuffer(), "Tone Map", color);

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
