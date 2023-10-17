#include "aspch.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <Glad/glad.h>

namespace Astan
{
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		AS_CORE_ASSERT(windowHandle,"Window handle is null!")
	 }

	void OpenGLContext::Init()
	{
		AS_PROFILE_FUNCTION();

		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		AS_CORE_ASSERT(status, "Failed to initialize Glad;");

		//AS_CORE_INFO("OpenGL Renderer: {0}",glGetString(GL_RENDERER));

	}
	void OpenGLContext::SwapBuffers()
	{
		AS_PROFILE_FUNCTION();

		glfwSwapBuffers(m_WindowHandle);
	}
}