#pragma once
#include "RenderPipeline.h"
#include "Astan/Scene/Scene.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"

namespace Astan
{
	class RenderSystem
	{
	public:
		RenderSystem() = default;
		~RenderSystem();
		void Initialize();
		void Tick(Scene scene);
	private:
		Ref<VulkanRendererAPI> m_RenderCommand;
		Ref<RenderPipeline> m_RenderPipeline;
		Camera m_Camera;
		Scene m_Scene;
		
	};
}