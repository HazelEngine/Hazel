#pragma once

#include "Hazel/Core/Timestep.h"
#include "Hazel/Camera/PerspectiveCamera.h"
#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/Events/MouseEvent.h"

namespace Hazel {

	class PerspectiveCameraController
	{
	public:
		HAZEL_API PerspectiveCameraController(float fov, float width, float height, float zNear, float zFar);

		HAZEL_API void OnUpdate(Timestep ts);
		HAZEL_API void OnEvent(Event& e);

		PerspectiveCamera& GetCamera() { return m_Camera; }
		const PerspectiveCamera& GetCamera() const { return m_Camera; }

	private:
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);

		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(float delta);

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;

	private:
		PerspectiveCamera m_Camera;

		float m_Distance;
		float m_Pitch, m_Yaw;

		glm::vec2 m_InitialMousePosition;
	};

}