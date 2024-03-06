#include "aspch.h"
#include "RenderSystem.h"
namespace Astan
{
	RenderSystem::~RenderSystem()
	{
	}

	// 序列化后处理
	void RenderSystem::Initialize()
	{
		// 初始化 RHI
		m_RenderCommand = CreateRef<VulkanRendererAPI>();
		m_RenderCommand->Initialize();
		
		// 设置摄像头
		m_RenderCamera = CreateRef<EditorCamera>();

		// 设置渲染场景
		m_RenderScene = CreateRef<Scene>();

		// 初始化渲染管线
		m_RenderPipeline = CreateRef<RenderPipeline>();
		m_RenderPipeline->m_RenderCommand = m_RenderCommand;
		m_RenderPipeline->Initialize();

	}


	void RenderSystem::Tick()
	{
	}
}