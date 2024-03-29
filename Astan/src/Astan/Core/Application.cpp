#include "aspch.h"
#include "Application.h"
#include "Astan/Core/Log.h"
#include "Input.h"


#include "Astan/Renderer/Renderer.h"
#include "Astan/Scripting/ScriptEngine.h"

#include <glfw/glfw3.h>

namespace Astan {

#define BIND_EVENT_FN(x) std::bind(&Application::x,this,std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& specification)
		: m_Specification(specification)
	{
		AS_PROFILE_FUNCTION();

		AS_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		// Set working directory here
		if (!m_Specification.WorkingDirectory.empty())
			std::filesystem::current_path(m_Specification.WorkingDirectory);

		m_Window = Window::Create(WindowProps(m_Specification.Name));
		m_Window->SetEventCallback(AS_BIND_EVENT_FN(Application::OnEvent));

		Renderer::Init();

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{
		AS_PROFILE_FUNCTION();

		ScriptEngine::Shutdown();

	}

	void Application::PushLayer(Layer* layer)
	{
		AS_PROFILE_FUNCTION();

		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		AS_PROFILE_FUNCTION();

		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	void  Application::SubmitToMainThread(const std::function<void()>& function)
	{
		std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);
		m_MainThreadQueue.emplace_back(function);
	}


	void Application::OnEvent(Event& e)
	{
		AS_PROFILE_FUNCTION();

		EventDispatcher dispacther(e);
		dispacther.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
		dispacther.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));


		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}

	void Application::Run()
	{
		AS_PROFILE_FUNCTION();
		
		while ( m_Running )
		{
			AS_PROFILE_SCOPE("RunLoop");

			float time = (float)glfwGetTime();
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			ExecuteMainThreadQueue();

			if (!m_Minimized)
			{
				{
					AS_PROFILE_SCOPE("LayerStack OnUpdate");

					for (Layer* layer : m_LayerStack)
						layer->OnUpdate(timestep);
				}
				m_ImGuiLayer->Begin();
				{
					AS_PROFILE_SCOPE("LayerStack OnImGuiRender");

					for (Layer* layer : m_LayerStack)
						layer->OnImGuiRender();
				}
				m_ImGuiLayer->End();

			}

			m_Window->OnUpdate();
			
		}
	} 

	void Application::Close()
	{
		m_Running = false;
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{ 
		m_Running = false;
		return true;
	}
	
	bool Application::OnWindowResize(WindowResizeEvent& e)
	{ 
		AS_PROFILE_FUNCTION();

		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}
		m_Minimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}

	void Application::ExecuteMainThreadQueue()
	{
		std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

		for (auto& func : m_MainThreadQueue)
			func();

		m_MainThreadQueue.clear();
	}
}