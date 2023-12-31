#include "aspch.h"
#include "Astan/Renderer/OrthographicCamera.h"

#include <glm/gtc/matrix_transform.hpp>
namespace Astan
{
	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
		: m_ProjectionMatrix(glm::ortho(left, right, bottom, top,-1.0f,1.0f)) ,m_ViewMatrix(1.0f)
	{
		AS_PROFILE_FUNCTION();

		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera::SetProjection(float left, float right, float bottom, float top)
	{
		AS_PROFILE_FUNCTION();

		m_ProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera::RecalculateViewMatrix() 
	{
		AS_PROFILE_FUNCTION();
		// 求camera变化逆矩阵,先求旋转，后求平移
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) * 
			glm::rotate(glm::mat4(1.0f),m_Rotation,glm::vec3(0,0,1));
		// 求逆矩阵
		m_ViewMatrix = glm::inverse(transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}
}