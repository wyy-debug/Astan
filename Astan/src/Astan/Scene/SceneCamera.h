#pragma once

#include "Astan/Renderer/Camera.h"

namespace Astan
{
	class SceneCamera : public Camera
	{
	public:
		enum class ProjectionType { Perspective = 0, Orthographic = 1};
	public:
		SceneCamera();
		virtual ~SceneCamera() = default;

		void SetOrthographic(float size, float nearClip, float farClip);
		void SetPerspective(float verticalFov, float nearClip, float farClip);

		void SetViewportSize(uint32_t width, uint32_t height);

		float GetPerspectiveVerticalFOV() const { return m_PerspectiveFOV; }
		void SetPerspectiveVerticalFOV(float verticalFov) { m_PerspectiveFOV = verticalFov; RecalculateProjection(); }
		float GetPerspectiveVerticalNearClip() const { return m_PerspectiveNear; }
		void SetPerspectiveVerticalNearClip(float nearClip) { m_PerspectiveNear = nearClip; RecalculateProjection(); }
		float GetPerspectiveVerticalFarClip() const { return m_PerspectiveFar; }
		void SetPerspectiveVerticalFarClip(float farClip) { m_PerspectiveFar = farClip; RecalculateProjection(); }


		float GetOrthographicSzie() const { return m_OrthographicSize; }
		void SetOrthographicSzie(float size) { m_OrthographicSize = size; RecalculateProjection(); }
		float GetOrthographicNearClip() const { return m_OrthographicNear; }
		void SetOrthographicNearClip(float nearClip) { m_OrthographicNear = nearClip; RecalculateProjection(); }
		float GetOrthographicFarClip() const { return m_OrthographicFar; }
		void SetOrthographicFarClip(float farClip) { m_OrthographicFar = farClip; RecalculateProjection(); }


		ProjectionType GetProjectionType() const { return m_ProjectionType; }
		void SetProjectionType(ProjectionType type) { m_ProjectionType = type; RecalculateProjection();} 
	private:
		void RecalculateProjection();
	private:
		ProjectionType m_ProjectionType = ProjectionType::Orthographic;

		float m_OrthographicSize = 10.0f;
		float m_OrthographicNear = -1.0f;
		float m_OrthographicFar = 1.0f;


		float m_PerspectiveFOV = glm::radians(45.0f);
		float m_PerspectiveNear = 0.01f;
		float m_PerspectiveFar = 1000.0f;

		float m_AspectRatio = 0.0f;
	};
}