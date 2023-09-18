#include <Astan.h>

//#include "imgui/imgui.h"


class ExampleLayer : public Astan::Layer
{
public:
	ExampleLayer()
		:Layer("Exampler")
	{
	}

	//virtual void OnImGuiRender() override
	//{
	//	ImGui::Begin("Test");
	//	ImGui::Text("hello world");
	//	ImGui::End();
	//}
	void OnUpdate() override
	{
		//AS_INFO("ExampleLayer::Update");
		if (Astan::Input::IsKeyPressed(AS_KEY_TAB)) {
			AS_TRACE("Tab key is pressed(poll)");
		}
	}

	void OnEvent(Astan::Event& event) override
	{
		if (event.GetEventType() == Astan::EventType::KeyPressed)
		{
			Astan::KeyPressedEvent& e = (Astan::KeyPressedEvent&)event;
			if (e.GetKeyCode() == AS_KEY_TAB) {
				AS_TRACE("Tab key is pressed(event)");
			}
			AS_TRACE("{0}", (char)e.GetKeyCode());
		}
		/*AS_TRACE("{0}", event);*/
	}
};


class Sandbox : public Astan::Application
{
public:
	Sandbox() 
	{
		PushLayer(new ExampleLayer());
	}

	~Sandbox() {}
};

Astan::Application* Astan::CreateApplication()
{
	return new Sandbox();
}
