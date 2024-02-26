#include "aspch.h"
#include "Texture.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"
#include "Platform/Vulkan/VulkanTexture.h"

namespace Astan
{
	Ref<Texture2D> Texture2D::Create(const TextureSpecification& specification)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: AS_CORE_ASSERT(false, "RendererAPI::Nnoe is currently not supported"); return nullptr;
		case RendererAPI::API::OpenGL: return CreateRef<OpenGLTexture2D>(specification);
		case RendererAPI::API::Vulkan: return CreateRef<VulkanTexture2D>(specification);
		}
		AS_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}


	Ref<Texture2D> Texture2D::Create(const std::string& path)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: AS_CORE_ASSERT(false, "RendererAPI::Nnoe is currently not supported"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLTexture2D>(path);
			case RendererAPI::API::Vulkan: return CreateRef<VulkanTexture2D>(path);
		}
		AS_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
	
	
}