#pragma once
#include "entt.hpp"
#include "Astan/Core/Timestep.h"
namespace Astan
{
	class Scene
	{
	public:
		Scene();
		~Scene();

		entt::entity CreateEntity();

		entt::registry& Reg() { return m_Registry; }
		void OnUpdate(Timestep ts);
	private:
		entt::registry m_Registry;
	};
}