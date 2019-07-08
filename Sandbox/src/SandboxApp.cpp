#include <Hazel.h>

#include "imgui.h"

class ExampleLayer : public Hazel::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f)
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
				-0.75f, -0.75f, 0.0f,
				 0.75f, -0.75f, 0.0f,
				 0.75f,  0.75f, 0.0f,
				-0.75f,  0.75f, 0.0f
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

				void main()
				{
					v_Color = a_Color;
					gl_Position = u_ViewProjection * vec4(a_Position, 1.0f);
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

				void main()
				{
					v_Color = vec4(0.25f, 0.95f, 0.71f, 1.0f);
					gl_Position = u_ViewProjection * vec4(a_Position, 1.0f);
				}
			)";

			m_Shader.reset(new Hazel::Shader(vertexSrc, fragmentSrc));
			m_SquareShader.reset(new Hazel::Shader(squareVertexSrc, fragmentSrc));
		}
	}

	void OnUpdate() override
	{
		m_Camera.SetPosition({ 0.5f, 0.5f, 0.0f });
		m_Camera.SetRotation(45.0f);

		Hazel::Renderer::BeginScene(m_Camera);
		Hazel::Renderer::Submit(m_SquareShader, m_SquareVA);
		Hazel::Renderer::Submit(m_Shader, m_VertexArray);
		Hazel::Renderer::EndScene();

		if (Hazel::Input::IsKeyPressed(HZ_KEY_TAB))
		{
			HZ_TRACE("Tab key is pressed!")
		}
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Test");
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Hello World");
		ImGui::End();
	}

	void OnEvent(Hazel::Event& event) override
	{
		//HZ_TRACE("{0}", event)
	}

private:
	Hazel::OrthographicCamera m_Camera;

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