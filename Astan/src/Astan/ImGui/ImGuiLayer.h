#pragma once

#include "Astan/Core/Layer.h"

#include "Astan/Events/ApplicationEvent.h"
#include "Astan/Events/MouseEvent.h"
#include "Astan/Events/KeyEvent.h"
namespace Astan
{
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& e) override;
	
		void Begin();
		void End();
		void BlockEvents(bool block) { m_BlockEvents = block; }
		void SetDarkThemeColor();

		uint32_t GetActiveWidgetID() const;
	private:
		bool m_BlockEvents = true;
		float m_Time = 0.0f;

	};
}