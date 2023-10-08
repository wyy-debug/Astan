#include "aspch.h"
#include "VertexArray.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"
namespace Astan 
{
	VertexArray* VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::None: AS_CORE_ASSERT(false, "RendererAPI::Nnoe is currently not supported"); return nullptr;
			case RendererAPI::OpenGL: return new OpenGLVertexArray();
		}
		AS_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}