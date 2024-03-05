#pragma once
#include "MainCameraPass.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"

namespace Astan 
{
	class RenderPipeline
	{
	public:
		RenderPipeline() = default;
		~RenderPipeline();
		void Initialize();

	//TODO father class
	public:
		Ref<VulkanRendererAPI> m_RenderCommand;
	private:
		Ref<MainCameraPass> m_MainCameraPass;
	};
}