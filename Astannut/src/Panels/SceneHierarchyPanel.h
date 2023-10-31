#pragma once

#include "Astan/Core/Core.h" 
#include "Astan/Scene/Scene.h"
#include "Astan/Core/Log.h"
#include "Astan/Scene/Entity.h"

namespace Astan
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& scene);

		void SetContext(const Ref<Scene>& scene);

		void OnImGuiRender();
	private:
		void DrawEntityNode(Entity entity);
	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;
	};
}