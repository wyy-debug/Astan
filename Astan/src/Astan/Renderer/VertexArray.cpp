#include "aspch.h"
#include "VertexArray.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"
#include "Platform/Vulkan/VulkanVertexArray.h"
namespace Astan 
{
	Ref<VertexArray> VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: AS_CORE_ASSERT(false, "RendererAPI::Nnoe is currently not supported"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLVertexArray>();
			case RendererAPI::API::Vulkan: return CreateRef<VulkanVertexArray>();
		}
		AS_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}