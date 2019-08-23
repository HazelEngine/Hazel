#include <Hazel.h>

#include "Platform/OpenGL/OpenGLShader.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

class ExampleLayer : public Hazel::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f),
		  m_CameraPosition(0.0f), m_SquarePosition(0.0f)
	{
		// Vertex array
		{
			m_VertexArray.reset(Hazel::VertexArray::Create());

			float vertices[3 * 7] =
			{
				-0.5f, -0.5f, 0.0f,    0.8f, 0.2f, 0.8f, 1.0f,
				 0.5f, -0.5f, 0.0f,    0.2f, 0.3f, 0.8f, 1.0f,
				 0.0f,  0.5f, 0.0f,    0.8f, 0.8f, 0.2f, 1.0f
			};
			Hazel::Ref<Hazel::VertexBuffer> vertexBuffer;
			vertexBuffer.reset(Hazel::VertexBuffer::Create(vertices, _countof(vertices)));
			vertexBuffer->SetLayout({
				{ Hazel::ShaderDataType::Float3, "a_Position" },
				{ Hazel::ShaderDataType::Float4, "a_Color" }
			});
			m_VertexArray->AddVertexBuffer(vertexBuffer);

			unsigned int indices[3] = { 0, 1, 2 };
			Hazel::Ref<Hazel::IndexBuffer> indexBuffer;
			indexBuffer.reset(Hazel::IndexBuffer::Create(indices, _countof(vertices)));
			m_VertexArray->SetIndexBuffer(indexBuffer);
		}

		// Square vertex array
		{
			m_SquareVA.reset(Hazel::VertexArray::Create());

			float squareVertices[4 * 3] =
			{
				-0.5f, -0.5f, 0.0f,
				 0.5f, -0.5f, 0.0f,
				 0.5f,  0.5f, 0.0f,
				-0.5f,  0.5f, 0.0f
			};
			Hazel::Ref<Hazel::VertexBuffer> squareVB;
			squareVB.reset(Hazel::VertexBuffer::Create(squareVertices, _countof(squareVertices)));
			squareVB->SetLayout({
				{ Hazel::ShaderDataType::Float3, "a_Position" }
			});
			m_SquareVA->AddVertexBuffer(squareVB);

			uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
			Hazel::Ref<Hazel::IndexBuffer> squareIB;
			squareIB.reset(Hazel::IndexBuffer::Create(squareIndices, _countof(squareIndices)));
			m_SquareVA->SetIndexBuffer(squareIB);
		}

		// Shaders
		{
			std::string vertexSrc = R"(
				#version 330 core
			
				layout (location = 0) in vec3 a_Position;
				layout (location = 1) in vec4 a_Color;

				out vec4 v_Color;

				uniform mat4 u_ViewProjection;
				uniform mat4 u_Transform;

				void main()
				{
					v_Color = a_Color;
					gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0f);
				}
			)";

			std::string fragmentSrc = R"(
				#version 330 core

				in vec4 v_Color;

				layout (location = 0) out vec4 FragColor;

				void main()
				{
					FragColor = v_Color;
				}
			)";

			m_Shader.reset(Hazel::Shader::Create(vertexSrc, fragmentSrc));

			std::string flatColorShaderVertexSrc = R"(
				#version 330 core
			
				layout (location = 0) in vec3 a_Position;

				out vec3 v_Position;

				uniform mat4 u_ViewProjection;
				uniform mat4 u_Transform;

				void main()
				{
					v_Position = a_Position;
					gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0f);
				}
			)";

			std::string flatColorShaderFragmentSrc = R"(
				#version 330 core

				in vec3 v_Position;

				layout (location = 0) out vec4 FragColor;

				uniform vec3 u_Color;

				void main()
				{
					FragColor = vec4(u_Color, 1.0f);
				}
			)";

			m_FlatColorShader.reset(Hazel::Shader::Create(flatColorShaderVertexSrc, flatColorShaderFragmentSrc));
		}
	}

	void OnUpdate(Hazel::Timestep ts) override
	{
		if (Hazel::Input::IsKeyPressed(HZ_KEY_A)) {
			m_CameraPosition.x -= m_CameraMoveSpeed * ts;
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_D)) {
			m_CameraPosition.x += m_CameraMoveSpeed * ts;
		}

		if (Hazel::Input::IsKeyPressed(HZ_KEY_W)) {
			m_CameraPosition.y += m_CameraMoveSpeed * ts;
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_S)) {
			m_CameraPosition.y -= m_CameraMoveSpeed * ts;
		}

		if (Hazel::Input::IsKeyPressed(HZ_KEY_Q)) {
			m_CameraRotation += m_CameraRotationSpeed * ts;
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_E)) {
			m_CameraRotation -= m_CameraRotationSpeed * ts;
		}

		if (Hazel::Input::IsKeyPressed(HZ_KEY_J)) {
			m_SquarePosition.x -= m_SquareMoveSpeed * ts;
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_L)) {
			m_SquarePosition.x += m_SquareMoveSpeed * ts;
		}

		if (Hazel::Input::IsKeyPressed(HZ_KEY_I)) {
			m_SquarePosition.y += m_SquareMoveSpeed * ts;
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_K)) {
			m_SquarePosition.y -= m_SquareMoveSpeed * ts;
		}

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);

		Hazel::Renderer::BeginScene(m_Camera);

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

		glm::vec4 redColor(0.8f, 0.2f, 0.3f, 1.0f);
		glm::vec4 blueColor(0.2f, 0.3f, 0.8f, 1.0f);

		auto glFlatColorShader = std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_FlatColorShader);
		
		glFlatColorShader->Bind();
		glFlatColorShader->UploadUniformFloat3("u_Color", m_SquareColor);

		for (int y = 0; y < 20; y++)
		{
			for (int x = 0; x < 20; x++)
			{
				glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;

				Hazel::Renderer::Submit(m_FlatColorShader, m_SquareVA, transform);
			}
		}

		Hazel::Renderer::Submit(m_Shader, m_VertexArray);
		
		Hazel::Renderer::EndScene();
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
		ImGui::End();
	}

	void OnEvent(Hazel::Event& event) override
	{
	}

private:
	float m_CameraMoveSpeed = 5.0f;
	float m_CameraRotationSpeed = 180.0f;

	Hazel::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition;
	float m_CameraRotation = 0.0f;

	glm::vec3 m_SquarePosition;
	glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };
	float m_SquareMoveSpeed = 5.0f;

	Hazel::Ref<Hazel::Shader> m_Shader;
	Hazel::Ref<Hazel::VertexArray> m_VertexArray;

	Hazel::Ref<Hazel::Shader> m_FlatColorShader;
	Hazel::Ref<Hazel::VertexArray> m_SquareVA;
};

class Sandbox : public Hazel::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}

	~Sandbox()
	{
	}
};

Hazel::Application* Hazel::CreateApplication()
{
	return new Sandbox();
}