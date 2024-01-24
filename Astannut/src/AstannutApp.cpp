#include <Astan.h>
#include <Astan/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Astan{
	class Astannut : public Application
	{
	public:
		Astannut(const ApplicationSpecification& spec)
			: Application(spec)
		{
			PushLayer(new EditorLayer());
		}

		~Astannut() {}
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		ApplicationSpecification spec;
		spec.Name = "Astannut";
		spec.CommandLineArgs = args;

		return new Astannut(spec);
	}
}