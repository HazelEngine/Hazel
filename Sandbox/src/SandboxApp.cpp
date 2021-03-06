#include <Hazel.h>
#include <Hazel/Core/EntryPoint.h>

#include "Platform/OpenGL/OpenGLShader.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Sandbox2D.h"

class ExampleLayer : public Hazel::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_CameraController(1280.0f / 720.0f)
	{
		// Vertex array
		{
			m_VertexArray = Hazel::VertexArray::Create();

			float vertices[3 * 7] =
			{
				-0.5f, -0.5f, 0.0f,    0.8f, 0.2f, 0.8f, 1.0f,
				 0.5f, -0.5f, 0.0f,    0.2f, 0.3f, 0.8f, 1.0f,
				 0.0f,  0.5f, 0.0f,    0.8f, 0.8f, 0.2f, 1.0f
			};
			Hazel::Ref<Hazel::VertexBuffer> vertexBuffer;
			vertexBuffer = Hazel::VertexBuffer::Create(vertices, _countof(vertices));
			vertexBuffer->SetLayout({
				{ Hazel::ShaderDataType::Float3, "a_Position" },
				{ Hazel::ShaderDataType::Float4, "a_Color" }
			});
			m_VertexArray->AddVertexBuffer(vertexBuffer);

			unsigned int indices[3] = { 0, 1, 2 };
			Hazel::Ref<Hazel::IndexBuffer> indexBuffer;
			indexBuffer = Hazel::IndexBuffer::Create(indices, _countof(vertices));
			m_VertexArray->SetIndexBuffer(indexBuffer);
		}

		// Square vertex array
		{
			m_SquareVA = Hazel::VertexArray::Create();

			float squareVertices[4 * 5] =
			{
				// Position		       // TexCoord
				-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,
				 0.5f, -0.5f, 0.0f,		1.0f, 0.0f,
				 0.5f,  0.5f, 0.0f,		1.0f, 1.0f,
				-0.5f,  0.5f, 0.0f,		0.0f, 1.0f
			};
			Hazel::Ref<Hazel::VertexBuffer> squareVB;
			squareVB = Hazel::VertexBuffer::Create(squareVertices, _countof(squareVertices));
			squareVB->SetLayout({
				{ Hazel::ShaderDataType::Float3, "a_Position" },
				{ Hazel::ShaderDataType::Float2, "a_TexCoord" }
			});
			m_SquareVA->AddVertexBuffer(squareVB);

			uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
			Hazel::Ref<Hazel::IndexBuffer> squareIB;
			squareIB = Hazel::IndexBuffer::Create(squareIndices, _countof(squareIndices));
			m_SquareVA->SetIndexBuffer(squareIB);
		}

		// Shaders
		{
			m_Shader = Hazel::Shader::CreateFromSpirv(
				"Shader",
				"assets/Shaders/Compiled/Shader.vs",
				"assets/Shaders/Compiled/Shader.fs"
			);

			m_FlatColorShader = Hazel::Shader::CreateFromSpirv(
				"FlatColorShader3D",
				"assets/Shaders/Compiled/FlatColor.vs",
				"assets/Shaders/Compiled/FlatColor.fs"
			);

			m_TextureShader = Hazel::Shader::CreateFromSpirv(
				"TextureShader",
				"assets/Shaders/Compiled/Texture.vs",
				"assets/Shaders/Compiled/Texture.fs"
			);

			auto texShader = std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_TextureShader);
			texShader->Bind();
			texShader->UploadUniformInt("u_Texture", 0);
		}

		// Textures
		{
			m_Texture = Hazel::Texture2D::Create("assets/textures/Checkerboard.png");
			m_PikaTexture = Hazel::Texture2D::Create("assets/textures/Pika.png");
		}
	}

	void OnUpdate(Hazel::Timestep ts) override
	{
		// Update
		m_CameraController.OnUpdate(ts);

		// Render
		Hazel::Renderer::BeginScene(m_CameraController.GetCamera());

		auto glFlatColorShader = std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_FlatColorShader);
		glFlatColorShader->Bind();
		glFlatColorShader->UploadUniformFloat3("u_RenderData.Color", m_SquareColor);

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
		for (int y = 0; y < 20; y++)
		{
			for (int x = 0; x < 20; x++)
			{
				glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;

				Hazel::Renderer::Submit(m_FlatColorShader, m_SquareVA, transform);
			}
		}

		scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.5f));
		
		auto glTexShader = std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_TextureShader);
		glTexShader->Bind();
		glTexShader->UploadUniformFloat3("u_RenderData.Color", m_TextureColor);

		m_Texture->Bind(0);
		Hazel::Renderer::Submit(m_TextureShader, m_SquareVA, scale);

		m_PikaTexture->Bind(0);
		Hazel::Renderer::Submit(m_TextureShader, m_SquareVA, scale);
		
		Hazel::Renderer::EndScene();
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
		ImGui::ColorEdit3("Texture Color", glm::value_ptr(m_TextureColor));
		ImGui::End();
	}

	void OnEvent(Hazel::Event& e) override
	{
		m_CameraController.OnEvent(e);
	}

private:
	Hazel::OrthographicCameraController m_CameraController;

	Hazel::Ref<Hazel::Shader> m_Shader;
	Hazel::Ref<Hazel::VertexArray> m_VertexArray;

	Hazel::Ref<Hazel::Shader> m_FlatColorShader, m_TextureShader;
	Hazel::Ref<Hazel::VertexArray> m_SquareVA;

	Hazel::Ref<Hazel::Texture2D> m_Texture, m_PikaTexture;
	glm::vec3 m_TextureColor = { 1.0f, 1.0f, 1.0f };

	glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };
};

class Sandbox : public Hazel::Application
{
public:
	Sandbox()
	{
		//PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2D());
	}

	~Sandbox() {}
};

Hazel::Application* Hazel::CreateApplication()
{
	return new Sandbox();
}