#include "aspch.h"
#include "PickPass.h"

#include <map>
#include <stdexcept>
#include <Astan/Scene/Scene.h>
#include "RenderMesh.h"
#include <Astan/Renderer/NewSystem/shader/mesh_vert.h>
#include <Astan/Renderer/NewSystem/RenderUtils.h>

namespace Astan
{
    void PickPass::Initialize(const RenderPassInitInfo* init_info)
    {
        RenderPass::Initialize(nullptr);

        const PickPassInitInfo* _init_info = static_cast<const PickPassInitInfo*>(init_info);
        _per_mesh_layout = _init_info->per_mesh_layout;

        SetupAttachments();
        SetupRenderPass();
        SetupFramebuffer();
        SetupDescriptorSetLayout();
        SetupPipelines();
        SetupDescriptorSet();
    }
    void PickPass::PostInitialize() {}
    void PickPass::PreparePassData(Ref<RenderSource> source)
    {
        _mesh_inefficient_pick_perframe_storage_buffer_object.proj_view_matrix =
            source->m_MeshInefficientPickPerframeStorageBufferObject.proj_view_matrix;
        _mesh_inefficient_pick_perframe_storage_buffer_object.rt_width = m_RenderCommand->GetSwapchainInfo().extent.width;
        _mesh_inefficient_pick_perframe_storage_buffer_object.rt_height = m_RenderCommand->GetSwapchainInfo().extent.height;
    }
    void PickPass::Draw() {}
    void PickPass::SetupAttachments()
    {
        m_FrameBuffer.attachments.resize(1);
        m_FrameBuffer.attachments[0].format = RHI_FORMAT_R32_UINT;

        m_RenderCommand->CreateImage(m_RenderCommand->GetSwapchainInfo().extent.width,
            m_RenderCommand->GetSwapchainInfo().extent.height,
            m_FrameBuffer.attachments[0].format,
            RHI_IMAGE_TILING_OPTIMAL,
            RHI_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | RHI_IMAGE_USAGE_TRANSFER_SRC_BIT,
            RHI_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_FrameBuffer.attachments[0].image,
            m_FrameBuffer.attachments[0].mem,
            0,
            1,
            1);
        m_RenderCommand->CreateImageView(m_FrameBuffer.attachments[0].image,
            m_FrameBuffer.attachments[0].format,
            RHI_IMAGE_ASPECT_COLOR_BIT,
            RHI_IMAGE_VIEW_TYPE_2D,
            1,
            1,
            m_FrameBuffer.attachments[0].view);
    }
    void PickPass::SetupRenderPass()
    {
        RHIAttachmentDescription color_attachment_description{};
        color_attachment_description.format = m_FrameBuffer.attachments[0].format;
        color_attachment_description.samples = RHI_SAMPLE_COUNT_1_BIT;
        color_attachment_description.loadOp = RHI_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment_description.storeOp = RHI_ATTACHMENT_STORE_OP_STORE;
        color_attachment_description.stencilLoadOp = RHI_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment_description.stencilStoreOp = RHI_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment_description.initialLayout = RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color_attachment_description.finalLayout = RHI_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

        RHIAttachmentDescription depth_attachment_description{};
        depth_attachment_description.format = m_RenderCommand->GetDepthImageInfo().depth_image_format;
        depth_attachment_description.samples = RHI_SAMPLE_COUNT_1_BIT;
        depth_attachment_description.loadOp = RHI_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment_description.storeOp = RHI_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment_description.stencilLoadOp = RHI_ATTACHMENT_LOAD_OP_DONT_CARE;
        depth_attachment_description.stencilStoreOp = RHI_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment_description.initialLayout = RHI_IMAGE_LAYOUT_UNDEFINED;
        depth_attachment_description.finalLayout = RHI_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        RHIAttachmentDescription attachments[2] = { color_attachment_description, depth_attachment_description };

        RHIAttachmentReference color_attachment_reference{};
        color_attachment_reference.attachment = 0;
        color_attachment_reference.layout = RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        RHIAttachmentReference depth_attachment_reference{};
        depth_attachment_reference.attachment = 1;
        depth_attachment_reference.layout = RHI_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        RHISubpassDescription subpass{};
        subpass.pipelineBindPoint = RHI_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_reference;
        subpass.pDepthStencilAttachment = &depth_attachment_reference;

        RHIRenderPassCreateInfo renderpass_create_info{};
        renderpass_create_info.sType = RHI_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderpass_create_info.attachmentCount = sizeof(attachments) / sizeof(attachments[0]);
        renderpass_create_info.pAttachments = attachments;
        renderpass_create_info.subpassCount = 1;
        renderpass_create_info.pSubpasses = &subpass;
        renderpass_create_info.dependencyCount = 0;
        renderpass_create_info.pDependencies = NULL;

        if (m_RenderCommand->CreateRenderPass(&renderpass_create_info, m_FrameBuffer.render_pass) != RHI_SUCCESS)
        {
            throw std::runtime_error("create inefficient pick render pass");
        }
    }
    void PickPass::SetupFramebuffer()
    {
        RHIImageView* attachments[2] = { m_FrameBuffer.attachments[0].view, m_RenderCommand->GetDepthImageInfo().depth_image_view };

        RHIFramebufferCreateInfo framebuffer_create_info{};
        framebuffer_create_info.sType = RHI_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass = m_FrameBuffer.render_pass;
        framebuffer_create_info.attachmentCount = sizeof(attachments) / sizeof(attachments[0]);
        framebuffer_create_info.pAttachments = attachments;
        framebuffer_create_info.width = m_RenderCommand->GetSwapchainInfo().extent.width;
        framebuffer_create_info.height = m_RenderCommand->GetSwapchainInfo().extent.height;
        framebuffer_create_info.layers = 1;

        if (m_RenderCommand->CreateFramebuffer(&framebuffer_create_info, m_FrameBuffer.framebuffer) != RHI_SUCCESS)
        {
            throw std::runtime_error("create inefficient pick framebuffer");
        }
    }
    void PickPass::SetupDescriptorSetLayout()
    {
        m_DescriptorInfos.resize(1);

        RHIDescriptorSetLayoutBinding mesh_inefficient_pick_global_layout_bindings[3];

        RHIDescriptorSetLayoutBinding& mesh_inefficient_pick_global_layout_perframe_storage_buffer_binding =
            mesh_inefficient_pick_global_layout_bindings[0];
        mesh_inefficient_pick_global_layout_perframe_storage_buffer_binding.binding = 0;
        mesh_inefficient_pick_global_layout_perframe_storage_buffer_binding.descriptorType =
            RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_inefficient_pick_global_layout_perframe_storage_buffer_binding.descriptorCount = 1;
        mesh_inefficient_pick_global_layout_perframe_storage_buffer_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        mesh_inefficient_pick_global_layout_perframe_storage_buffer_binding.pImmutableSamplers = NULL;

        RHIDescriptorSetLayoutBinding& mesh_inefficient_pick_global_layout_perdrawcall_storage_buffer_binding =
            mesh_inefficient_pick_global_layout_bindings[1];
        mesh_inefficient_pick_global_layout_perdrawcall_storage_buffer_binding.binding = 1;
        mesh_inefficient_pick_global_layout_perdrawcall_storage_buffer_binding.descriptorType =
            RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_inefficient_pick_global_layout_perdrawcall_storage_buffer_binding.descriptorCount = 1;
        mesh_inefficient_pick_global_layout_perdrawcall_storage_buffer_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        mesh_inefficient_pick_global_layout_perdrawcall_storage_buffer_binding.pImmutableSamplers = NULL;

        RHIDescriptorSetLayoutBinding&
            mesh_inefficient_pick_global_layout_perdrawcall_vertex_blending_storage_buffer_binding =
            mesh_inefficient_pick_global_layout_bindings[2];
        mesh_inefficient_pick_global_layout_perdrawcall_vertex_blending_storage_buffer_binding.binding = 2;
        mesh_inefficient_pick_global_layout_perdrawcall_vertex_blending_storage_buffer_binding.descriptorType =
            RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_inefficient_pick_global_layout_perdrawcall_vertex_blending_storage_buffer_binding.descriptorCount = 1;
        mesh_inefficient_pick_global_layout_perdrawcall_vertex_blending_storage_buffer_binding.stageFlags =
            RHI_SHADER_STAGE_VERTEX_BIT;
        mesh_inefficient_pick_global_layout_perdrawcall_vertex_blending_storage_buffer_binding.pImmutableSamplers =
            NULL;

        RHIDescriptorSetLayoutCreateInfo mesh_inefficient_pick_global_layout_create_info;
        mesh_inefficient_pick_global_layout_create_info.sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        mesh_inefficient_pick_global_layout_create_info.pNext = NULL;
        mesh_inefficient_pick_global_layout_create_info.flags = 0;
        mesh_inefficient_pick_global_layout_create_info.bindingCount =
            (sizeof(mesh_inefficient_pick_global_layout_bindings) /
                sizeof(mesh_inefficient_pick_global_layout_bindings[0]));
        mesh_inefficient_pick_global_layout_create_info.pBindings = mesh_inefficient_pick_global_layout_bindings;

        if (RHI_SUCCESS != m_RenderCommand->CreateDescriptorSetLayout(&mesh_inefficient_pick_global_layout_create_info, m_DescriptorInfos[0].layout))

        {
            throw std::runtime_error("create mesh inefficient pick global layout");
        }
    }
    void PickPass::SetupPipelines()
    {
        m_RenderPipelines.resize(1);

        RHIDescriptorSetLayout* descriptorset_layouts[] = { m_DescriptorInfos[0].layout, _per_mesh_layout };

        RHIPipelineLayoutCreateInfo pipeline_layout_create_info{};
        pipeline_layout_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_create_info.setLayoutCount = sizeof(descriptorset_layouts) / sizeof(descriptorset_layouts[0]);
        pipeline_layout_create_info.pSetLayouts = descriptorset_layouts;

        if (m_RenderCommand->CreatePipelineLayout(&pipeline_layout_create_info, m_RenderPipelines[0].layout) != RHI_SUCCESS)
        {
            throw std::runtime_error("create mesh inefficient pick pipeline layout");
        }

        RHIShader* vert_shader_module =
            m_RenderCommand->CreateShaderModule(MESH_VERT);
        RHIShader* frag_shader_module =
            m_RenderCommand->CreateShaderModule(MESH_VERT);

        RHIPipelineShaderStageCreateInfo vert_pipeline_shader_stage_create_info{};
        vert_pipeline_shader_stage_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vert_pipeline_shader_stage_create_info.stage = RHI_SHADER_STAGE_VERTEX_BIT;
        vert_pipeline_shader_stage_create_info.module = vert_shader_module;
        vert_pipeline_shader_stage_create_info.pName = "main";

        RHIPipelineShaderStageCreateInfo frag_pipeline_shader_stage_create_info{};
        frag_pipeline_shader_stage_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        frag_pipeline_shader_stage_create_info.stage = RHI_SHADER_STAGE_FRAGMENT_BIT;
        frag_pipeline_shader_stage_create_info.module = frag_shader_module;
        frag_pipeline_shader_stage_create_info.pName = "main";

        RHIPipelineShaderStageCreateInfo shader_stages[] = { vert_pipeline_shader_stage_create_info,
                                                           frag_pipeline_shader_stage_create_info };

        auto vertex_binding_descriptions = MeshVertex::GetBindingDescriptions();
        auto vertex_attribute_descriptions = MeshVertex::GetAttributeDescriptions();
        RHIPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
        vertex_input_state_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_state_create_info.vertexBindingDescriptionCount = 1;
        vertex_input_state_create_info.pVertexBindingDescriptions = &vertex_binding_descriptions[0];
        vertex_input_state_create_info.vertexAttributeDescriptionCount = 1;
        vertex_input_state_create_info.pVertexAttributeDescriptions = &vertex_attribute_descriptions[0];

        RHIPipelineInputAssemblyStateCreateInfo input_assembly_create_info{};
        input_assembly_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_create_info.topology = RHI_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly_create_info.primitiveRestartEnable = RHI_FALSE;

        RHIPipelineViewportStateCreateInfo viewport_state_create_info{};
        viewport_state_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state_create_info.viewportCount = 1;
        viewport_state_create_info.pViewports = m_RenderCommand->GetSwapchainInfo().viewport;
        viewport_state_create_info.scissorCount = 1;
        viewport_state_create_info.pScissors = m_RenderCommand->GetSwapchainInfo().scissor;

        RHIPipelineRasterizationStateCreateInfo rasterization_state_create_info{};
        rasterization_state_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization_state_create_info.depthClampEnable = RHI_FALSE;
        rasterization_state_create_info.rasterizerDiscardEnable = RHI_FALSE;
        rasterization_state_create_info.polygonMode = RHI_POLYGON_MODE_FILL;
        rasterization_state_create_info.lineWidth = 1.0f;
        rasterization_state_create_info.cullMode = RHI_CULL_MODE_BACK_BIT;
        rasterization_state_create_info.frontFace = RHI_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterization_state_create_info.depthBiasEnable = RHI_FALSE;
        rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
        rasterization_state_create_info.depthBiasClamp = 0.0f;
        rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;

        RHIPipelineMultisampleStateCreateInfo multisample_state_create_info{};
        multisample_state_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample_state_create_info.sampleShadingEnable = RHI_FALSE;
        multisample_state_create_info.rasterizationSamples = RHI_SAMPLE_COUNT_1_BIT;

        RHIPipelineColorBlendAttachmentState color_blend_attachment_state{};
        color_blend_attachment_state.colorWriteMask = RHI_COLOR_COMPONENT_R_BIT;
        color_blend_attachment_state.blendEnable = RHI_FALSE;
        color_blend_attachment_state.srcColorBlendFactor = RHI_BLEND_FACTOR_ONE;
        color_blend_attachment_state.dstColorBlendFactor = RHI_BLEND_FACTOR_ZERO;
        color_blend_attachment_state.colorBlendOp = RHI_BLEND_OP_ADD;
        color_blend_attachment_state.srcAlphaBlendFactor = RHI_BLEND_FACTOR_ONE;
        color_blend_attachment_state.dstAlphaBlendFactor = RHI_BLEND_FACTOR_ZERO;
        color_blend_attachment_state.alphaBlendOp = RHI_BLEND_OP_ADD;

        RHIPipelineColorBlendStateCreateInfo color_blend_state_create_info{};
        color_blend_state_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend_state_create_info.logicOpEnable = RHI_FALSE;
        color_blend_state_create_info.logicOp = RHI_LOGIC_OP_COPY;
        color_blend_state_create_info.attachmentCount = 1;
        color_blend_state_create_info.pAttachments = &color_blend_attachment_state;
        color_blend_state_create_info.blendConstants[0] = 0.0f;
        color_blend_state_create_info.blendConstants[1] = 0.0f;
        color_blend_state_create_info.blendConstants[2] = 0.0f;
        color_blend_state_create_info.blendConstants[3] = 0.0f;

        RHIPipelineDepthStencilStateCreateInfo depth_stencil_create_info{};
        depth_stencil_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil_create_info.depthTestEnable = RHI_TRUE;
        depth_stencil_create_info.depthWriteEnable = RHI_TRUE;
        depth_stencil_create_info.depthCompareOp = RHI_COMPARE_OP_LESS;
        depth_stencil_create_info.depthBoundsTestEnable = RHI_FALSE;
        depth_stencil_create_info.stencilTestEnable = RHI_FALSE;

        RHIDynamicState                   dynamic_states[] = { RHI_DYNAMIC_STATE_VIEWPORT, RHI_DYNAMIC_STATE_SCISSOR };
        RHIPipelineDynamicStateCreateInfo dynamic_state_create_info{};
        dynamic_state_create_info.sType = RHI_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_create_info.dynamicStateCount = 2;
        dynamic_state_create_info.pDynamicStates = dynamic_states;

        RHIGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = RHI_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shader_stages;
        pipelineInfo.pVertexInputState = &vertex_input_state_create_info;
        pipelineInfo.pInputAssemblyState = &input_assembly_create_info;
        pipelineInfo.pViewportState = &viewport_state_create_info;
        pipelineInfo.pRasterizationState = &rasterization_state_create_info;
        pipelineInfo.pMultisampleState = &multisample_state_create_info;
        pipelineInfo.pColorBlendState = &color_blend_state_create_info;
        pipelineInfo.pDepthStencilState = &depth_stencil_create_info;
        pipelineInfo.layout = m_RenderPipelines[0].layout;
        pipelineInfo.renderPass = m_FrameBuffer.render_pass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = RHI_NULL_HANDLE;
        pipelineInfo.pDynamicState = &dynamic_state_create_info;

        if (m_RenderCommand->CreateGraphicsPipelines(RHI_NULL_HANDLE, 1, &pipelineInfo, m_RenderPipelines[0].pipeline) != RHI_SUCCESS)
        {
            throw std::runtime_error("create mesh inefficient pick graphics pipeline");
        }

        m_RenderCommand->DestroyShaderModule(vert_shader_module);
        m_RenderCommand->DestroyShaderModule(frag_shader_module);
    }
    void PickPass::SetupDescriptorSet()
    {
        RHIDescriptorSetAllocateInfo mesh_inefficient_pick_global_descriptor_set_alloc_info;
        mesh_inefficient_pick_global_descriptor_set_alloc_info.sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        mesh_inefficient_pick_global_descriptor_set_alloc_info.pNext = NULL;
        mesh_inefficient_pick_global_descriptor_set_alloc_info.descriptorPool = m_RenderCommand->GetDescriptorPoor();
        mesh_inefficient_pick_global_descriptor_set_alloc_info.descriptorSetCount = 1;
        mesh_inefficient_pick_global_descriptor_set_alloc_info.pSetLayouts = &m_DescriptorInfos[0].layout;

        if (RHI_SUCCESS != m_RenderCommand->AllocateDescriptorSets(&mesh_inefficient_pick_global_descriptor_set_alloc_info, m_DescriptorInfos[0].descriptor_set))
        {
            throw std::runtime_error("allocate mesh inefficient pick global descriptor set");
        }

        RHIDescriptorBufferInfo mesh_inefficient_pick_perframe_storage_buffer_info = {};
        // this offset plus dynamic_offset should not be greater than the size of
        // the buffer
        mesh_inefficient_pick_perframe_storage_buffer_info.offset = 0;
        // the range means the size actually used by the shader per draw call
        mesh_inefficient_pick_perframe_storage_buffer_info.range =
            sizeof(MeshInefficientPickPerframeStorageBufferObject);
        mesh_inefficient_pick_perframe_storage_buffer_info.buffer =
            m_global_render_resource->_storage_buffer._global_upload_ringbuffer;

        RHIDescriptorBufferInfo mesh_inefficient_pick_perdrawcall_storage_buffer_info = {};
        mesh_inefficient_pick_perdrawcall_storage_buffer_info.offset = 0;
        mesh_inefficient_pick_perdrawcall_storage_buffer_info.range =
            sizeof(MeshInefficientPickPerdrawcallStorageBufferObject);
        mesh_inefficient_pick_perdrawcall_storage_buffer_info.buffer =
            m_global_render_resource->_storage_buffer._global_upload_ringbuffer;
        assert(mesh_inefficient_pick_perdrawcall_storage_buffer_info.range <
            m_global_render_resource->_storage_buffer._max_storage_buffer_range);

        RHIDescriptorBufferInfo mesh_inefficient_pick_perdrawcall_vertex_blending_storage_buffer_info = {};
        mesh_inefficient_pick_perdrawcall_vertex_blending_storage_buffer_info.offset = 0;
        mesh_inefficient_pick_perdrawcall_vertex_blending_storage_buffer_info.range =
            sizeof(MeshInefficientPickPerdrawcallVertexBlendingStorageBufferObject);
        mesh_inefficient_pick_perdrawcall_vertex_blending_storage_buffer_info.buffer =
            m_global_render_resource->_storage_buffer._global_upload_ringbuffer;
        assert(mesh_inefficient_pick_perdrawcall_vertex_blending_storage_buffer_info.range <
            m_global_render_resource->_storage_buffer._max_storage_buffer_range);

        RHIWriteDescriptorSet mesh_descriptor_writes_info[3];

        mesh_descriptor_writes_info[0].sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mesh_descriptor_writes_info[0].pNext = NULL;
        mesh_descriptor_writes_info[0].dstSet = m_DescriptorInfos[0].descriptor_set;
        mesh_descriptor_writes_info[0].dstBinding = 0;
        mesh_descriptor_writes_info[0].dstArrayElement = 0;
        mesh_descriptor_writes_info[0].descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_descriptor_writes_info[0].descriptorCount = 1;
        mesh_descriptor_writes_info[0].pBufferInfo = &mesh_inefficient_pick_perframe_storage_buffer_info;

        mesh_descriptor_writes_info[1].sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mesh_descriptor_writes_info[1].pNext = NULL;
        mesh_descriptor_writes_info[1].dstSet = m_DescriptorInfos[0].descriptor_set;
        mesh_descriptor_writes_info[1].dstBinding = 1;
        mesh_descriptor_writes_info[1].dstArrayElement = 0;
        mesh_descriptor_writes_info[1].descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_descriptor_writes_info[1].descriptorCount = 1;
        mesh_descriptor_writes_info[1].pBufferInfo = &mesh_inefficient_pick_perdrawcall_storage_buffer_info;

        mesh_descriptor_writes_info[2].sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mesh_descriptor_writes_info[2].pNext = NULL;
        mesh_descriptor_writes_info[2].dstSet = m_DescriptorInfos[0].descriptor_set;
        mesh_descriptor_writes_info[2].dstBinding = 2;
        mesh_descriptor_writes_info[2].dstArrayElement = 0;
        mesh_descriptor_writes_info[2].descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_descriptor_writes_info[2].descriptorCount = 1;
        mesh_descriptor_writes_info[2].pBufferInfo =
            &mesh_inefficient_pick_perdrawcall_vertex_blending_storage_buffer_info;

        m_RenderCommand->UpdateDescriptorSets(sizeof(mesh_descriptor_writes_info) / sizeof(mesh_descriptor_writes_info[0]),
            mesh_descriptor_writes_info,
            0,
            NULL);
    }
    void PickPass::RecreateFramebuffer()
    {
        for (size_t i = 0; i < m_FrameBuffer.attachments.size(); i++)
        {
            m_RenderCommand->DestroyImage(m_FrameBuffer.attachments[i].image);
            m_RenderCommand->DestroyImageView(m_FrameBuffer.attachments[i].view);
            m_RenderCommand->FreeMemory(m_FrameBuffer.attachments[i].mem);
        }
        m_RenderCommand->DestroyFramebuffer(m_FrameBuffer.framebuffer);

        SetupAttachments();
        SetupFramebuffer();
    }
    uint32_t PickPass::Pick(const glm::vec2& picked_uv)
    {
        uint32_t pixel_x =
            static_cast<uint32_t>(picked_uv.x * m_RenderCommand->GetSwapchainInfo().viewport->width + m_RenderCommand->GetSwapchainInfo().viewport->x);
        uint32_t pixel_y =
            static_cast<uint32_t>(picked_uv.y * m_RenderCommand->GetSwapchainInfo().viewport->height + m_RenderCommand->GetSwapchainInfo().viewport->y);
        uint32_t picked_pixel_index = m_RenderCommand->GetSwapchainInfo().extent.width * pixel_y + pixel_x;
        if (pixel_x >= m_RenderCommand->GetSwapchainInfo().extent.width || pixel_y >= m_RenderCommand->GetSwapchainInfo().extent.height)
            return 0;

        struct MeshNode
        {
            const glm::mat4* model_matrix{ nullptr };
            const glm::mat4* joint_matrices{ nullptr };
            uint32_t         joint_count{ 0 };
            uint32_t         node_id;
        };

        std::map<VulkanPBRMaterial*, std::map<VulkanMesh*, std::vector<MeshNode>>> main_camera_mesh_drawcall_batch;

        // reorganize mesh
        for (RenderMeshNode& node : *(m_visiable_nodes.p_main_camera_visible_mesh_nodes))
        {
            auto& mesh_instanced = main_camera_mesh_drawcall_batch[node.ref_material];
            auto& model_nodes = mesh_instanced[node.ref_mesh];

            MeshNode temp;
            temp.model_matrix = node.model_matrix;
            temp.node_id = node.node_id;
            if (node.ref_mesh->enable_vertex_blending)
            {
                temp.joint_matrices = node.joint_matrices;
                temp.joint_count = node.joint_count;
            }

            model_nodes.push_back(temp);
        }

        m_RenderCommand->PrepareContext();

        // reset storage buffer offset
        m_global_render_resource->_storage_buffer._global_upload_ringbuffers_end[m_RenderCommand->GetCurrentFrameIndex()] =
            m_global_render_resource->_storage_buffer._global_upload_ringbuffers_begin[m_RenderCommand->GetCurrentFrameIndex()];

        m_RenderCommand->WaitForFences();

        m_RenderCommand->ResetCommandPool();

        RHICommandBufferBeginInfo command_buffer_Begin_info{};
        command_buffer_Begin_info.sType = RHI_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        command_buffer_Begin_info.flags = 0;
        command_buffer_Begin_info.pInheritanceInfo = nullptr;

        bool res_Begin_command_buffer = m_RenderCommand->BeginCommandBufferPFN(
            m_RenderCommand->GetCurrentCommandBuffer(), &command_buffer_Begin_info);
        assert(RHI_SUCCESS == res_Begin_command_buffer);

        {
            RHIImageMemoryBarrier transfer_to_render_barrier{};
            transfer_to_render_barrier.sType = RHI_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            transfer_to_render_barrier.pNext = nullptr;
            transfer_to_render_barrier.srcAccessMask = 0;
            transfer_to_render_barrier.dstAccessMask = RHI_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            transfer_to_render_barrier.oldLayout = RHI_IMAGE_LAYOUT_UNDEFINED;
            transfer_to_render_barrier.newLayout = RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            transfer_to_render_barrier.srcQueueFamilyIndex = m_RenderCommand->GetQueueFamilyIndices().graphics_family.value();
            transfer_to_render_barrier.dstQueueFamilyIndex = m_RenderCommand->GetQueueFamilyIndices().graphics_family.value();
            transfer_to_render_barrier.image = m_FrameBuffer.attachments[0].image;
            transfer_to_render_barrier.subresourceRange = { RHI_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
            m_RenderCommand->CmdPipelineBarrier(m_RenderCommand->GetCurrentCommandBuffer(),
                RHI_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                RHI_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                0,
                0,
                nullptr,
                0,
                nullptr,
                1,
                &transfer_to_render_barrier);
        }

        m_RenderCommand->CmdSetViewportPFN(m_RenderCommand->GetCurrentCommandBuffer(), 0, 1, m_RenderCommand->GetSwapchainInfo().viewport);
        m_RenderCommand->CmdSetScissorPFN(m_RenderCommand->GetCurrentCommandBuffer(), 0, 1, m_RenderCommand->GetSwapchainInfo().scissor);

        RHIRenderPassBeginInfo renderpass_Begin_info{};
        renderpass_Begin_info.sType = RHI_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderpass_Begin_info.renderPass = m_FrameBuffer.render_pass;
        renderpass_Begin_info.framebuffer = m_FrameBuffer.framebuffer;
        renderpass_Begin_info.renderArea.offset = { 0, 0 };
        renderpass_Begin_info.renderArea.extent = m_RenderCommand->GetSwapchainInfo().extent;

        RHIClearColorValue color_value = { 0, 0, 0, 0 };
        RHIClearValue      clearValues[2] = { color_value, {1.0f, 0} };
        renderpass_Begin_info.clearValueCount = 2;
        renderpass_Begin_info.pClearValues = clearValues;

        m_RenderCommand->CmdBeginRenderPassPFN(m_RenderCommand->GetCurrentCommandBuffer(),
            &renderpass_Begin_info,
            RHI_SUBPASS_CONTENTS_INLINE); // no second buffer

        float color[4] = { 1.0f,1.0f,1.0f,1.0f };
        m_RenderCommand->PushEvent(m_RenderCommand->GetCurrentCommandBuffer(), "Mesh Inefficient Pick", color);

        m_RenderCommand->CmdBindPipelinePFN(m_RenderCommand->GetCurrentCommandBuffer(),
            RHI_PIPELINE_BIND_POINT_GRAPHICS,
            m_RenderPipelines[0].pipeline);
        m_RenderCommand->CmdSetViewportPFN(m_RenderCommand->GetCurrentCommandBuffer(), 0, 1, m_RenderCommand->GetSwapchainInfo().viewport);
        m_RenderCommand->CmdSetScissorPFN(m_RenderCommand->GetCurrentCommandBuffer(), 0, 1, m_RenderCommand->GetSwapchainInfo().scissor);

        // perframe storage buffer
        uint32_t perframe_dynamic_offset =
            roundUp(m_global_render_resource->_storage_buffer
                ._global_upload_ringbuffers_end[m_RenderCommand->GetCurrentFrameIndex()],
                m_global_render_resource->_storage_buffer._min_storage_buffer_offset_alignment);
        m_global_render_resource->_storage_buffer
            ._global_upload_ringbuffers_end[m_RenderCommand->GetCurrentFrameIndex()] =
            perframe_dynamic_offset + sizeof(MeshInefficientPickPerframeStorageBufferObject);
        assert(m_global_render_resource->_storage_buffer
            ._global_upload_ringbuffers_end[m_RenderCommand->GetCurrentFrameIndex()] <=
            (m_global_render_resource->_storage_buffer
                ._global_upload_ringbuffers_begin[m_RenderCommand->GetCurrentFrameIndex()] +
                m_global_render_resource->_storage_buffer
                ._global_upload_ringbuffers_size[m_RenderCommand->GetCurrentFrameIndex()]));

        (*reinterpret_cast<MeshInefficientPickPerframeStorageBufferObject*>(
            reinterpret_cast<uintptr_t>(
                m_global_render_resource->_storage_buffer._global_upload_ringbuffer_memory_pointer) +
            perframe_dynamic_offset)) = _mesh_inefficient_pick_perframe_storage_buffer_object;

        for (auto& pair1 : main_camera_mesh_drawcall_batch)
        {
            VulkanPBRMaterial& material = (*pair1.first);
            auto& mesh_instanced = pair1.second;

            // TODO: render from near to far

            for (auto& pair2 : mesh_instanced)
            {
                VulkanMesh& mesh = (*pair2.first);
                auto& mesh_nodes = pair2.second;

                uint32_t total_instance_count = static_cast<uint32_t>(mesh_nodes.size());
                if (total_instance_count > 0)
                {
                    // bind per mesh
                    m_RenderCommand->CmdBindDescriptorSetsPFN(m_RenderCommand->GetCurrentCommandBuffer(),
                        RHI_PIPELINE_BIND_POINT_GRAPHICS,
                        m_RenderPipelines[0].layout,
                        1,
                        1,
                        &mesh.mesh_vertex_blending_descriptor_set,
                        0,
                        NULL);

                    RHIBuffer* vertex_buffers[] = { mesh.mesh_vertex_position_buffer };
                    RHIDeviceSize offsets[] = { 0 };
                    m_RenderCommand->CmdBindVertexBuffersPFN(m_RenderCommand->GetCurrentCommandBuffer(),
                        0,
                        1,
                        vertex_buffers,
                        offsets);
                    m_RenderCommand->CmdBindIndexBufferPFN(m_RenderCommand->GetCurrentCommandBuffer(),
                        mesh.mesh_index_buffer,
                        0,
                        RHI_INDEX_TYPE_UINT16);

                    uint32_t drawcall_max_instance_count =
                        (sizeof(MeshInefficientPickPerdrawcallStorageBufferObject::model_matrices) /
                            sizeof(MeshInefficientPickPerdrawcallStorageBufferObject::model_matrices[0]));
                    uint32_t drawcall_count =
                        roundUp(total_instance_count, drawcall_max_instance_count) / drawcall_max_instance_count;

                    for (uint32_t drawcall_index = 0; drawcall_index < drawcall_count; ++drawcall_index)
                    {
                        uint32_t current_instance_count =
                            ((total_instance_count - drawcall_max_instance_count * drawcall_index) <
                                drawcall_max_instance_count) ?
                            (total_instance_count - drawcall_max_instance_count * drawcall_index) :
                            drawcall_max_instance_count;

                        // perdrawcall storage buffer
                        uint32_t perdrawcall_dynamic_offset =
                            roundUp(m_global_render_resource->_storage_buffer
                                ._global_upload_ringbuffers_end[m_RenderCommand->GetCurrentFrameIndex()],
                                m_global_render_resource->_storage_buffer._min_storage_buffer_offset_alignment);
                        m_global_render_resource->_storage_buffer
                            ._global_upload_ringbuffers_end[m_RenderCommand->GetCurrentFrameIndex()] =
                            perdrawcall_dynamic_offset + sizeof(MeshInefficientPickPerdrawcallStorageBufferObject);
                        assert(m_global_render_resource->_storage_buffer
                            ._global_upload_ringbuffers_end[m_RenderCommand->GetCurrentFrameIndex()] <=
                            (m_global_render_resource->_storage_buffer
                                ._global_upload_ringbuffers_begin[m_RenderCommand->GetCurrentFrameIndex()] +
                                m_global_render_resource->_storage_buffer
                                ._global_upload_ringbuffers_size[m_RenderCommand->GetCurrentFrameIndex()]));

                        MeshInefficientPickPerdrawcallStorageBufferObject& perdrawcall_storage_buffer_object =
                            (*reinterpret_cast<MeshInefficientPickPerdrawcallStorageBufferObject*>(
                                reinterpret_cast<uintptr_t>(m_global_render_resource->_storage_buffer
                                    ._global_upload_ringbuffer_memory_pointer) +
                                perdrawcall_dynamic_offset));
                        for (uint32_t i = 0; i < current_instance_count; ++i)
                        {
                            perdrawcall_storage_buffer_object.model_matrices[i] =
                                *mesh_nodes[drawcall_max_instance_count * drawcall_index + i].model_matrix;
                            perdrawcall_storage_buffer_object.node_ids[i] =
                                mesh_nodes[drawcall_max_instance_count * drawcall_index + i].node_id;
                        }

                        // per drawcall vertex blending storage buffer
                        uint32_t per_drawcall_vertex_blending_dynamic_offset;
                        if (mesh.enable_vertex_blending)
                        {
                            per_drawcall_vertex_blending_dynamic_offset =
                                roundUp(m_global_render_resource->_storage_buffer
                                    ._global_upload_ringbuffers_end[m_RenderCommand->GetCurrentFrameIndex()],
                                    m_global_render_resource->_storage_buffer._min_storage_buffer_offset_alignment);
                            m_global_render_resource->_storage_buffer
                                ._global_upload_ringbuffers_end[m_RenderCommand->GetCurrentFrameIndex()] =
                                per_drawcall_vertex_blending_dynamic_offset +
                                sizeof(MeshInefficientPickPerdrawcallVertexBlendingStorageBufferObject);
                            assert(m_global_render_resource->_storage_buffer
                                ._global_upload_ringbuffers_end[m_RenderCommand->GetCurrentFrameIndex()] <=
                                (m_global_render_resource->_storage_buffer
                                    ._global_upload_ringbuffers_begin[m_RenderCommand->GetCurrentFrameIndex()] +
                                    m_global_render_resource->_storage_buffer
                                    ._global_upload_ringbuffers_size[m_RenderCommand->GetCurrentFrameIndex()]));

                            MeshInefficientPickPerdrawcallVertexBlendingStorageBufferObject&
                                per_drawcall_vertex_blending_storage_buffer_object =
                                (*reinterpret_cast<
                                    MeshInefficientPickPerdrawcallVertexBlendingStorageBufferObject*>(
                                        reinterpret_cast<uintptr_t>(m_global_render_resource->_storage_buffer
                                            ._global_upload_ringbuffer_memory_pointer) +
                                        per_drawcall_vertex_blending_dynamic_offset));
                            for (uint32_t i = 0; i < current_instance_count; ++i)
                            {
                                for (uint32_t j = 0;
                                    j < mesh_nodes[drawcall_max_instance_count * drawcall_index + i].joint_count;
                                    ++j)
                                {
                                    per_drawcall_vertex_blending_storage_buffer_object
                                        .joint_matrices[s_mesh_vertex_blending_max_joint_count * i + j] =
                                        mesh_nodes[drawcall_max_instance_count * drawcall_index + i].joint_matrices[j];
                                }
                            }
                        }
                        else
                        {
                            per_drawcall_vertex_blending_dynamic_offset = 0;
                        }

                        // bind perdrawcall
                        uint32_t dynamic_offsets[3] = { perframe_dynamic_offset,
                                                       perdrawcall_dynamic_offset,
                                                       per_drawcall_vertex_blending_dynamic_offset };
                        m_RenderCommand->CmdBindDescriptorSetsPFN(m_RenderCommand->GetCurrentCommandBuffer(),
                            RHI_PIPELINE_BIND_POINT_GRAPHICS,
                            m_RenderPipelines[0].layout,
                            0,
                            1,
                            &m_DescriptorInfos[0].descriptor_set,
                            sizeof(dynamic_offsets) / sizeof(dynamic_offsets[0]),
                            dynamic_offsets);

                        m_RenderCommand->CmdDrawIndexedPFN(m_RenderCommand->GetCurrentCommandBuffer(),
                            mesh.mesh_index_count,
                            current_instance_count,
                            0,
                            0,
                            0);
                    }
                }
            }
        }

        m_RenderCommand->PopEvent(m_RenderCommand->GetCurrentCommandBuffer());

        // end render pass
        m_RenderCommand->CmdEndRenderPassPFN(m_RenderCommand->GetCurrentCommandBuffer());

        // end command buffer
        bool res_end_command_buffer = m_RenderCommand->EndCommandBufferPFN(m_RenderCommand->GetCurrentCommandBuffer());
        assert(RHI_SUCCESS == res_end_command_buffer);

        bool res_reset_fences = m_RenderCommand->ResetFencesPFN(1, &m_RenderCommand->GetFenceList()[m_RenderCommand->GetCurrentFrameIndex()]);
        assert(RHI_SUCCESS == res_reset_fences);

        RHISubmitInfo submit_info = {};
        submit_info.sType = RHI_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitSemaphores = NULL;
        submit_info.pWaitDstStageMask = 0;
        submit_info.commandBufferCount = 1;
        RHICommandBuffer* commandBuffer = m_RenderCommand->GetCurrentCommandBuffer();
        submit_info.pCommandBuffers = &commandBuffer;
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = NULL;

        bool res_queue_submit = m_RenderCommand->QueueSubmit(m_RenderCommand->GetGraphicsQueue(),
            1,
            &submit_info,
            m_RenderCommand->GetFenceList()[m_RenderCommand->GetCurrentFrameIndex()]);
        assert(RHI_SUCCESS == res_queue_submit);

        m_RenderCommand->SetCurrentFrameIndex((m_RenderCommand->GetCurrentFrameIndex() + 1) % m_RenderCommand->GetMaxFramesInFlight());

        // implicit host read barrier
        bool res_wait_for_fences = m_RenderCommand->WaitForFencesPFN(m_RenderCommand->GetMaxFramesInFlight(),
            m_RenderCommand->GetFenceList(),
            RHI_TRUE,
            UINT64_MAX);
        assert(RHI_SUCCESS == res_wait_for_fences);

        auto command_buffer = m_RenderCommand->BeginSingleTimeCommands();

        RHIBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = RHI_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { m_RenderCommand->GetSwapchainInfo().extent.width, m_RenderCommand->GetSwapchainInfo().extent.height, 1 };

        uint32_t   buffer_size = m_RenderCommand->GetSwapchainInfo().extent.width * m_RenderCommand->GetSwapchainInfo().extent.height * 4;
        RHIBuffer* inefficient_staging_buffer;
        RHIDeviceMemory* inefficient_staging_buffer_memory;
        m_RenderCommand->CreateBuffer(buffer_size,
            RHI_BUFFER_USAGE_TRANSFER_DST_BIT,
            RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            inefficient_staging_buffer,
            inefficient_staging_buffer_memory);

        RHIImageMemoryBarrier copy_to_buffer_barrier{};
        copy_to_buffer_barrier.sType = RHI_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        copy_to_buffer_barrier.pNext = nullptr;
        copy_to_buffer_barrier.srcAccessMask = RHI_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        copy_to_buffer_barrier.dstAccessMask = RHI_ACCESS_TRANSFER_READ_BIT;
        copy_to_buffer_barrier.oldLayout = RHI_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        copy_to_buffer_barrier.newLayout = RHI_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        copy_to_buffer_barrier.srcQueueFamilyIndex = m_RenderCommand->GetQueueFamilyIndices().graphics_family.value();
        copy_to_buffer_barrier.dstQueueFamilyIndex = m_RenderCommand->GetQueueFamilyIndices().graphics_family.value();
        copy_to_buffer_barrier.image = m_FrameBuffer.attachments[0].image;
        copy_to_buffer_barrier.subresourceRange = { RHI_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        m_RenderCommand->CmdPipelineBarrier(command_buffer,
            RHI_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            RHI_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &copy_to_buffer_barrier);

        m_RenderCommand->CmdCopyImageToBuffer(command_buffer,
            m_FrameBuffer.attachments[0].image,
            RHI_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            inefficient_staging_buffer,
            1,
            &region);

        m_RenderCommand->EndSingleTimeCommands(command_buffer);

        uint32_t* data = nullptr;
        m_RenderCommand->MapMemory(inefficient_staging_buffer_memory, 0, buffer_size, 0, (void**)&data);

        uint32_t node_id = data[picked_pixel_index];
        m_RenderCommand->UnmapMemory(inefficient_staging_buffer_memory);

        m_RenderCommand->DestroyBuffer(inefficient_staging_buffer);
        m_RenderCommand->FreeMemory(inefficient_staging_buffer_memory);

        return node_id;
    }
} // namespace Piccolo
