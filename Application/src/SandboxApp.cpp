#include <Astan.h>

class ExampleLayer : public Astan::Layer
{
public:
	ExampleLayer()
		:Layer("Exampler")
	{}

	void OnUpdate() override
	{
		AS_INFO("ExampleLayer::Update");
	}

	void OnEvent(Astan::Event& event) override
	{
		AS_TRACE("{0}", event);
	}
};


class Sandbox : public Astan::Application
{
public:
	Sandbox() 
	{
		PushLayer(new ExampleLayer());
		PushOverlay(new Astan::ImGuiLayer());
	}

	~Sandbox() {}
};

Astan::Application* Astan::CreateApplication()
{
	return new Sandbox();
}
