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
 
		m_CheckerboardTexture = Texture2D::Create("assets/textures/Checkerboard.png");
		m_SpriteSheet = Texture2D::Create("assets/game/textures/RPGpack_sheet_2X.png");
	
		m_TextureStaris = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 1,11 }, { 128,128 });
		m_TextureTree = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 2,1 }, { 128,128 }, { 1,2 });
	
		m_MapWith = s_MapWidth;
		m_MapHeight = strlen(s_MapTiles) / s_MapWidth;

		s_TextureMap['D'] = SubTexture2D::CreateFromCoords(m_SpriteSheet, {6,11}, {128,128});
		s_TextureMap['W'] = SubTexture2D::CreateFromCoords(m_SpriteSheet, {11,11}, {128,128});

		m_CameraController.SetZoomLevel(1.0f);

		FramebufferSpecification fbSpec;
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		m_Framebuffer = Framebuffer::Create(fbSpec);

		m_ActiveScene = CreateRef<Scene>();

		//Entity
		auto square = m_ActiveScene->CreateEntity("Square");

		square.AddComponent<SpriteRendererComponent>(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		
		m_SquareEntity = square;

		m_CameraEnity = m_ActiveScene->CreateEntity("Camera Entity");
		m_CameraEnity.AddComponent<CameraComponent>();

		m_SecondCameraEnity = m_ActiveScene->CreateEntity("Camera Entity");
		auto& cc = m_SecondCameraEnity.AddComponent<CameraComponent>();
		cc.Primary = false;

		class CameraController : public ScriptableEntity
		{
		public:
			void OnCreate()
			{
				GetComponent<TransformComponent>();
			}

			void OnDestroy()
			{

			}

			void OnUpdate(Timestep ts)
			{
				auto& transform = GetComponent<TransformComponent>().Transform;
				float speed = 5.0f;
				
				if (Input::IsKeyPressed(AS_KEY_A))
					transform[3][0] -= speed * ts;
				if(Input::IsKeyPressed(AS_KEY_D))
					transform[3][0] += speed * ts;
				if(Input::IsKeyPressed(AS_KEY_W))
					transform[3][1] += speed * ts;
				if(Input::IsKeyPressed(AS_KEY_S))
					transform[3][1] -= speed * ts;
			}
		};


		m_CameraEnity.AddComponent<NativeScriptComponent>().Bind<CameraController>();

	}

	void EditorLayer::OnDetach()
	{}


	void EditorLayer::OnUpdate(Timestep ts)
	{ 
		AS_PROFILE_FUNCTION();
		if(FramebufferSpecification spec = m_Framebuffer->GetSpecification();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x,(uint32_t)m_ViewportSize.y);
			m_CameraController.OnResize(m_ViewportSize.x, m_ViewportSize.y);

			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		//Update
		if(m_ViewporFocused)
			m_CameraController.OnUpdate(ts);

		//Render
		Renderer2D::ResetStats();
		m_Framebuffer->Bind();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		RenderCommand::Clear();
		
		m_ActiveScene->OnUpdate(ts);
		
		m_Framebuffer->Unbind();

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
					if (ImGui::MenuItem("Exit")) Application::Get().Close();
						ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}
			ImGui::Begin("Setting");

			auto stats = Renderer2D::GetStats();
			ImGui::Text("Renderer2D Stats:");
			ImGui::Text("Draw Calls: %d", stats.DrawCalls);
			ImGui::Text("Quads: %d", stats.QuadCount);
			ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
			ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
			
			if (m_SquareEntity)
			{
				auto& squareColor = m_SquareEntity.GetComponent<SpriteRendererComponent>().Color;
				ImGui::ColorEdit4("Square Color", glm::value_ptr(squareColor));
			}
			ImGui::DragFloat3("Camer Transform",
				glm::value_ptr(m_CameraEnity.GetComponent<TransformComponent>().Transform[3]));
			if (ImGui::Checkbox("Camera A", &m_Primary))
			{
				m_CameraEnity.GetComponent<CameraComponent>().Primary = m_Primary;
				m_SecondCameraEnity.GetComponent<CameraComponent>().Primary = !m_Primary;
			}

			{
				auto& camera = m_SecondCameraEnity.GetComponent<CameraComponent>().Camera;
				float orthoSize = camera.GetOrthographicSzie();
				if (ImGui::DragFloat("Second Camera Ortho Size", &orthoSize))
					camera.SetOrthographicSzie(orthoSize);
			}

			ImGui::End();

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0,0 });
			ImGui::Begin("Viewport");
			m_ViewporFocused = ImGui::IsWindowFocused();
			m_ViewporHovered = ImGui::IsWindowHovered();
			Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewporFocused || !m_ViewporHovered);
			
			ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
			m_ViewportSize = { viewportPanelSize.x,viewportPanelSize.y };
			
			uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
			ImGui::Image((void*)textureID, ImVec2{ m_ViewportSize.x,m_ViewportSize.y }, ImVec2{ 0,1 }, ImVec2{ 1,0 });
			ImGui::End();
			ImGui::PopStyleVar();


			ImGui::End();
		}
		else
		{
			ImGui::Begin("Setting");

			auto stats = Renderer2D::GetStats();
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

	void EditorLayer::OnEvent(Event& event)
	{
		m_CameraController.OnEvent(event);
	}
}