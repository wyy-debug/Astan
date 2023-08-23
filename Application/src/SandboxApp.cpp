#include <Astan.h>

class Sandbox : public Astan::Application
{
public:
	Sandbox() {}

	~Sandbox() {}
};

Astan::Application* Astan::CreateApplication()
{
	return new Sandbox();
}
