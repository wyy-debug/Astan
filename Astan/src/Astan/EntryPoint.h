#pragma once

#ifdef AS_PLATFORM_WINDOWS


extern Astan::Application* Astan::CreateApplication();

void main(int argc, char** argv)
{
	Astan::Log::Init();
	AS_CORE_WARN("Initialized Log");
	int a = 5;
	AS_INFO("Hello! Var={0}", a);
	auto app = Astan::CreateApplication();
	app->Run();
	delete app;
}

#endif
