#pragma once

#include "Astan/Renderer/UniformBuffer.h"

namespace Astan {

	class VulkanUniformBuffer : public UniformBuffer
	{
	public:
		VulkanUniformBuffer(uint32_t size, uint32_t binding);
		virtual ~VulkanUniformBuffer();

		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;
	private:
		uint32_t m_RendererID = 0;
	};
}