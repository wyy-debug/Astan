#include <Astan.h>
#include "Astan/Core/EntryPoint.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "EditorLayer.h"

namespace Astan{
	class Astannut : public Application
	{
	public:
		Astannut()
			: Application("Astannut")
		{
			PushLayer(new EditorLayer());
		}

		~Astannut() {}
	};

	Application* CreateApplication()
	{

		return new Astannut();
	}
}