#pragma once

#include "Astan/Layer.h"

#include "Astan/Events/ApplicationEvent.h"
#include "Astan/Events/MouseEvent.h"
#include "Astan/Events/KeyEvent.h"
namespace Astan
{
	class ASTAN_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;
	
		void Begin();
		void End();
	private:
		float m_Time = 0.0f;

	};
}