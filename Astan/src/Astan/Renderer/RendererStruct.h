#pragma once

#include <optional>
namespace Astan
{
	// render type
	enum RHIStructureType : int
	{
		RHI_STRUCTURE_TYPE_APPLICATION_INFO = 0
	};

	enum RHIDefaultSamplerType
	{
		Default_Sampler_Linear,
		Default_Sampler_Nearest
	};


	typedef uint32_t RHIAccessFlags;
	typedef uint32_t RHIMemoryPropertyFlags;
	typedef uint32_t RHIBufferUsageFlags;
	typedef uint64_t RHIDeviceSize;


	/// Class
	class RHIBuffer {};
	class RHIBufferView {};
	class RHICommandBuffer {};
	class RHIDescriptorPool {};
	class RHIDescriptorSet {};
	class RHIDescriptorSetLayout {};
	class RHIDevice {};
	class RHIDeviceMemory {};
	class RHIEvent {};
	class RHIFence {};
	class RHIFramebuffer {};
	class RHIImage {};
	class RHIImageView {};
	class RHIInstance {};
	class RHIQueue {};
	class RHIPhysicalDevice {};
	class RHIPipeline {};
	class RHIPipelineCache {};
	class RHIPipelineLayout {};
	class RHIRenderPass {};
	class RHISampler {};
	class RHISemaphore {};
	class RHIShader {};

	/// Struct
	struct RHICommandBufferAllocateInfo
	{
		RHIStructureType sType;
		const void* pNext;
		RHIAccessFlags srcAccessMask;
		RHIAccessFlags dstAccessMask;
	};

	struct RHIDescriptorSetAllocateInfo
	{
		RHIStructureType sType;
		const void* pNext;
		RHIDescriptorPool* descriptorPool;
		uint32_t descriptorSetCount;
		const RHIDescriptorSetLayout* const* pSetLayouts;
	};
}