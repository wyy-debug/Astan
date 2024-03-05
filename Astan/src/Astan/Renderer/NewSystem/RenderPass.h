#pragma once
#include "Platform/Vulkan/VulkanRendererAPI.h"
namespace Astan
{ 
	class RenderPass
	{
	public:
		virtual void Initialize();
		virtual void Draw();
		virtual void SetCommonInfo(Ref<VulkanRendererAPI> renderCommand);
	protected:
		Ref<VulkanRendererAPI> m_RenderCommand;
	};
}
