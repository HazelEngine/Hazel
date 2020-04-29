#include "hzpch.h"
#include "Renderer2D.h"

#include "Hazel/Renderer/Renderer.h"
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/Pipeline.h"
#include "Hazel/Renderer/RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Hazel {

	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoord;
		glm::vec4 Color;
	};
	
	struct Renderer2DData
	{
		const uint32_t MaxQuads = 40000;
		const uint32_t MaxVertices = MaxQuads * 4;
		const uint32_t MaxIndices = MaxQuads * 6;
		
		Ref<Pipeline> QuadPipeline;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<IndexBuffer> QuadIndexBuffer;

		uint32_t IndexCount = 0;
		QuadVertex* QuadVertexBufferData = nullptr;
		QuadVertex* QuadVertexBufferDataPtr = nullptr;
	};

	static Renderer2DData* s_Data = new Renderer2DData();

	void Renderer2D::Init()
	{
		HZ_PROFILE_FUNCTION()
		
		s_Data->QuadVertexBuffer = VertexBuffer::Create(s_Data->MaxVertices * sizeof(QuadVertex));
		s_Data->QuadVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoords" },
			{ ShaderDataType::Float4, "a_Color" }
		});
		
		uint32_t* indices = new uint32_t[s_Data->MaxIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_Data->MaxIndices; i += 6)
		{
			indices[i + 0] = offset + 0;
			indices[i + 1] = offset + 1;
			indices[i + 2] = offset + 2;
			
			indices[i + 3] = offset + 2;
			indices[i + 4] = offset + 3;
			indices[i + 5] = offset + 0;

			offset += 4;
		}
		
		s_Data->QuadIndexBuffer = IndexBuffer::Create(indices, s_Data->MaxIndices);
		delete[] indices;
		
		// FlatColor shader
		const auto shader = Shader::Create(
			"FlatColor",
			"assets/Shaders/Compiled/FlatColor.vert.spv",
			"assets/Shaders/Compiled/FlatColor.frag.spv"
		);

		PipelineSpecification spec;
		spec.Shader = shader;
		spec.VertexBufferLayout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoords" },
			{ ShaderDataType::Float4, "a_Color" }
		};
		s_Data->QuadPipeline = Pipeline::Create(spec);
	}

	void Renderer2D::Shutdown()
	{
		HZ_PROFILE_FUNCTION()
		delete s_Data;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		HZ_PROFILE_FUNCTION()
		
		s_Data->IndexCount = 0;
		s_Data->QuadVertexBufferData = (QuadVertex*)s_Data->QuadVertexBuffer->Map();
		s_Data->QuadVertexBufferDataPtr = s_Data->QuadVertexBufferData;

		struct
		{
			glm::mat4 ViewProj;
			glm::mat4 Transform;
		} cameraUB;

		// Update matrices
		cameraUB.ViewProj = camera.GetViewProjectionMatrix();
		cameraUB.Transform = glm::mat4(1.0f);
		s_Data->QuadPipeline->GetSpecification()
			.Shader->SetUniformBuffer("u_SceneData", &cameraUB, sizeof(cameraUB));
	}

	void Renderer2D::EndScene()
	{
		HZ_PROFILE_FUNCTION()

		std::ptrdiff_t size = (char*)s_Data->QuadVertexBufferDataPtr - (char*)s_Data->QuadVertexBufferData;
		s_Data->QuadVertexBuffer->Unmap(size);

		Renderer::Submit(
			s_Data->QuadPipeline,
			s_Data->QuadVertexBuffer,
			s_Data->QuadIndexBuffer,
			s_Data->IndexCount
		);
	}

	void Renderer2D::OnResize()
	{
		// Framebuffer size has changed, need to re-build command buffer
		// (Note: This is MANDATORY on Vulkan and D3D12)
		Renderer::Submit(
			s_Data->QuadPipeline,
			s_Data->QuadVertexBuffer,
			s_Data->QuadIndexBuffer,
			s_Data->IndexCount
		);
		Renderer::FlushCommandBuffer();
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		HZ_PROFILE_FUNCTION()
		
		s_Data->QuadVertexBufferDataPtr->Position = position;
		s_Data->QuadVertexBufferDataPtr->TexCoord = { 0.0f, 0.0f };
		s_Data->QuadVertexBufferDataPtr->Color = color;
		s_Data->QuadVertexBufferDataPtr++;

		s_Data->QuadVertexBufferDataPtr->Position = { position.x + size.x, position.y, position.z };
		s_Data->QuadVertexBufferDataPtr->TexCoord = { 1.0f, 0.0f };
		s_Data->QuadVertexBufferDataPtr->Color = color;
		s_Data->QuadVertexBufferDataPtr++;

		s_Data->QuadVertexBufferDataPtr->Position = { position.x + size.x, position.y + size.y, position.z };
		s_Data->QuadVertexBufferDataPtr->TexCoord = { 1.0f, 1.0f };
		s_Data->QuadVertexBufferDataPtr->Color = color;
		s_Data->QuadVertexBufferDataPtr++;

		s_Data->QuadVertexBufferDataPtr->Position = { position.x, position.y + size.y, position.z };
		s_Data->QuadVertexBufferDataPtr->TexCoord = { 0.0f, 1.0f };
		s_Data->QuadVertexBufferDataPtr->Color = color;
		s_Data->QuadVertexBufferDataPtr++;

		s_Data->IndexCount += 6;
	}

	void Renderer2D::DrawQuad(
		const glm::vec2& position,
		const glm::vec2& size,
		const Ref<Texture2D>& texture,
		float tilingFactor,
		const glm::vec4& tintColor
	)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawQuad(
		const glm::vec3& position,
		const glm::vec2& size,
		const Ref<Texture2D>& texture,
		float tilingFactor,
		const glm::vec4& tintColor
	)
	{
		HZ_PROFILE_FUNCTION()
	}

	void Renderer2D::DrawRotatedQuad(
		const glm::vec2& position,
		const glm::vec2& size,
		float rotation,
		const glm::vec4& color
	)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, color);
	}

	void Renderer2D::DrawRotatedQuad(
		const glm::vec3& position,
		const glm::vec2& size,
		float rotation,
		const glm::vec4& color
	)
	{
		HZ_PROFILE_FUNCTION()
	}

	void Renderer2D::DrawRotatedQuad(
		const glm::vec2& position,
		const glm::vec2& size,
		float rotation,
		const Ref<Texture2D>& texture,
		float tilingFactor,
		const glm::vec4& tintColor
	)
	{
		DrawRotatedQuad(
			{ position.x, position.y, 0.0f },
			size, 
			rotation,
			texture,
			tilingFactor,
			tintColor
		);
	}

	void Renderer2D::DrawRotatedQuad(
		const glm::vec3& position,
		const glm::vec2& size,
		float rotation,
		const Ref<Texture2D>& texture,
		float tilingFactor,
		const glm::vec4& tintColor
	)
	{
		HZ_PROFILE_FUNCTION()
	}
}
