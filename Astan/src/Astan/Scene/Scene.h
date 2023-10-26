#pragma once
#include "entt.hpp"
namespace Astan
{
	class Scene
	{
	public:
		Scene();
		~Scene();
	private:
		entt::registry m_Registry;
	};
}