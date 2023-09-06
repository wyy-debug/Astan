#include "aspch.h"
#include "Application.h"
#include "Astan/Events/ApplicationEvent.h"
#include "Astan/Log.h"
namespace Astan {

	Application::Application()
	{}

	Application::~Application()
	{}


	void Application::Run()
	{
		WindowResizeEvent e(1280, 720);
		if (e.IsInCategory(EventCategoryApplication))
		{
			AS_TRACE(e);
		}
		
		while (true);
	} 

}