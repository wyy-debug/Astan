#pragma once
#include "Core.h"

#include "Window.h"
#include "Astan/LayerStack.h"
#include "Astan/Events/Event.h"
#include "Astan/Events/ApplicationEvent.h"


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
	private:
		bool OnWindowClose(WindowCloseEvent& e);
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
		LayerStack m_LayerStack;
	};

	//To be defined in Client
	Application* CreateApplication();
}

