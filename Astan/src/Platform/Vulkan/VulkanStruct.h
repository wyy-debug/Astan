#pragma once
#include <optional>
namespace Astan
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily = -1;
		bool isComplete()
		{
			return graphicsFamily >= 0;
		}
	};
}