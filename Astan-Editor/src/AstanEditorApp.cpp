#include <Astan.h>
#include "Astan/Core/EntryPoint.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "EditorLayer.h"

namespace Astan{
	class AstanEditor : public Application
	{
	public:
		AstanEditor()
			: Application("Astan Editor")
		{
			PushLayer(new EditorLayer());
		}

		~AstanEditor() {}
	};

	Application* CreateApplication()
	{

		return new AstanEditor();
	}
}