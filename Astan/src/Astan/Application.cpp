#include "aspch.h"
#include "Application.h"
#include "Astan/Log.h"
#include <glad/glad.h>
#include "Input.h"

namespace Astan {

#define BIND_EVENT_FN(x) std::bind(&Application::x,this,std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		AS_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
	}

	Application::~Application()
	{
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispacther(e);
		dispacther.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));

		//AS_CORE_TRACE("{0}",e);

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}

	void Application::Run()
	{
		//WindowResizeEvent e(1280, 720);
		//if (e.IsInCategory(EventCategoryApplication))
		//{
		//	AS_TRACE(e);
		//}
		
		while ( m_Running )
		{
			glClearColor(1, 0, 1, 1);
			glClear(GL_COLOR_BUFFER_BIT);
			for (Layer* layer : m_LayerStack)
			{
				layer->OnUpdate();
			}
			//auto [x, y] = Input::GetMousePosition();
			//AS_CORE_TRACE("{0},{1}", x, y);
			m_Window->OnUpdate();

			
		}
	} 

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{ 
		m_Running = false;
		return true;
	}

}