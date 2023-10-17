#pragma once
#include "OrthographicCamera.h"
#include "Texture.h"
namespace Astan
{
	class Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();
		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();
		
		static void DrawQuad(const glm::vec2& positon, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec3& positon, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec2& positon, const glm::vec2& size, const Ref<Texture2D>& texture);
		static void DrawQuad(const glm::vec3& positon, const glm::vec2& size, const Ref<Texture2D>& texture);
	
	private:
	};
}