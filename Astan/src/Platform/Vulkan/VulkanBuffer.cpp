#include "aspch.h"
#include "vulkan/vulkan.h"
#include "VulkanBuffer.h"
#include "VulkanRendererAPI.h"


namespace Astan
{
	VulkanVertexBuffer::VulkanVertexBuffer(uint32_t size)
	{
		Ref<VulkanRendererAPI> rhi;
		VkBufferCreateInfo bufferInfo = {};
		//buffer
	}
	VulkanVertexBuffer::VulkanVertexBuffer(float* vertices, uint32_t size)
	{
	}
	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
	}
	void VulkanVertexBuffer::Bind() const
	{
	}
	void VulkanVertexBuffer::Unbind() const
	{
	}
	void VulkanVertexBuffer::SetData(const void* data, uint32_t size)
	{
	}


	VulkanIndexBuffer::VulkanIndexBuffer(uint32_t* indices, uint32_t count)
	{
	}
	VulkanIndexBuffer::~VulkanIndexBuffer()
	{
	}
	void VulkanIndexBuffer::Bind() const
	{
	}
	void VulkanIndexBuffer::Unbind() const
	{
	}

}