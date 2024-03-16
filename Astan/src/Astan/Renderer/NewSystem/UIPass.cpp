#include "aspch.h"
#include "UIPass.h"
#include <examples/imgui_impl_vulkan.h>
#include <examples/imgui_impl_glfw.h>

namespace Astan
{
    void UIPass::Initialize(const RenderPassInitInfo* init_info)
    {
        RenderPass::Initialize(nullptr);

        m_FrameBuffer.render_pass = static_cast<const UIPassInitInfo*>(init_info)->render_pass;
    }

    void UIPass::InitializeUIRenderBackend(WindowUI* window_ui)
    {
        m_window_ui = window_ui;

        ImGui_ImplGlfw_InitForVulkan(std::static_pointer_cast<VulkanRendererAPI>(m_RenderCommand)->m_window, true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = std::static_pointer_cast<VulkanRendererAPI>(m_RenderCommand)->m_instance;
        init_info.PhysicalDevice = std::static_pointer_cast<VulkanRendererAPI>(m_RenderCommand)->m_physical_device;
        init_info.Device = std::static_pointer_cast<VulkanRendererAPI>(m_RenderCommand)->m_device;
        init_info.QueueFamily = m_RenderCommand->GetQueueFamilyIndices().graphics_family.value();
        init_info.Queue = ((VulkanQueue*)m_RenderCommand->GetGraphicsQueue())->getResource();
        init_info.DescriptorPool = std::static_pointer_cast<VulkanRendererAPI>(m_RenderCommand)->m_vk_descriptor_pool;
        //init_info.Subpass = _main_camera_subpass_ui;

        // may be different from the real swapchain image count
        // see ImGui_ImplVulkanH_GetMinImageCountFromPresentMode
        init_info.MinImageCount = 3;
        init_info.ImageCount = 3;
        ImGui_ImplVulkan_Init(&init_info, ((VulkanRenderPass*)m_FrameBuffer.render_pass)->getResource());

        UploadFonts();
    }

    void UIPass::UploadFonts()
    {
        RHICommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = RHI_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = RHI_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_RenderCommand->GetCommandPoor();
        allocInfo.commandBufferCount = 1;

        RHICommandBuffer* commandBuffer = new VulkanCommandBuffer();
        if (RHI_SUCCESS != m_RenderCommand->AllocateCommandBuffers(&allocInfo, commandBuffer))
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }

        RHICommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = RHI_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = RHI_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        if (RHI_SUCCESS != m_RenderCommand->BeginCommandBuffer(commandBuffer, &beginInfo))
        {
            throw std::runtime_error("Could not create one-time command buffer!");
        }

        ImGui_ImplVulkan_CreateFontsTexture(((VulkanCommandBuffer*)commandBuffer)->getResource());

        if (RHI_SUCCESS != m_RenderCommand->EndCommandBuffer(commandBuffer))
        {
            throw std::runtime_error("failed to record command buffer!");
        }

        RHISubmitInfo submitInfo{};
        submitInfo.sType = RHI_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        m_RenderCommand->QueueSubmit(m_RenderCommand->GetGraphicsQueue(), 1, &submitInfo, RHI_NULL_HANDLE);
        m_RenderCommand->QueueWaitIdle(m_RenderCommand->GetGraphicsQueue());

        m_RenderCommand->FreeCommandBuffers(m_RenderCommand->GetCommandPoor(), 1, commandBuffer);

        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    void UIPass::Draw()
    {
        if (m_window_ui)
        {
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            //m_window_ui->preRender();

            float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            m_RenderCommand->PushEvent(m_RenderCommand->GetCurrentCommandBuffer(), "ImGUI", color);

            ImGui::Render();

            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), std::static_pointer_cast<VulkanRendererAPI>(m_RenderCommand)->m_vk_current_command_buffer);

            m_RenderCommand->PopEvent(m_RenderCommand->GetCurrentCommandBuffer());
        }
    }
} // namespace Piccolo
