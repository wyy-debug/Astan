#pragma once
#include "Core.h"
namespace Astan {
	class ASTAN_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};
	//To be defined in Client
	Application* CreateApplication();
}

