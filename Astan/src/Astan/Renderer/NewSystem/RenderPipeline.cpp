#include "aspch.h"
#include "RenderPipeline.h"

namespace Astan
{
	RenderPipeline::~RenderPipeline()
	{
	}

	void RenderPipeline::Initialize()
	{
		m_MainCameraPass = CreateRef<MainCameraPass>();

		m_MainCameraPass->SetCommonInfo(m_RenderCommand);


	}
}