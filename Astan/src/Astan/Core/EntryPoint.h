#pragma once

#ifdef AS_PLATFORM_WINDOWS


extern Astan::Application* Astan::CreateApplication();

void main(int argc, char** argv)
{
	Astan::Log::Init();
	AS_PROFILE_BEGIN_SESSION("Startup", "AstanProfile-Startup.json");
	auto app = Astan::CreateApplication();
	AS_PROFILE_END_SESSION();
	AS_PROFILE_BEGIN_SESSION("Runtime", "AstanProfile-Runtime.json");
	app->Run();
	AS_PROFILE_END_SESSION();
	AS_PROFILE_BEGIN_SESSION("Shutdown", "AstanProfile-Shutdown.json");
	delete app;
	AS_PROFILE_END_SESSION();
}

#endif
