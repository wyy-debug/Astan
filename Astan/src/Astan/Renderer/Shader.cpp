#include "aspch.h"
#include "shader.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"
namespace Astan
{
	Shader* Shader::Create(const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: AS_CORE_ASSERT(false, "RendererAPI::Nnoe is currently not supported"); return nullptr;
		case RendererAPI::API::OpenGL: return new OpenGLShader(vertexSrc, fragmentSrc);
		}
		AS_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}