#pragma once
#include "Astan.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Astan/Renderer/EditorCamera.h"

namespace Astan {
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;
		virtual void OnAttach() override;
		virtual void OnDetach() override;


		void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		void OnEvent(Event& event) override;
	private:
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
		
		void NewScene();
		void OpenScene();
		void OpenScene(const std::filesystem::path& path);
		void SaveScene();
		void SaveSceneAs();

		void SerializeScene(Ref<Scene> scene, const std::filesystem::path& path);

		void OnScenePlay();
		void OnSceneStop();

		void OnDuplicateEntity();
		// UI Panels
		void UI_Toolbar();


	private:
		OrthographicCameraController m_CameraController;
	
		//Temp
		Ref<VertexArray> m_SquareVA;
		Ref<Shader> m_FlatColorShader;
		
		Ref<Texture2D> m_CheckerboardTexture;
		Ref<Texture2D> m_SpriteSheet;

		Ref<SubTexture2D> m_TextureStaris;
		Ref<SubTexture2D> m_TextureBarrel;
		Ref<SubTexture2D> m_TextureTree;
		
		Ref<Framebuffer> m_Framebuffer;

		Ref<Scene> m_ActiveScene;
		Ref<Scene> m_EditorScene,m_RuntimeScene;
		std::filesystem::path m_EditorScenPath;

		Entity m_SquareEntity;
		Entity m_CameraEnity;
		Entity m_SecondCameraEnity;
		Entity m_HoveredEntity;
		bool m_Primary = true;
	
		EditorCamera m_EditorCamera;

		struct ProfileResult
		{
			const char* Name;
			float Time;
		};
		std::vector<ProfileResult> m_ProfileResults;

		bool m_ViewporFocused = false, m_ViewporHovered = false;
		glm::vec2 m_ViewportSize = {0.0f,0.0f};
		
		glm::vec4 m_SquareColor = { 0.2f,0.3f,0.8f,1.0f };
		glm::vec2 m_ViewportBounds[2];
		uint32_t m_MapWith, m_MapHeight;
		std::unordered_map<char, Ref<SubTexture2D>> s_TextureMap;

		int m_GizmoType = -1;

		enum class SceneState
		{
			Edit = 0, Play = 1
		};
		SceneState m_SceneState = SceneState::Edit;

		//Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentBrowserPanel m_ContentBrowserPanel;

		// Editor resources
		Ref<Texture2D> m_IconPlay, m_IconStop;
		
	};
}