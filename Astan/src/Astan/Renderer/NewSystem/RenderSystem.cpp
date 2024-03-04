#include "aspch.h"
#include "RenderSystem.h"
namespace Astan
{
	RenderSystem::~RenderSystem()
	{
	}


	void RenderSystem::Initialize()
	{
		m_Vulkan = CreateRef<VulkanRendererAPI>();
		m_Vulkan->Initialize();
		// camera,scene is missing
	}


	void RenderSystem::Tick(Scene scene)
	{
	}
}