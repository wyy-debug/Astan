#include "EditorLayer.h"
#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <chrono>

static const uint32_t s_MapWidth = 24;
static const char* s_MapTiles = 
"WWWWWWWWWWWWWWWWWWWWWWWW"
"WWWWWWDWWDDDDWDWWWWWWWWW"
"WWWWWDDWWDDDDDWWWWWWWWWW"
"WWWWDWWWDDDDDDDDDWWWWWWW"
"WWWDDDDDDDDDDDDDDWWWWWWW"
"WWDDDDDDDDDDDDWWWWWWWWWW"
"WDDDDDDDDDDDDDWWWWWWWWWW"
"WWDWWWWWWWWWWWWWWWEWWWWW"
"WWWDWWWWWWWWWWWWWWWWWWWW"
"WWWWDWWWWWWWWWWWWWWWWWWW"
"WWWWWDWWWWWWWWWWWWWWWWWW"
"WWWWWWDWWWWWWWWWWWWWWWWW"
"WWWWWWWWWWWWWWWWWWWWWWWW"
"WWWWWWWWWWWWWWWWWWWWWWWW";
namespace Astan {
	EditorLayer::EditorLayer()
		:Layer("EditorLayer"), m_CameraController(1280.0f / 720.0f)
	{

	}


	void EditorLayer::OnAttach()
	{
		AS_PROFILE_FUNCTION();
 
		m_CheckerboardTexture = Astan::Texture2D::Create("assets/textures/Checkerboard.png");
		m_SpriteSheet = Astan::Texture2D::Create("assets/game/textures/RPGpack_sheet_2X.png");
	
		m_TextureStaris = Astan::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 1,11 }, { 128,128 });
		m_TextureTree = Astan::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 2,1 }, { 128,128 }, { 1,2 });
	
		m_MapWith = s_MapWidth;
		m_MapHeight = strlen(s_MapTiles) / s_MapWidth;

		s_TextureMap['D'] = Astan::SubTexture2D::CreateFromCoords(m_SpriteSheet, {6,11}, {128,128});
		s_TextureMap['W'] = Astan::SubTexture2D::CreateFromCoords(m_SpriteSheet, {11,11}, {128,128});

		m_CameraController.SetZoomLevel(1.0f);

		Astan::FramebufferSpecification fbSpec;
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		m_Framebuffer = Astan::Framebuffer::Create(fbSpec);
	}

	void EditorLayer::OnDetach()
	{}


	void EditorLayer::OnUpdate(Astan::Timestep ts)
	{ 
		AS_PROFILE_FUNCTION();
		//Update
		m_CameraController.OnUpdate(ts);

		//Reset stats here
		Astan::Renderer2D::ResetStats();
		//Render
		{
			AS_PROFILE_SCOPE("Render Prep");
			m_Framebuffer->Bind();
			Astan::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
			Astan::RenderCommand::Clear();
		}


		{
			static float rotation = 0.0f;
			rotation += ts * 50.0f;

			AS_PROFILE_SCOPE("Render Draw");
			Astan::Renderer2D::BeginScene(m_CameraController.GetCamera());
			Astan::Renderer2D::DrawRotatedQuad({ 1.0f,0.0f }, { 0.8f,0.8f }, glm::radians(- 45.0f), {0.8f,0.2f,0.3f,1.0f});
			Astan::Renderer2D::DrawQuad({ -1.0f,0.0f }, { 0.8f,0.8f }, { 0.8f,0.2f,0.3f,1.0f });
			Astan::Renderer2D::DrawQuad({ 0.5f,-0.5f }, { 0.5f,0.75f }, { 0.2f,0.3f,0.8f,1.0f });
			Astan::Renderer2D::DrawQuad({ 0.0f,0.0f,-0.1f }, {20.0f,20.0f}, m_CheckerboardTexture, 10.0f);
			Astan::Renderer2D::DrawRotatedQuad({ -2.0f,0.5f,0.0f }, { 1.0f,1.0f }, glm::radians(rotation), m_CheckerboardTexture, 20.0f);
			Astan::Renderer2D::EndScene();

			Astan::Renderer2D::BeginScene(m_CameraController.GetCamera());
			for (float y = -5.0f; y < 5.0f; y+=0.5f)
			{
				for (float x = -5.0f; x < 5.0f; x+= 0.5f)
				{
					glm::vec4 color = { (x + 5.0f) / 10.f,0.4f,(y + 5.0f) / 10.f,0.7f };
					Astan::Renderer2D::DrawQuad({ x,y }, { 0.45f,0.45f }, color);
				}
			}
			Astan::Renderer2D::EndScene();
			m_Framebuffer->Unbind();
		}
	#if 0
		for (uint32_t y = 0; y < m_MapHeight; y++)
		{
			for (uint32_t x = 0; x < m_MapWith; x++)
			{
				char tileType = s_MapTiles[x + y * m_MapWith];
				Astan::Ref<Astan::SubTexture2D> texture;
				if (s_TextureMap.find(tileType) != s_TextureMap.end())
					texture = s_TextureMap[tileType];
				else
					texture = m_TextureStaris;

				Astan::Renderer2D::DrawQuad({ x-m_MapWith / 2.0f,m_MapHeight - y - m_MapHeight/2.0f,0.5f }, { 1.0f,1.0f }, texture);
			}
		}
	#endif

	}

	void EditorLayer::OnImGuiRender()
	{
		AS_PROFILE_FUNCTION();

		static bool dockingEnable = true;
		if (dockingEnable)
		{
			static bool dockspaceOpen = true;
			static bool opt_fullscreen_persistant = true;
			bool opt_fullscreen = opt_fullscreen_persistant;
			static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

			ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
			if(opt_fullscreen)
			{
				ImGuiViewport* viewport = ImGui::GetWindowViewport();
				ImGui::SetNextWindowPos(viewport->Pos);
				ImGui::SetNextWindowSize(viewport->Size);
				ImGui::SetNextWindowViewport(viewport->ID);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
				window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
				window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
			}

			if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
				window_flags |= ImGuiWindowFlags_NoBackground;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
			ImGui::PopStyleVar();

			if (opt_fullscreen)
				ImGui::PopStyleVar(2);
	
			ImGuiIO& io = ImGui::GetIO();
			if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
			{
				ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
				ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
			}

			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("Exit")) Astan::Application::Get().Close();
						ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}
			ImGui::Begin("Setting");

			auto stats = Astan::Renderer2D::GetStats();
			ImGui::Text("Renderer2D Stats:");
			ImGui::Text("Draw Calls: %d", stats.DrawCalls);
			ImGui::Text("Quads: %d", stats.QuadCount);
			ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
			ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
			
			ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
			
			ImGui::End();

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0,0 });
			ImGui::Begin("Viewport");
			ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
			if (m_ViewportSize != *(glm::vec2*)&viewportPanelSize)
			{
				m_Framebuffer->Resize(uint32_t(viewportPanelSize.x),uint32_t(viewportPanelSize.y));
				m_ViewportSize = { viewportPanelSize.x,viewportPanelSize.y };
				m_CameraController.OnResize(viewportPanelSize.x, viewportPanelSize.y);
			}
			uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
			ImGui::Image((void*)textureID, ImVec2{ m_ViewportSize.x,m_ViewportSize.y }, ImVec2{ 0,1 }, ImVec2{ 1,0 });
			ImGui::End();
			ImGui::PopStyleVar();


			ImGui::End();
		}
		else
		{
			ImGui::Begin("Setting");

			auto stats = Astan::Renderer2D::GetStats();
			ImGui::Text("Renderer2D Stats:");
			ImGui::Text("Draw Calls: %d", stats.DrawCalls);
			ImGui::Text("Quads: %d", stats.QuadCount);
			ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
			ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

			ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));

			uint32_t textureID = m_CheckerboardTexture->GetRendererID();
			ImGui::Image((void*)textureID, ImVec2{ 256.0f,256.0f });
			ImGui::End();
		}
	}

	void EditorLayer::OnEvent(Astan::Event& event)
	{
		m_CameraController.OnEvent(event);
	}
}