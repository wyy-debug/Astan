#pragma once
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
		Ref<VulkanRendererAPI> m_Vulkan;
		Camera m_Camera;
		Scene m_Scene;
		
	};
}