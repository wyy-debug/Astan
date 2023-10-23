#include "aspch.h"
#include "Framebuffer.h"
#include "Astan/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"
namespace Astan
{
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: AS_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLFramebuffer>(spec);
		}
		AS_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}