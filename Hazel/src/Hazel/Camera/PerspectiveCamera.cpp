#include "hzpch.h"
#include "PerspectiveCamera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Hazel {

	PerspectiveCamera::PerspectiveCamera(float fov, float width, float height, float zNear, float zFar)
		: m_ViewMatrix(1.0f),
		  m_ViewProjectionMatrix(1.0f)
	{
		m_Position = { -5.0f, 5.0f, 5.0f };
		m_Rotation = glm::vec3(0.0f);
		m_FocalPoint = glm::vec3(0.0f);

		SetProjection(fov, width, height, zNear, zFar);
	}

	void PerspectiveCamera::SetProjection(float fov, float width, float height, float zNear, float zFar)
	{
		m_Fov = fov;
		m_ViewportWidth = width;
		m_ViewportHeight = height;
		m_Near = zNear;
		m_Far = zFar;

		m_ProjectionMatrix = glm::perspectiveFov(fov, width, height, zNear, zFar);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void PerspectiveCamera::SetViewportSize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;
		SetProjection(m_Fov, width, height, m_Near, m_Far);
	}

	glm::vec3 PerspectiveCamera::GetUpDirection()
	{
		return glm::rotate(m_Orientation, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 PerspectiveCamera::GetRightDirection()
	{
		return glm::rotate(m_Orientation, glm::vec3(1.0f, 0.0f, 0.0f));
	}

	glm::vec3 PerspectiveCamera::GetForwardDirection()
	{
		return glm::rotate(m_Orientation, glm::vec3(0.0f, 0.0f, -1.0f));
	}

	void PerspectiveCamera::RecalculateViewMatrix()
	{
		m_Rotation = glm::eulerAngles(m_Orientation) * (180.0f / (float)M_PI);

		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, m_Position) * glm::toMat4(m_Orientation);

		m_ViewMatrix = glm::inverse(transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

}