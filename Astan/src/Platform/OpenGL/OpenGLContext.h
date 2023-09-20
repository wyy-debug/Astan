#pragma once

#include "Astan/Renderer/GraphicsContext.h"

struct GLFWwindow;
namespace Astan
{
	
	class OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext(GLFWwindow* windowHandle);
		virtual void Init() override;
		virtual void SwapBuffers() override;
	private:
		GLFWwindow* m_WindowHandle;
	};
}
