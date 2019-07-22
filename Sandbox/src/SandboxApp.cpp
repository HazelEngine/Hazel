#include <Hazel.h>

#include <imgui.h>
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
			std::shared_ptr<Hazel::VertexBuffer> vertexBuffer;
			vertexBuffer.reset(Hazel::VertexBuffer::Create(vertices, _countof(vertices)));
			vertexBuffer->SetLayout({
				{ Hazel::ShaderDataType::Float3, "a_Position" },
				{ Hazel::ShaderDataType::Float4, "a_Color" }
			});
			m_VertexArray->AddVertexBuffer(vertexBuffer);

			unsigned int indices[3] = { 0, 1, 2 };
			std::shared_ptr<Hazel::IndexBuffer> indexBuffer;
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
			std::shared_ptr<Hazel::VertexBuffer> squareVB;
			squareVB.reset(Hazel::VertexBuffer::Create(squareVertices, _countof(squareVertices)));
			squareVB->SetLayout({
				{ Hazel::ShaderDataType::Float3, "a_Position" }
			});
			m_SquareVA->AddVertexBuffer(squareVB);

			uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
			std::shared_ptr<Hazel::IndexBuffer> squareIB;
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

			std::string squareVertexSrc = R"(
				#version 330 core
			
				layout (location = 0) in vec3 a_Position;

				out vec4 v_Color;

				uniform mat4 u_ViewProjection;
				uniform mat4 u_Transform;

				void main()
				{
					v_Color = vec4(0.25f, 0.95f, 0.71f, 1.0f);
					gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0f);
				}
			)";

			m_Shader.reset(new Hazel::Shader(vertexSrc, fragmentSrc));
			m_SquareShader.reset(new Hazel::Shader(squareVertexSrc, fragmentSrc));
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

		for (int y = 0; y < 20; y++)
		{
			for (int x = 0; x < 20; x++)
			{
				glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				Hazel::Renderer::Submit(m_SquareShader, m_SquareVA, transform);
			}
		}

		Hazel::Renderer::Submit(m_Shader, m_VertexArray);
		
		Hazel::Renderer::EndScene();
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Test");
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Hello World");
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
	float m_SquareMoveSpeed = 5.0f;

	std::shared_ptr<Hazel::Shader> m_Shader;
	std::shared_ptr<Hazel::VertexArray> m_VertexArray;

	std::shared_ptr<Hazel::Shader> m_SquareShader;
	std::shared_ptr<Hazel::VertexArray> m_SquareVA;
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