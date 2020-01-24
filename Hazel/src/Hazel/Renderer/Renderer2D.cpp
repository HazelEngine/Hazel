#include "hzpch.h"
#include "Renderer2D.h"

#include "Hazel/Renderer/VertexArray.h"
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Hazel {

	struct Renderer2DStorage
	{
		Ref<VertexArray> QuadVertexArray;
		Ref<Shader> FlatColorShader;
	};

	static Renderer2DStorage* s_Data;

	void Renderer2D::Init()
	{
		s_Data = new Renderer2DStorage();

		// QuadVertexArray
		s_Data->QuadVertexArray = VertexArray::Create();

		float vertices[4 * 3] =
		{
			// Position		   
			-0.5f, -0.5f, 0.0f,
			 0.5f, -0.5f, 0.0f,
			 0.5f,  0.5f, 0.0f,
			-0.5f,  0.5f, 0.0f
		};
		Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(vertices, _countof(vertices));
		vertexBuffer->SetLayout({
			{ Hazel::ShaderDataType::Float3, "a_Position" }
		});
		s_Data->QuadVertexArray->AddVertexBuffer(vertexBuffer);

		uint32_t indices[6] = { 0, 1, 2, 2, 3, 0 };
		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(indices, _countof(indices));
		s_Data->QuadVertexArray->SetIndexBuffer(indexBuffer);

		// FlatColorShader
		s_Data->FlatColorShader = Shader::CreateFromSpirv(
			"FlatColorShader",
			"assets/Shaders/Compiled/FlatColor.vs",
			"assets/Shaders/Compiled/FlatColor.fs"
		);
	}

	void Renderer2D::Shutdown()
	{
		delete s_Data;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		s_Data->FlatColorShader->Bind();
		s_Data->FlatColorShader->SetMat4("u_SceneData.ViewProjection", camera.GetViewProjectionMatrix());
	}

	void Renderer2D::EndScene() {}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, position);
		transform = glm::scale(transform, glm::vec3(size, 1.0f));

		s_Data->FlatColorShader->Bind();
		s_Data->FlatColorShader->SetMat4("u_SceneData.Transform", transform);
		s_Data->FlatColorShader->SetFloat4("u_RenderData.Color", color);

		s_Data->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
	}

}