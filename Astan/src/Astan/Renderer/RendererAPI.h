#pragma once

#include <glm/glm.hpp>
#include <vk_mem_alloc.h>


#include "VertexArray.h"
#include "RendererStruct.h"
namespace Astan 
{
	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0, OpenGL = 1,Vulkan = 2
		};
	public:
		virtual void Init() = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;
		virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) = 0;

		virtual void SetLineWidth(float thickness) = 0;
		
		inline static API GetAPI() { return s_API; }
	

	public:
		virtual ~RendererAPI() = 0;
		// TODO some thing is missing
		virtual void Initialize() = 0;
		virtual void PrepareContext() = 0;

		virtual bool IsPointLightShadowEnable() = 0;

		// allocate and create
		virtual bool AllocateCommandBuffers(const RHICommandBufferAllocateInfo* pAllocateInfo, RHICommandBuffer* &pCommandBuffers) = 0;
		virtual bool AllocateDescriptorSets(const RHIDescriptorSetAllocateInfo* pAllocateInfo, RHIDescriptorSet*& pDescriptorSets) = 0;
		virtual void CreateSwapchain() = 0;
		virtual void RecreateSwapchain() = 0;
		virtual void CreateSwapchainImageViews() = 0;
		virtual void CreateFramebufferImageAndView() = 0; 
		virtual RHISampler* GetOrCreateDefaultSampler(RHIDefaultSamplerType type) = 0;
		virtual RHISampler* GetOrCreateMipmapSampler(uint32_t width, uint32_t height) = 0;
		virtual RHISampler* CreateShaderModule(const std::vector<unsigned char>& shader_code) = 0;
		virtual void CreateBuffer(RHIDeviceSize size, RHIBufferUsageFlags usage, RHIMemoryPropertyFlags properties, RHIBuffer*& buffer, RHIDeviceMemory* &bufferMemory) = 0;
		virtual void CreateBufferAndInitialize(RHIBufferUsageFlags usage, RHIMemoryPropertyFlags properties, RHIBuffer*& buffer, RHIDeviceMemory*& bufferMemory, RHIDeviceSize size, void* data = nullptr, int datasize = 0) = 0;
		virtual bool CreateBufferVMA(VmaAllocator allocator,
			const RHIBufferCreateInfo* pBufferCreateInfo,
			const VmaAllocationCreateInfo* pAllocationCreateInfo,
			RHIBuffer*& pBuffer,
			VmaAllocation* pAllocation,
			VmaAllocationInfo* pAllocationInfo) = 0;

	private:
		static API s_API;
	};
}