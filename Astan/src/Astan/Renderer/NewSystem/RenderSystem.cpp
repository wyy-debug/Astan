#include "aspch.h"
#include "RenderSystem.h"
namespace Astan
{
	RenderSystem::~RenderSystem()
	{
	}

	// ���л�����
	void RenderSystem::Initialize()
	{
		// ��ʼ�� RHI
		m_RenderCommand = CreateRef<VulkanRendererAPI>();
		m_RenderCommand->Initialize();
		
		// ��������ͷ
		m_RenderCamera = CreateRef<EditorCamera>();

		// ������Ⱦ����
		m_RenderScene = CreateRef<Scene>();

		// ��ʼ����Ⱦ����
		m_RenderPipeline = CreateRef<RenderPipeline>();
		m_RenderPipeline->m_RenderCommand = m_RenderCommand;
		m_RenderPipeline->Initialize();

	}


	void RenderSystem::Tick()
	{
	}
}