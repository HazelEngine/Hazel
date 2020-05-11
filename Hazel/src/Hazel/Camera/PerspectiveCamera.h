#pragma once

#include <Hazel/Camera/Camera.h>

#include <glm/gtc/quaternion.hpp>

namespace Hazel {

	class PerspectiveCamera : public Camera
	{
	public:
		PerspectiveCamera() = default;
		PerspectiveCamera(float fov, float width, float height, float zNear, float zFar);

		void SetProjection(float fov, float width, float height, float zNear, float zFar);
		void SetViewportSize(uint32_t width, uint32_t height);

		const glm::vec3& GetPosition() const override { return m_Position; }
		void SetPosition(const glm::vec3& position) override
		{
			m_Position = position;
			RecalculateViewMatrix();
		}

		const glm::vec3& GetRotation() const { return m_Rotation; }

		const glm::vec3& GetFocalPoint() const { return m_FocalPoint; }
		void SetFocalPoint(const glm::vec3& focalPoint)
		{
			m_FocalPoint = focalPoint;
		}

		const glm::quat& GetOrientation() const { return m_Orientation; }
		void SetOrientation(const glm::quat& orientation)
		{
			m_Orientation = orientation;
			RecalculateViewMatrix();
		}

		glm::vec3 GetUpDirection();
		glm::vec3 GetRightDirection();
		glm::vec3 GetForwardDirection();

		const glm::mat4& GetProjectionMatrix() const override { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const override { return m_ViewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() const override { return m_ViewProjectionMatrix; }

		uint32_t GetViewportWidth() const { return m_ViewportWidth; }
		uint32_t GetViewportHeight() const { return m_ViewportHeight; }

	private:
		void RecalculateViewMatrix();

	private:
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;

		glm::vec3 m_Position;
		glm::vec3 m_Rotation;
		glm::vec3 m_FocalPoint;

		glm::quat m_Orientation;

		float m_Fov, m_Near, m_Far;
		uint32_t m_ViewportWidth, m_ViewportHeight;
	};

}