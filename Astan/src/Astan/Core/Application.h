#pragma once
#include "Core.h"

#include "Window.h"
#include "Astan/Core/LayerStack.h"
#include "Astan/Events/Event.h"
#include "Astan/Events/ApplicationEvent.h"

#include "Astan/ImGui/ImGuiLayer.h"

#include "Astan/Renderer/Renderer.h"
#include "Astan/Renderer/Shader.h"
#include "Astan/Renderer/Buffer.h"
#include "Astan/Renderer/VertexArray.h"
#include "Astan/Renderer/OrthographicCamera.h"

#include "Astan/Core/Timestep.h"



namespace Astan {

	struct ApplicationCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			AS_CORE_ASSERT(index < Count);
			return Args[index];
		}
	};

	struct ApplicationSpecification
	{
		std::string Name = "Astan Application";
		std::string WorkingDirectory;
		ApplicationCommandLineArgs CommandLineArgs;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& specification);
		virtual ~Application();

		void Run();
		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		Window& GetWindow() { return *m_Window;}
		void Close();;

		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }
		static Application& Get() { return *s_Instance; }

		const ApplicationSpecification& GetSpecification() const { return m_Specification; }

		void SubmitToMainThread(const std::function<void()> &function);
	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
		
		void ExecuteMainThreadQueue();
	private:
		ApplicationSpecification m_Specification;
		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;
		Timestep m_Timestep;
		float m_LastFrameTime = 0.0f;

		std::vector<std::function<void()>> m_MainThreadQueue;
		std::mutex m_MainThreadQueueMutex;
	private:
		static Application* s_Instance;
	};

	//To be defined in Client
	Application* CreateApplication(ApplicationCommandLineArgs args);
}

