#include "aspch.h"
#include "RenderPass.h"

void Astan::RenderPass::Initialize()
{
}

void Astan::RenderPass::Draw()
{
}

void Astan::RenderPass::SetCommonInfo(Ref<VulkanRendererAPI> renderCommand)
{
	m_RenderCommand = renderCommand;
}
