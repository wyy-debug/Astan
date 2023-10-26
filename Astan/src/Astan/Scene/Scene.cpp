#include "aspch.h"
#include "Scene.h"
#include "Component.h"
#include "Astan/Renderer/Renderer2D.h"
#include <glm/glm.hpp>
namespace Astan
{
	static void DoMath(const glm::mat4& transform)
	{

	}

	static void OnTransformConstruct(entt::registry& registry, entt::entity entity)
	{
	}

	Scene::Scene()
	{
#if ENTT_EXAMPLE_CODE
		struct MeshComponent
		{
			bool Data;
			MeshComponent() = default;
		};
		struct TransformComponent
		{
			glm::mat4 Transform;

			TransformComponent() = default;
			TransformComponent(const TransformComponent&) = default;
			TransformComponent(const glm::mat4& transform)
				: Transform(transform) {}

			operator glm::mat4& () { return Transform; }
			operator const glm::mat4& () const { return Transform; }
		};

		entt::entity entity = m_Registry.create();//组件只是一个ID，hash表
		m_Registry.emplace<TransformComponent>(entity, glm::mat4(1.0f));

		m_Registry.on_construct<TransformComponent>().connect<&OnTransformConstruct>();

		if (m_Registry.all_of<TransformComponent>(entity))
			auto& transform = m_Registry.emplace<TransformComponent>(entity, glm::mat4(1.0f));

		auto view = m_Registry.view<TransformComponent>();
		for (auto entity : view)
		{
			TransformComponent& transform = m_Registry.get<TransformComponent>(entity);
		}

		auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (auto entity : group)
		{
			auto& [transform, sprite] = m_Registry.get<TransformComponent, SpriteRendererComponent>(entity);
		}

#endif
		
	}

	Scene::~Scene()
	{
	}

	entt::entity Scene::CreateEntity()
	{
		return m_Registry.create();
	}

	void Scene::OnUpdate(Timestep ts)
	{
		auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (auto entity : group)
		{
			auto& [transform, sprite] = m_Registry.get<TransformComponent, SpriteRendererComponent>(entity);
			Renderer2D::DrawQuad(transform,sprite.Color);
		}
	}

}