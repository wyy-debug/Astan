#include "aspch.h"
#include "Entity.h"

namespace Astan
{
	Entity::Entity(entt::entity handle, Scene* scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{

	}
}