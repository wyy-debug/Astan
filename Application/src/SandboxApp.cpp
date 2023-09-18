#include <Astan.h>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>



class ExampleLayer : public Astan::Layer
{
public:
	ExampleLayer()
		:Layer("Exampler")
	{
	}

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
		PushOverlay(new Astan::ImGuiLayer());
	}

	~Sandbox() {}
};

Astan::Application* Astan::CreateApplication()
{
	return new Sandbox();
}
