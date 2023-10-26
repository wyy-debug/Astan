#include "aspch.h"
#include "Scene.h"

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
		struct MeshComponent {};
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

		if(m_Registry.all_of<TransformComponent>(entity))
			auto& transform = m_Registry.emplace<TransformComponent>(entity,glm::mat4(1.0f));

		auto view = m_Registry.view<TransformComponent>();
		for (auto entity : view)
		{
			TransformComponent& transform = m_Registry.get<TransformComponent>(entity);
		}

		auto group = m_Registry.group<TransformComponent>(entt::get<MeshComponent>);
		for (auto entity : view)
		{
			auto&[transform, mesh] = m_Registry.get<TransformComponent, MeshComponent>(entity);
			
			Renderer::Submit(mesh, transform);
		}
	}

	Scene::~Scene()
	{
	}
}