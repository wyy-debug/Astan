#pragma once
#include "RenderPass.h"

namespace Astan
{
	class MainCameraPass : public RenderPass
	{
	public:
		void Initialize() override;
		void Draw() override;
	};
}