#include "EditorLayer.h"
#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <chrono>

#include "Astan/Scene/SceneSerializer.h"
#include "Astan/Utils/PlatformUtils.h"
#include "Astan/Math/Math.h"

#include "ImGuizmo.h"
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

		m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);

#if 0
		//Entity
		auto square = m_ActiveScene->CreateEntity("Square");
		square.AddComponent<SpriteRendererComponent>(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

		auto redSquare = m_ActiveScene->CreateEntity("Red Square");
		redSquare.AddComponent<SpriteRendererComponent>(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		
		m_SquareEntity = square;

		m_CameraEnity = m_ActiveScene->CreateEntity("Camera A");
		m_CameraEnity.AddComponent<CameraComponent>();

		m_SecondCameraEnity = m_ActiveScene->CreateEntity("Camera B");
		auto& cc = m_SecondCameraEnity.AddComponent<CameraComponent>();
		cc.Primary = false;

		class CameraController : public ScriptableEntity
		{
		public:
			virtual void OnCreate() override
			{
				auto& translation = GetComponent<TransformComponent>().Translation;
				translation.x = rand() % 10 - 5.0f;
			}

			virtual void OnDestory() override
			{

			}

			virtual void OnUpdate(Timestep ts) override
			{
				auto& translation = GetComponent<TransformComponent>().Translation;
				float speed = 5.0f;
				
				if (Input::IsKeyPressed(AS_KEY_A))
					translation.x -= speed * ts;
				if(Input::IsKeyPressed(AS_KEY_D))
					translation.x += speed * ts;
				if(Input::IsKeyPressed(AS_KEY_W))
					translation.y += speed * ts;
				if(Input::IsKeyPressed(AS_KEY_S))
					translation.y -= speed * ts;
			}
		};


		m_CameraEnity.AddComponent<NativeScriptComponent>().Bind<CameraController>();
		m_SecondCameraEnity.AddComponent<NativeScriptComponent>().Bind<CameraController>();
#endif
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		
		
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
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		//Update
		if (!m_ViewporFocused)
			m_CameraController.OnUpdate(ts);
		m_EditorCamera.OnUpdate(ts);

		//Render
		Renderer2D::ResetStats();
		m_Framebuffer->Bind();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		RenderCommand::Clear();
		
		m_ActiveScene->OnUpdateEditor(ts,m_EditorCamera);
		
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
			ImGuiStyle& style = ImGui::GetStyle();
			float minWinSize = style.WindowMinSize.x;
			style.WindowMinSize.x = 370.0f;
			if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
			{
				ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
				ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
			}

			style.WindowMinSize.x = minWinSize;

			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("new", "Ctrl+Shife+N"))
						NewScene();
					
					if (ImGui::MenuItem("Open...","Ctrl+O"))
						OpenScene();

					if (ImGui::MenuItem("Save As...","Ctrl+Shife+S"))
						SaveScene();

					if (ImGui::MenuItem("Exit")) Application::Get().Close();
						ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			m_SceneHierarchyPanel.OnImGuiRender();
			
			ImGui::Begin("Stats");

			auto stats = Renderer2D::GetStats();
			ImGui::Text("Renderer2D Stats:");
			ImGui::Text("Draw Calls: %d", stats.DrawCalls);
			ImGui::Text("Quads: %d", stats.QuadCount);
			ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
			ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

			ImGui::End();

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0,0 });
			ImGui::Begin("Viewport");
			m_ViewporFocused = ImGui::IsWindowFocused();
			m_ViewporHovered = ImGui::IsWindowHovered();
			Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewporFocused && !m_ViewporHovered);
			
			ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
			m_ViewportSize = { viewportPanelSize.x,viewportPanelSize.y };
			
			uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
			ImGui::Image((void*)textureID, ImVec2{ m_ViewportSize.x,m_ViewportSize.y }, ImVec2{ 0,1 }, ImVec2{ 1,0 });
			
			// Gizmos
			Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
			if (selectedEntity && m_GizmoType != -1)
			{
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();
				float windowWidth = (float)ImGui::GetWindowWidth();
				float windowHeight = (float)ImGui::GetWindowHeight();
				ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);
				
				// Camera
				// Runtime Camera form entity
				//auto cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
				//const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
				//const glm::mat4& cameraProjection =  camera.GetProjection();
				//glm::mat4 cameraView = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());
				
				// Editor camera
				const glm::mat4& cameraProjection = m_EditorCamera.GetProjection();
				glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();


				//Entity
				auto& tc = selectedEntity.GetComponent<TransformComponent>();
				glm::mat4 transform = tc.GetTransform();

				//Snapping
				bool snap = Input::IsKeyPressed(AS_KEY_LEFT_CONTROL);
				float snapValue = 0.5f;
				if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
					snapValue = 45.0f;

				float snapValues[3] = { snapValue,snapValue ,snapValue };

				ImGuizmo::Manipulate(glm::value_ptr(cameraView),glm::value_ptr(cameraProjection),
					(ImGuizmo::OPERATION)m_GizmoType,ImGuizmo::LOCAL,glm::value_ptr(transform),
					nullptr,snap ? snapValues : nullptr);
				
				if (ImGuizmo::IsUsing())
				{
					glm::vec3 translation, rotation, scale;
					Math::DecomposeTransform(transform, translation, rotation, scale);
					
					glm::vec3 deltaRotation = rotation - tc.Rotation;
					tc.Translation = translation;
					tc.Rotation += deltaRotation;
					tc.Scale = scale;
				}
			}
			
			
			
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
		m_EditorCamera.OnEvent(event);
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>(AS_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{ 
		if (e.GetRepeatCount() > 0)
			return false;

		bool control = Input::IsKeyPressed(AS_KEY_RIGHT_CONTROL) || Input::IsKeyPressed(AS_KEY_LEFT_CONTROL);
		bool shift = Input::IsKeyPressed(AS_KEY_RIGHT_SHIFT) || Input::IsKeyPressed(AS_KEY_LEFT_SHIFT);

		switch (e.GetKeyCode())
		{
			case AS_KEY_N:
			{
				if (control && shift)
					NewScene();

				break;
			}
			case AS_KEY_O:
			{
				if (control)
					OpenScene();

				break;
			}
			case AS_KEY_S:
			{
				if (control && shift)
					SaveScene();

				break;
			}

			// Gizmos
			case AS_KEY_Q:
				m_GizmoType = -1;
				break;
			case AS_KEY_W:
				m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
				break;
			case AS_KEY_E:
				m_GizmoType = ImGuizmo::OPERATION::ROTATE;
				break;
			case AS_KEY_R:
				m_GizmoType = ImGuizmo::OPERATION::SCALE;
				break;
		}
	}


	void EditorLayer::NewScene()
	{
		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::OpenScene() 
	{
		std::string filepath = FileDialogs::OpenFile("Astan Scene (*.astan)\0*.astan\0");
		if (!filepath.empty())
		{
			m_ActiveScene = CreateRef<Scene>();
			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_SceneHierarchyPanel.SetContext(m_ActiveScene);

			SceneSerializer serializer(m_ActiveScene);
			serializer.Deserialize(filepath);
		}
	}

	void EditorLayer::SaveScene() 
	{
		std::string filepath = FileDialogs::SaveFile("Astan Scene (*.astan)\0*.astan\0");
		if (!filepath.empty())
		{
			SceneSerializer serializer(m_ActiveScene);
			serializer.Serialize(filepath);
		}
	}
}