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

		void SetSelectedEntity(Entity entity);
		Entity GetSelectedEntity() const { return m_SelectionContext; }
	private:
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);
	private:
		template<typename T>
		void DisplayAddComponentEntry(const std::string& entryName);

		Ref<Scene> m_Context;
		Entity m_SelectionContext;
	};
}