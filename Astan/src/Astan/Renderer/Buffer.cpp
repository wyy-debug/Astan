#include "aspch.h"
#include "Buffer.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"
#include "Platform/Vulkan/VulkanBuffer.h"

namespace Astan
{
	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: AS_CORE_ASSERT(false, "RendererAPI::Nnoe is currently not supported"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLVertexBuffer>(size);
			case RendererAPI::API::Vulkan: return CreateRef<VulkanVertexBuffer>(size);
		}
		AS_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: AS_CORE_ASSERT(false, "RendererAPI::Nnoe is currently not supported"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLVertexBuffer>(vertices, size);
			case RendererAPI::API::Vulkan: return CreateRef<VulkanVertexBuffer>(vertices, size);
		}
		AS_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: AS_CORE_ASSERT(false, "RendererAPI::Nnoe is currently not supported"); return nullptr;
		case RendererAPI::API::OpenGL: return CreateRef<OpenGLIndexBuffer>(indices, count);
		case RendererAPI::API::Vulkan: return CreateRef<VulkanIndexBuffer>(indices, count);
		}
		AS_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

}
