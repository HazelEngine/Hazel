#include "Sandbox2D.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f) {}

void Sandbox2D::OnAttach()
{
	HZ_PROFILE_FUNCTION()

	m_PikaTex = Hazel::Texture2D::Create("assets/Textures/Pika.png");
	m_CheckerboardTex = Hazel::Texture2D::Create("assets/Textures/Checkerboard.png");
}

void Sandbox2D::OnDetach()
{
	HZ_PROFILE_FUNCTION()
}

float s_PikaRotation = 0.0f;
void Sandbox2D::OnUpdate(Hazel::Timestep ts)
{
	HZ_PROFILE_FUNCTION()

	// Update
	m_CameraController.OnUpdate(ts);

	// Update Pika rotation
	s_PikaRotation += ts.GetMilliseconds() * 0.001f;

	// Render
	{
		HZ_PROFILE_SCOPE("Renderer Prep")
		Hazel::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		Hazel::RenderCommand::Clear();
	}

	{
		HZ_PROFILE_SCOPE("Renderer Draw")
		Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());
		
		Hazel::Renderer2D::DrawQuad(
			{ 0.7f, 0.0f },
			{ 1.0f, 1.0f }, 
			m_SquareColor
		);

		Hazel::Renderer2D::DrawQuad(
			{ -1.0f, 0.0f}, 
			{ 0.8f, 0.8f },
			m_PikaTintColor
		);

		/*
		Hazel::Renderer2D::DrawRotatedQuad(
			{ -0.7f, 0.0f }, 
			{ 0.6f, 0.6f }, 
			s_PikaRotation, 
			m_PikaTex, 
			1.0f,
			m_PikaTintColor
		);
		
		Hazel::Renderer2D::DrawQuad(
			{ -0.7f, 0.0f, -0.1f },
			{ 10.0f, 10.0f }, 
			m_CheckerboardTex, 
			10.f
		);
		*/

		Hazel::Renderer2D::EndScene();
	}
}

void Sandbox2D::OnImGuiRender() 
{
	HZ_PROFILE_FUNCTION()

	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
	ImGui::ColorEdit4("Pika Tint Color", glm::value_ptr(m_PikaTintColor));
	ImGui::End();
}

void Sandbox2D::OnEvent(Hazel::Event& e)
{
	m_CameraController.OnEvent(e);
}
