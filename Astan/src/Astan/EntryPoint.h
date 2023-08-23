#pragma once

#ifdef AS_PLATFORM_WINDOWS


extern Astan::Application* Astan::CreateApplication();

void main(int argc, char** argv)
{
	auto app = Astan::CreateApplication();
	app->Run();
	delete app;
}

#endif
