#pragma once
#include "Core.h"

#include "Window.h"
#include "Astan/LayerStack.h"
#include "Astan/Events/Event.h"
#include "Astan/Events/ApplicationEvent.h"

#include "Astan/ImGui/ImGuiLayer.h"
#include "Astan/Renderer/Shader.h"
#include "Astan/Renderer/Buffer.h"
#include "Astan/Renderer/VertexArray.h"

namespace Astan {
	class ASTAN_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		inline Window& GetWindow() { return *m_Window;}
		inline static Application& Get() { return *s_Instance; }
	private:
		bool OnWindowClose(WindowCloseEvent& e);

		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		LayerStack m_LayerStack;

		std::shared_ptr<Shader> m_Shader;
		std::shared_ptr<VertexArray> m_VertexArray;


		std::shared_ptr<VertexArray> m_SquareVA;
		std::shared_ptr<Shader> m_BlueShader;


	private:
		static Application* s_Instance;
	};

	//To be defined in Client
	Application* CreateApplication();
}

