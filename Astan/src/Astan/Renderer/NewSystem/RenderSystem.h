#pragma once
#include "RenderPipeline.h"
#include "RenderSource.h"
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
		void Tick();
	private:
		Ref<VulkanRendererAPI> m_RenderCommand;
		Ref<RenderPipeline> m_RenderPipeline;
		Ref<EditorCamera> m_RenderCamera;
		Ref<Scene> m_RenderScene;
		Ref<RenderSource> m_RenderResource;
		

	private:
		void ProcessSwapData();
		
	};
}