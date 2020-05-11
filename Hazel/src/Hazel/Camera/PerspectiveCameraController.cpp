#include "hzpch.h"
#include "PerspectiveCameraController.h"

#include "Hazel/Core/Input.h"
#include "Hazel/Core/KeyCodes.h"
#include "Hazel/Core/MouseButtonCodes.h"

#include <glm/gtc/quaternion.hpp>

namespace Hazel {

	PerspectiveCameraController::PerspectiveCameraController(
		float fov,
		float width,
		float height,
		float zNear,
		float zFar
	) : m_Camera(fov, width, height, zNear, zFar)
	{
		m_Distance = glm::distance(m_Camera.GetPosition(), m_Camera.GetFocalPoint());

		m_Yaw = 3.0f * (float)M_PI / 4.0f;
		m_Pitch = M_PI / 4.0f;
	}

	void PerspectiveCameraController::OnUpdate(Timestep ts)
	{
		if (Input::IsKeyPressed(HZ_KEY_LEFT_ALT))
		{
			const glm::vec2& mouse { Input::GetMouseX(), Input::GetMouseY() };
			glm::vec2 delta = mouse - m_InitialMousePosition;
			m_InitialMousePosition = mouse;

			delta *= ts.GetSeconds();

			if (Input::IsMouseButtonPressed(HZ_MOUSE_BUTTON_MIDDLE))
				MousePan(delta);
			else if (Input::IsMouseButtonPressed(HZ_MOUSE_BUTTON_LEFT))
				MouseRotate(delta);
			else if (Input::IsMouseButtonPressed(HZ_MOUSE_BUTTON_RIGHT))
				MouseZoom(delta.y);
		}

		// Calculate and set orientation quaternion
		auto orientation = glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
		m_Camera.SetOrientation(orientation);

		// Calculate and set position
		auto position = m_Camera.GetFocalPoint() - m_Camera.GetForwardDirection() * m_Distance;
		m_Camera.SetPosition(position);
	}

	void PerspectiveCameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(HZ_BIND_EVENT_FN(PerspectiveCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(HZ_BIND_EVENT_FN(PerspectiveCameraController::OnWindowResized));
	}

	bool PerspectiveCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		float delta = e.GetYOffset() * 0.1f;
		MouseZoom(delta);
		return false;
	}

	bool PerspectiveCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		m_Camera.SetViewportSize(e.GetWidth(), e.GetHeight());
		return false;
	}

	void PerspectiveCameraController::MousePan(const glm::vec2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		auto focalPoint = m_Camera.GetFocalPoint();
		focalPoint += -m_Camera.GetRightDirection() * delta.x * xSpeed * m_Distance;
		focalPoint += m_Camera.GetUpDirection() * delta.y * ySpeed * m_Distance;
		m_Camera.SetFocalPoint(focalPoint);
	}

	void PerspectiveCameraController::MouseRotate(const glm::vec2& delta)
	{
		float yawSign = m_Camera.GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * RotationSpeed();
		m_Pitch += delta.y * RotationSpeed();
	}

	void PerspectiveCameraController::MouseZoom(float delta)
	{
		m_Distance -= delta * ZoomSpeed();
		if (m_Distance < 1.0f)
		{
			auto focalPoint = m_Camera.GetFocalPoint() + m_Camera.GetForwardDirection();
			m_Camera.SetFocalPoint(focalPoint);
			m_Distance = 1.0f;
		}
	}

	std::pair<float, float> PerspectiveCameraController::PanSpeed() const
	{
		float x = std::min(m_Camera.GetViewportWidth() / 1000.0f, 2.4f); // max = 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(m_Camera.GetViewportHeight() / 1000.0f, 2.4f); // max = 2.4f
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float PerspectiveCameraController::RotationSpeed() const
	{
		return 0.8f;
	}

	float PerspectiveCameraController::ZoomSpeed() const
	{
		float distance = m_Distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, 100.0f); // max speed = 100
		return speed;
	}

}