#pragma once

#include "Astan/Core/Application.h"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <functional>
#include <map>
#include <vector>
#include "VulkanRHIResource.h"

namespace Astan
{
    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR        capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR>   presentModes;
    };

	class VulkanRendererAPI :public RendererAPI
	{
	public:
		virtual void Init() override;
        virtual ~VulkanRendererAPI() override final;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;
		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) override;
		virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) override;
		virtual void SetLineWidth(float thickness) override;
		void SetWindowHandle();
	public:

		GLFWwindow* m_window{nullptr};
	public:
        // initialize
        virtual void Initialize() override final;
        virtual void PrepareContext() override final;

        // allocate and create
        bool AllocateCommandBuffers(const RHICommandBufferAllocateInfo* pAllocateInfo, RHICommandBuffer*& pCommandBuffers) override;
        bool AllocateDescriptorSets(const RHIDescriptorSetAllocateInfo* pAllocateInfo, RHIDescriptorSet*& pDescriptorSets) override;
        void CreateSwapchain() override;
        void RecreateSwapchain() override;
        void CreateSwapchainImageViews() override;
        void CreateFramebufferImageAndView() override;
        RHISampler* GetOrCreateDefaultSampler(RHIDefaultSamplerType type) override;
        RHISampler* GetOrCreateMipmapSampler(uint32_t width, uint32_t height) override;
        RHIShader* CreateShaderModule(const std::vector<unsigned char>& shader_code) override;
        void CreateBuffer(RHIDeviceSize size, RHIBufferUsageFlags usage, RHIMemoryPropertyFlags properties, RHIBuffer*& buffer, RHIDeviceMemory*& buffer_memory) override;
        void CreateBufferAndInitialize(RHIBufferUsageFlags usage, RHIMemoryPropertyFlags properties, RHIBuffer*& buffer, RHIDeviceMemory*& buffer_memory, RHIDeviceSize size, void* data = nullptr, int datasize = 0) override;
        bool CreateBufferVMA(VmaAllocator allocator,
            const RHIBufferCreateInfo* pBufferCreateInfo,
            const VmaAllocationCreateInfo* pAllocationCreateInfo,
            RHIBuffer*& pBuffer,
            VmaAllocation* pAllocation,
            VmaAllocationInfo* pAllocationInfo) override;
        bool CreateBufferWithAlignmentVMA(
            VmaAllocator allocator,
            const RHIBufferCreateInfo* pBufferCreateInfo,
            const VmaAllocationCreateInfo* pAllocationCreateInfo,
            RHIDeviceSize minAlignment,
            RHIBuffer*& pBuffer,
            VmaAllocation* pAllocation,
            VmaAllocationInfo* pAllocationInfo) override;
        void CopyBuffer(RHIBuffer* srcBuffer, RHIBuffer* dstBuffer, RHIDeviceSize srcOffset, RHIDeviceSize dstOffset, RHIDeviceSize size) override;
        void CreateImage(uint32_t image_width, uint32_t image_height, RHIFormat format, RHIImageTiling image_tiling, RHIImageUsageFlags image_usage_flags, RHIMemoryPropertyFlags memory_property_flags,
            RHIImage*& image, RHIDeviceMemory*& memory, RHIImageCreateFlags image_create_flags, uint32_t array_layers, uint32_t miplevels) override;
        void CreateImageView(RHIImage* image, RHIFormat format, RHIImageAspectFlags image_aspect_flags, RHIImageViewType view_type, uint32_t layout_count, uint32_t miplevels,
            RHIImageView*& image_view) override;
        void CreateGlobalImage(RHIImage*& image, RHIImageView*& image_view, VmaAllocation& image_allocation, uint32_t texture_image_width, uint32_t texture_image_height, void* texture_image_pixels, RHIFormat texture_image_format, uint32_t miplevels = 0) override;
        void CreateCubeMap(RHIImage*& image, RHIImageView*& image_view, VmaAllocation& image_allocation, uint32_t texture_image_width, uint32_t texture_image_height, std::array<void*, 6> texture_image_pixels, RHIFormat texture_image_format, uint32_t miplevels) override;
        bool CreateCommandPool(const RHICommandPoolCreateInfo* pCreateInfo, RHICommandPool*& pCommandPool) override;
        bool CreateDescriptorPool(const RHIDescriptorPoolCreateInfo* pCreateInfo, RHIDescriptorPool*& pDescriptorPool) override;
        bool CreateDescriptorSetLayout(const RHIDescriptorSetLayoutCreateInfo* pCreateInfo, RHIDescriptorSetLayout*& pSetLayout) override;
        bool CreateFence(const RHIFenceCreateInfo* pCreateInfo, RHIFence*& pFence) override;
        bool CreateFramebuffer(const RHIFramebufferCreateInfo* pCreateInfo, RHIFramebuffer*& pFramebuffer) override;
        bool CreateGraphicsPipelines(RHIPipelineCache* pipelineCache, uint32_t createInfoCount, const RHIGraphicsPipelineCreateInfo* pCreateInfos, RHIPipeline*& pPipelines) override;
        bool CreateComputePipelines(RHIPipelineCache* pipelineCache, uint32_t createInfoCount, const RHIComputePipelineCreateInfo* pCreateInfos, RHIPipeline*& pPipelines) override;
        bool CreatePipelineLayout(const RHIPipelineLayoutCreateInfo* pCreateInfo, RHIPipelineLayout*& pPipelineLayout) override;
        bool CreateRenderPass(const RHIRenderPassCreateInfo* pCreateInfo, RHIRenderPass*& pRenderPass) override;
        bool CreateSampler(const RHISamplerCreateInfo* pCreateInfo, RHISampler*& pSampler) override;
        bool createSemaphore(const RHISemaphoreCreateInfo* pCreateInfo, RHISemaphore*& pSemaphore) override;

        // command and command write
        bool WaitForFencesPFN(uint32_t fenceCount, RHIFence* const* pFence, RHIBool32 waitAll, uint64_t timeout) override;
        bool ResetFencesPFN(uint32_t fenceCount, RHIFence* const* pFences) override;
        bool ResetCommandPoolPFN(RHICommandPool* commandPool, RHICommandPoolResetFlags flags) override;
        bool BeginCommandBufferPFN(RHICommandBuffer* commandBuffer, const RHICommandBufferBeginInfo* pBeginInfo) override;
        bool EndCommandBufferPFN(RHICommandBuffer* commandBuffer) override;
        void CmdBeginRenderPassPFN(RHICommandBuffer* commandBuffer, const RHIRenderPassBeginInfo* pRenderPassBegin, RHISubpassContents contents) override;
        void CmdNextSubpassPFN(RHICommandBuffer* commandBuffer, RHISubpassContents contents) override;
        void CmdEndRenderPassPFN(RHICommandBuffer* commandBuffer) override;
        void CmdBindPipelinePFN(RHICommandBuffer* commandBuffer, RHIPipelineBindPoint pipelineBindPoint, RHIPipeline* pipeline) override;
        void CmdSetViewportPFN(RHICommandBuffer* commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const RHIViewport* pViewports) override;
        void CmdSetScissorPFN(RHICommandBuffer* commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const RHIRect2D* pScissors) override;
        void CmdBindVertexBuffersPFN(
            RHICommandBuffer* commandBuffer,
            uint32_t firstBinding,
            uint32_t bindingCount,
            RHIBuffer* const* pBuffers,
            const RHIDeviceSize* pOffsets) override;
        void CmdBindIndexBufferPFN(RHICommandBuffer* commandBuffer, RHIBuffer* buffer, RHIDeviceSize offset, RHIIndexType indexType) override;
        void CmdBindDescriptorSetsPFN(
            RHICommandBuffer* commandBuffer,
            RHIPipelineBindPoint pipelineBindPoint,
            RHIPipelineLayout* layout,
            uint32_t firstSet,
            uint32_t descriptorSetCount,
            const RHIDescriptorSet* const* pDescriptorSets,
            uint32_t dynamicOffsetCount,
            const uint32_t* pDynamicOffsets) override;
        void CmdDrawIndexedPFN(RHICommandBuffer* commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) override;
        void CmdClearAttachmentsPFN(RHICommandBuffer* commandBuffer, uint32_t attachmentCount, const RHIClearAttachment* pAttachments, uint32_t rectCount, const RHIClearRect* pRects) override;

        bool BeginCommandBuffer(RHICommandBuffer* commandBuffer, const RHICommandBufferBeginInfo* pBeginInfo) override;
        void CmdCopyImageToBuffer(RHICommandBuffer* commandBuffer, RHIImage* srcImage, RHIImageLayout srcImageLayout, RHIBuffer* dstBuffer, uint32_t regionCount, const RHIBufferImageCopy* pRegions) override;
        void CmdCopyImageToImage(RHICommandBuffer* commandBuffer, RHIImage* srcImage, RHIImageAspectFlagBits srcFlag, RHIImage* dstImage, RHIImageAspectFlagBits dstFlag, uint32_t width, uint32_t height) override;
        void CmdCopyBuffer(RHICommandBuffer* commandBuffer, RHIBuffer* srcBuffer, RHIBuffer* dstBuffer, uint32_t regionCount, RHIBufferCopy* pRegions) override;
        void CmdDraw(RHICommandBuffer* commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;
        void CmdDispatch(RHICommandBuffer* commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;
        void CmdDispatchIndirect(RHICommandBuffer* commandBuffer, RHIBuffer* buffer, RHIDeviceSize offset) override;
        void CmdPipelineBarrier(RHICommandBuffer* commandBuffer, RHIPipelineStageFlags srcStageMask, RHIPipelineStageFlags dstStageMask, RHIDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const RHIMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const RHIBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const RHIImageMemoryBarrier* pImageMemoryBarriers) override;
        bool EndCommandBuffer(RHICommandBuffer* commandBuffer) override;
        void UpdateDescriptorSets(uint32_t descriptorWriteCount, const RHIWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const RHICopyDescriptorSet* pDescriptorCopies) override;
        bool QueueSubmit(RHIQueue* queue, uint32_t submitCount, const RHISubmitInfo* pSubmits, RHIFence* fence) override;
        bool QueueWaitIdle(RHIQueue* queue) override;
        void ResetCommandPool() override;
        void WaitForFences() override;
        bool WaitForFences(uint32_t fenceCount, const RHIFence* const* pFences, RHIBool32 waitAll, uint64_t timeout);

        // query
        void GetPhysicalDeviceProperties(RHIPhysicalDeviceProperties* pProperties) override;
        RHICommandBuffer* GetCurrentCommandBuffer() const override;
        RHICommandBuffer* const* GetCommandBufferList() const override;
        RHICommandPool* GetCommandPoor() const override;
        RHIDescriptorPool* GetDescriptorPoor()const override;
        RHIFence* const* GetFenceList() const override;
        QueueFamilyIndices GetQueueFamilyIndices() const override;
        RHIQueue* GetGraphicsQueue() const override;
        RHIQueue* GetComputeQueue() const override;
        RHISwapChainDesc GetSwapchainInfo() override;
        RHIDepthImageDesc GetDepthImageInfo() const override;
        uint8_t GetMaxFramesInFlight() const override;
        uint8_t GetCurrentFrameIndex() const override;
        void SetCurrentFrameIndex(uint8_t index) override;

        // command write
        RHICommandBuffer* BeginSingleTimeCommands() override;
        void            EndSingleTimeCommands(RHICommandBuffer* command_buffer) override;
        bool PrepareBeforePass(std::function<void()> passUpdateAfterRecreateSwapchain) override;
        void SubmitRendering(std::function<void()> passUpdateAfterRecreateSwapchain) override;
        void PushEvent(RHICommandBuffer* commond_buffer, const char* name, const float* color) override;
        void PopEvent(RHICommandBuffer* commond_buffer) override;

        // destory
        void ClearSwapchain() override;
        void DestroyDefaultSampler(RHIDefaultSamplerType type) override;
        void DestroyMipmappedSampler() override;
        void DestroyShaderModule(RHIShader* shader) override;
        void DestroySemaphore(RHISemaphore* semaphore) override;
        void DestroySampler(RHISampler* sampler) override;
        void DestroyInstance(RHIInstance* instance) override;
        void DestroyImageView(RHIImageView* imageView) override;
        void DestroyImage(RHIImage* image) override;
        void DestroyFramebuffer(RHIFramebuffer* framebuffer) override;
        void DestroyFence(RHIFence* fence) override;
        void DestroyDevice() override;
        void DestroyCommandPool(RHICommandPool* commandPool) override;
        void DestroyBuffer(RHIBuffer*& buffer) override;
        void FreeCommandBuffers(RHICommandPool* commandPool, uint32_t commandBufferCount, RHICommandBuffer* pCommandBuffers) override;

        // memory
        void FreeMemory(RHIDeviceMemory*& memory) override;
        bool MapMemory(RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size, RHIMemoryMapFlags flags, void** ppData) override;
        void UnmapMemory(RHIDeviceMemory* memory) override;
        void InvalidateMappedMemoryRanges(void* pNext, RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size) override;
        void FlushMappedMemoryRanges(void* pNext, RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size) override;

        //semaphores
        RHISemaphore*& GetTextureCopySemaphore(uint32_t index) override;

    private:
        void CreateInstance();
        void InitializeDebugMessenger();
        void CreateWindowSurface();
        void InitializePhysicalDevice();
        void CreateLogicalDevice();
        void CreateCommandPool() override;;
        void CreateCommandBuffers();
        void CreateDescriptorPool();
        void CreateSyncPrimitives();
        void CreateAssetAllocator();

    public:
        bool isPointLightShadowEnabled() override;
    

    public:
        static uint8_t const k_max_frames_in_flight{ 3 };

        RHIQueue* m_graphics_queue{ nullptr };
        RHIQueue* m_compute_queue{ nullptr };

        RHIFormat m_swapchain_image_format{ RHI_FORMAT_UNDEFINED };
        std::vector<RHIImageView*> m_swapchain_imageviews;
        RHIExtent2D m_swapchain_extent;
        RHIViewport m_viewport;
        RHIRect2D m_scissor;

        RHIFormat m_depth_image_format{ RHI_FORMAT_UNDEFINED };
        RHIImageView* m_depth_image_view = new VulkanImageView();

        RHIFence* m_RenderCommand_is_frame_in_flight_fences[k_max_frames_in_flight];

        RHIDescriptorPool* m_descriptor_pool = new VulkanDescriptorPool();

        RHICommandPool* m_RenderCommand_command_pool;

        RHICommandBuffer* m_command_buffers[k_max_frames_in_flight];
        RHICommandBuffer* m_current_command_buffer = new VulkanCommandBuffer();

        QueueFamilyIndices m_queue_indices;

        VkInstance         m_instance{ nullptr };
        VkSurfaceKHR       m_surface{ nullptr };
        VkPhysicalDevice   m_physical_device{ nullptr };
        VkDevice           m_device{ nullptr };
        VkQueue            m_present_queue{ nullptr };

        VkSwapchainKHR           m_swapchain{ nullptr };
        std::vector<VkImage>     m_swapchain_images;

        RHIImage* m_depth_image = new VulkanImage();
        VkDeviceMemory m_depth_image_memory{ nullptr };

        std::vector<VkFramebuffer> m_swapchain_framebuffers;

        // asset allocator use VMA library
        VmaAllocator m_assets_allocator;

        // function pointers
        PFN_vkCmdBeginDebugUtilsLabelEXT _vkCmdBeginDebugUtilsLabelEXT;
        PFN_vkCmdEndDebugUtilsLabelEXT   _vkCmdEndDebugUtilsLabelEXT;
        PFN_vkWaitForFences         _vkWaitForFences;
        PFN_vkResetFences           _vkResetFences;
        PFN_vkResetCommandPool      _vkResetCommandPool;
        PFN_vkBeginCommandBuffer    _vkBeginCommandBuffer;
        PFN_vkEndCommandBuffer      _vkEndCommandBuffer;
        PFN_vkCmdBeginRenderPass    _vkCmdBeginRenderPass;
        PFN_vkCmdNextSubpass        _vkCmdNextSubpass;
        PFN_vkCmdEndRenderPass      _vkCmdEndRenderPass;
        PFN_vkCmdBindPipeline       _vkCmdBindPipeline;
        PFN_vkCmdSetViewport        _vkCmdSetViewport;
        PFN_vkCmdSetScissor         _vkCmdSetScissor;
        PFN_vkCmdBindVertexBuffers  _vkCmdBindVertexBuffers;
        PFN_vkCmdBindIndexBuffer    _vkCmdBindIndexBuffer;
        PFN_vkCmdBindDescriptorSets _vkCmdBindDescriptorSets;
        PFN_vkCmdDrawIndexed        _vkCmdDrawIndexed;
        PFN_vkCmdClearAttachments   _vkCmdClearAttachments;

        // global descriptor pool
        VkDescriptorPool m_vk_descriptor_pool;

        // command pool and buffers
        uint8_t              m_current_frame_index{ 0 };
        VkCommandPool        m_command_pools[k_max_frames_in_flight];
        VkCommandBuffer      m_vk_command_buffers[k_max_frames_in_flight];
        VkSemaphore          m_image_available_for_render_semaphores[k_max_frames_in_flight];
        VkSemaphore          m_image_finished_for_presentation_semaphores[k_max_frames_in_flight];
        RHISemaphore* m_image_available_for_texturescopy_semaphores[k_max_frames_in_flight];
        VkFence              m_is_frame_in_flight_fences[k_max_frames_in_flight];

        // TODO: set
        VkCommandBuffer   m_vk_current_command_buffer;

        uint32_t m_current_swapchain_image_index;
    private:
        const std::vector<char const*> m_validation_layers{ "VK_LAYER_KHRONOS_validation" };
        uint32_t                       m_vulkan_api_version{ VK_API_VERSION_1_0 };

        std::vector<char const*> m_device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

        // default sampler cache
        RHISampler* m_linear_sampler = nullptr;
        RHISampler* m_nearest_sampler = nullptr;
        std::map<uint32_t, RHISampler*> m_mipmap_sampler_map;
    private:
        bool m_enable_validation_Layers{ true };
        bool m_enable_debug_utils_label{ true };
        bool m_enable_point_light_shadow{ true };

        // used in descriptor pool creation
        uint32_t m_max_vertex_blending_mesh_count{ 256 };
        uint32_t m_max_material_count{ 256 };

        bool                     checkValidationLayerSupport();
        std::vector<const char*> getRequiredExtensions();
        void                     populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

        VkDebugUtilsMessengerEXT m_debug_messenger = nullptr;
        VkResult                 createDebugUtilsMessengerEXT(VkInstance                                instance,
            const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
            const VkAllocationCallbacks* pAllocator,
            VkDebugUtilsMessengerEXT* pDebugMessenger);
        void                     destroyDebugUtilsMessengerEXT(VkInstance                   instance,
            VkDebugUtilsMessengerEXT     debugMessenger,
            const VkAllocationCallbacks* pAllocator);

        QueueFamilyIndices      findQueueFamilies(VkPhysicalDevice physicalm_device);
        bool                    checkDeviceExtensionSupport(VkPhysicalDevice physicalm_device);
        bool                    isDeviceSuitable(VkPhysicalDevice physicalm_device);
        SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice physicalm_device);

        VkFormat findDepthFormat();
        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
            VkImageTiling                tiling,
            VkFormatFeatureFlags         features);

        VkSurfaceFormatKHR
            chooseSwapchainSurfaceFormatFromDetails(const std::vector<VkSurfaceFormatKHR>& available_surface_formats);
        VkPresentModeKHR
            chooseSwapchainPresentModeFromDetails(const std::vector<VkPresentModeKHR>& available_present_modes);
        VkExtent2D chooseSwapchainExtentFromDetails(const VkSurfaceCapabilitiesKHR& capabilities);
	};
}