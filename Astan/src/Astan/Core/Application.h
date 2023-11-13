#pragma once
#include "Core.h"

#include "Window.h"
#include "Astan/Core/LayerStack.h"
#include "Astan/Events/Event.h"
#include "Astan/Events/ApplicationEvent.h"

#include "Astan/ImGui/ImGuiLayer.h"

#include "Astan/Renderer/Renderer.h"
#include "Astan/Renderer/Shader.h"
#include "Astan/Renderer/Buffer.h"
#include "Astan/Renderer/VertexArray.h"
#include "Astan/Renderer/OrthographicCamera.h"

#include "Astan/Core/Timestep.h"



namespace Astan {
	class Application
	{
	public:
		Application(const std::string& name = "Astan App");
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		inline Window& GetWindow() { return *m_Window;}
		void Close();;

		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }
		inline static Application& Get() { return *s_Instance; }
	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
	private:
		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;
		Timestep m_Timestep;
		float m_LastFrameTime = 0.0f;

	private:
		static Application* s_Instance;
	};

	//To be defined in Client
	Application* CreateApplication();
}

