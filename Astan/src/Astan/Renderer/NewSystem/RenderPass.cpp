#include "aspch.h"
#include "RenderPass.h"

void Astan::RenderPass::Initialize(const RenderPassInitInfo* init_info)
{
	m_global_render_resource =
		&(std::static_pointer_cast<RenderResource>(m_render_resource)->m_global_render_resource);
}

void Astan::RenderPass::Draw()
{
}

void Astan::RenderPass::SetCommonInfo(Ref<VulkanRendererAPI> renderCommand)
{
	m_RenderCommand = renderCommand;
}
