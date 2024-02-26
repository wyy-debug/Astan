#include "aspch.h"
#include "Framebuffer.h"
#include "Astan/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"
namespace Astan
{
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: AS_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLFramebuffer>(spec);
			case RendererAPI::API::Vulkan: return CreateRef<VulkanFramebuffer>(spec);
		}
		AS_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}