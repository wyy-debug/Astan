#pragma once
#include "RenderPipeline.h"
#include "Astan/Scene/Scene.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"
#include "RenderResourceBase.h"
#include "RenderSwapContext.h"

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
		RenderSwapContext m_SwapContext;

		Ref<VulkanRendererAPI> m_RenderCommand;
		Ref<RenderPipeline> m_RenderPipeline;
		Ref<EditorCamera> m_RenderCamera;
		Ref<Scene> m_RenderScene;
		Ref<RenderResourceBase> m_RenderResource;

	private:
		void ProcessSwapData();
		
	};
}