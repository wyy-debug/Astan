#include "aspch.h"
#include "RenderPass.h"

Astan::VisiableNodes Astan::RenderPass::m_visiable_nodes;
namespace Astan
{

	void RenderPass::Initialize(const RenderPassInitInfo* init_info)
	{
		m_global_render_resource =
			&(std::static_pointer_cast<RenderSource>(m_render_resource)->m_GlobalRenderResource);
	}

	void RenderPass::Draw()
	{
	}

	void RenderPass::SetCommonInfo(Ref<VulkanRendererAPI> renderCommand)
	{
		m_RenderCommand = renderCommand;
	}

};