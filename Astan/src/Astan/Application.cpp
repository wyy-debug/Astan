#include "aspch.h"
#include "Application.h"
#include "Astan/Events/ApplicationEvent.h"
#include "Astan/Log.h"
#include <GLFW/glfw3.h>
namespace Astan {

	Application::Application()
	{
		m_Window = std::unique_ptr<Window>(Window::Create());
	}

	Application::~Application()
	{}


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
			m_Window->OnUpdate();
		}
	} 

}