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
		float     TexIndex;
		float     TilingFactor;
	};
	
	struct Renderer2DData
	{
		static const uint32_t QuadVertexCount = 4;
		static const uint32_t QuadIndexCount = 6;

		static const uint32_t MaxQuads = 20000;
		static const uint32_t MaxVertices = MaxQuads * QuadVertexCount;
		static const uint32_t MaxIndices = MaxQuads * QuadIndexCount;
		static const uint32_t MaxTextureSlots = 32; // TODO: RenderCapabilities
		
		Ref<Pipeline> QuadPipeline;
		Ref<IndexBuffer> QuadIndexBuffer;

		std::vector<Ref<VertexBuffer>> QuadVertexBuffers;
		uint32_t VertexBufferIndex = 0;

		uint32_t IndexCount = 0;
		QuadVertex* QuadVertexBufferData = nullptr;
		QuadVertex* QuadVertexBufferDataPtr = nullptr;

		glm::vec4 QuadVertexPositions[4];

		Ref<Texture2D> WhiteTexture;

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1; // 0  = white texture

		Renderer2D::Statistics Stats;
	};

	static Renderer2DData* s_Data = new Renderer2DData();

	void Renderer2D::Init()
	{
		HZ_PROFILE_FUNCTION()
		
		Ref<VertexBuffer> vBuffer = VertexBuffer::Create(s_Data->MaxVertices * sizeof(QuadVertex));
		vBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position"     },
			{ ShaderDataType::Float2, "a_TexCoords"    },
			{ ShaderDataType::Float4, "a_Color"        },
			{ ShaderDataType::Float,  "a_TexIndex"     },
			{ ShaderDataType::Float,  "a_TilingFactor" }
		});
		s_Data->QuadVertexBuffers.push_back(vBuffer);

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
		
		s_Data->QuadIndexBuffer = IndexBuffer::Create(indices, s_Data->MaxIndices * sizeof(uint32_t));
		delete[] indices;
		
		// TextureColor shader
		const auto shader = Shader::Create(
			"TextureColor",
			"assets/Shaders/Compiled/TextureColor.vert.spv",
			"assets/Shaders/Compiled/TextureColor.frag.spv"
		);

		PipelineSpecification spec;
		spec.Shader = shader;
		spec.VertexBufferLayout = {
			{ ShaderDataType::Float3, "a_Position"     },
			{ ShaderDataType::Float2, "a_TexCoords"    },
			{ ShaderDataType::Float4, "a_Color"        },
			{ ShaderDataType::Float,  "a_TexIndex"     },
			{ ShaderDataType::Float,  "a_TilingFactor" }
		};
		s_Data->QuadPipeline = Pipeline::Create(spec);

		// Set the vertex positions used to transform the quads
		s_Data->QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		s_Data->QuadVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
		s_Data->QuadVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
		s_Data->QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

		// Create and set the white texture in the shader
		uint32_t content = 0xFFFFFFFF;
		s_Data->WhiteTexture = Texture2D::Create(&content, 1, 1, 4);
		s_Data->TextureSlots[0] = s_Data->WhiteTexture;
		s_Data->QuadPipeline->GetSpecification().Shader->BindTexture("u_Texture", 0, s_Data->WhiteTexture);
	}

	void Renderer2D::Shutdown()
	{
		HZ_PROFILE_FUNCTION()
		delete s_Data;
	}

	void Renderer2D::BeginScene(const Camera& camera)
	{
		HZ_PROFILE_FUNCTION()

		uint32_t vbIndex = s_Data->VertexBufferIndex = 0;
		
		s_Data->IndexCount = 0;
		s_Data->QuadVertexBufferData = (QuadVertex*)s_Data->QuadVertexBuffers[vbIndex]->Map();
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

		uint32_t vbIndex = s_Data->VertexBufferIndex;

		std::ptrdiff_t size = (char*)s_Data->QuadVertexBufferDataPtr - (char*)s_Data->QuadVertexBufferData;
		s_Data->QuadVertexBuffers[vbIndex]->Unmap(size);

		Renderer::Submit(
			s_Data->QuadPipeline,
			s_Data->QuadVertexBuffers[vbIndex],
			s_Data->QuadIndexBuffer,
			s_Data->IndexCount
		);

		s_Data->Stats.DrawCalls++;
	}

	void Renderer2D::OnResize()
	{
		for (auto& vBuffer : s_Data->QuadVertexBuffers)
		{
			// Framebuffer size has changed, need to re-build command buffer
			// (Note: This is MANDATORY on Vulkan and D3D12)
			Renderer::Submit(
				s_Data->QuadPipeline,
				vBuffer,
				s_Data->QuadIndexBuffer,
				s_Data->IndexCount
			);
		}
		
		Renderer::FlushCommandBuffer();
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		HZ_PROFILE_FUNCTION()

		constexpr glm::vec2 texCoords[] = {{ 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f }};
		float texIndex = 0.0f;
		float tilingFactor = 1.0f;

		if (s_Data->IndexCount >= Renderer2DData::MaxIndices)
			StartNewBatch();

		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, position);
		transform = glm::scale(transform, glm::vec3(size, 1.0f));

		for (uint32_t i = 0; i < s_Data->QuadVertexCount; i++)
		{
			s_Data->QuadVertexBufferDataPtr->Position = transform * s_Data->QuadVertexPositions[i];
			s_Data->QuadVertexBufferDataPtr->TexCoord = texCoords[i];
			s_Data->QuadVertexBufferDataPtr->Color = color;
			s_Data->QuadVertexBufferDataPtr->TexIndex = texIndex;
			s_Data->QuadVertexBufferDataPtr->TilingFactor = tilingFactor;
			s_Data->QuadVertexBufferDataPtr++;
		}

		s_Data->IndexCount += s_Data->QuadIndexCount;

		s_Data->Stats.QuadCount++;
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
		
		constexpr glm::vec2 texCoords[] = {{ 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f }};
		float texIndex = 0.0f;

		if (s_Data->IndexCount >= Renderer2DData::MaxIndices)
			StartNewBatch();

		for (uint32_t i = 1; i < s_Data->TextureSlotIndex; i++)
		{
			if (*s_Data->TextureSlots[i].get() == *texture.get())
			{
				texIndex = (float)i;
				break;
			}
		}

		if (texIndex == 0.0f)
		{
			// Save the texture in the texture slots array
			texIndex = (float)s_Data->TextureSlotIndex;
			s_Data->TextureSlots[s_Data->TextureSlotIndex] = texture;
			
			// Set the texture in the shader
			s_Data->QuadPipeline->GetSpecification().Shader->BindTexture(
				"u_Texture",
				s_Data->TextureSlotIndex,
				texture
			);

			// Increment the slot index
			s_Data->TextureSlotIndex++;
		}

		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, position);
		transform = glm::scale(transform, glm::vec3(size, 1.0f));

		for (uint32_t i = 0; i < s_Data->QuadVertexCount; i++)
		{
			s_Data->QuadVertexBufferDataPtr->Position = transform * s_Data->QuadVertexPositions[i];
			s_Data->QuadVertexBufferDataPtr->TexCoord = texCoords[i];
			s_Data->QuadVertexBufferDataPtr->Color = tintColor;
			s_Data->QuadVertexBufferDataPtr->TexIndex = texIndex;
			s_Data->QuadVertexBufferDataPtr->TilingFactor = tilingFactor;
			s_Data->QuadVertexBufferDataPtr++;
		}

		s_Data->IndexCount += s_Data->QuadIndexCount;

		s_Data->Stats.QuadCount++;
	}

	void Renderer2D::DrawQuad(
		const glm::vec2& position,
		const glm::vec2& size,
		const Ref<SubTexture2D>& subtexture,
		float tilingFactor,
		const glm::vec4& tintColor
	)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, subtexture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawQuad(
		const glm::vec3& position,
		const glm::vec2& size,
		const Ref<SubTexture2D>& subtexture,
		float tilingFactor,
		const glm::vec4& tintColor
	)
	{
		HZ_PROFILE_FUNCTION()

		const Ref<Texture2D> texture = subtexture->GetTexture();
		const glm::vec2* texCoords = subtexture->GetTexCoords();
		float texIndex = 0.0f;

		if (s_Data->IndexCount >= Renderer2DData::MaxIndices)
			StartNewBatch();

		for (uint32_t i = 1; i < s_Data->TextureSlotIndex; i++)
		{
			if (*s_Data->TextureSlots[i].get() == *texture.get())
			{
				texIndex = (float)i;
				break;
			}
		}

		if (texIndex == 0.0f)
		{
			// Save the texture in the texture slots array
			texIndex = (float)s_Data->TextureSlotIndex;
			s_Data->TextureSlots[s_Data->TextureSlotIndex] = texture;

			// Set the texture in the shader
			s_Data->QuadPipeline->GetSpecification().Shader->BindTexture(
				"u_Texture",
				s_Data->TextureSlotIndex,
				texture
			);

			// Increment the slot index
			s_Data->TextureSlotIndex++;
		}

		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, position);
		transform = glm::scale(transform, glm::vec3(size, 1.0f));

		for (uint32_t i = 0; i < s_Data->QuadVertexCount; i++)
		{
			s_Data->QuadVertexBufferDataPtr->Position = transform * s_Data->QuadVertexPositions[i];
			s_Data->QuadVertexBufferDataPtr->TexCoord = texCoords[i];
			s_Data->QuadVertexBufferDataPtr->Color = tintColor;
			s_Data->QuadVertexBufferDataPtr->TexIndex = texIndex;
			s_Data->QuadVertexBufferDataPtr->TilingFactor = tilingFactor;
			s_Data->QuadVertexBufferDataPtr++;
		}

		s_Data->IndexCount += s_Data->QuadIndexCount;

		s_Data->Stats.QuadCount++;
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
		
		constexpr glm::vec2 texCoords[] = {{ 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f }};
		float texIndex = 0.0f;
		float tilingFactor = 1.0f;

		if (s_Data->IndexCount >= Renderer2DData::MaxIndices)
			StartNewBatch();

		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, position);
		transform = glm::rotate(transform, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
		transform = glm::scale(transform, glm::vec3(size, 1.0f));

		for (uint32_t i = 0; i < s_Data->QuadVertexCount; i++)
		{
			s_Data->QuadVertexBufferDataPtr->Position = transform * s_Data->QuadVertexPositions[i];
			s_Data->QuadVertexBufferDataPtr->TexCoord = texCoords[i];
			s_Data->QuadVertexBufferDataPtr->Color = color;
			s_Data->QuadVertexBufferDataPtr->TexIndex = texIndex;
			s_Data->QuadVertexBufferDataPtr->TilingFactor = tilingFactor;
			s_Data->QuadVertexBufferDataPtr++;
		}

		s_Data->IndexCount += s_Data->QuadIndexCount;

		s_Data->Stats.QuadCount++;
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

		constexpr glm::vec2 texCoords[] = {{ 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f }};
		float texIndex = 0.0f;

		if (s_Data->IndexCount >= Renderer2DData::MaxIndices)
			StartNewBatch();

		for (uint32_t i = 1; i < s_Data->TextureSlotIndex; i++)
		{
			if (*s_Data->TextureSlots[i].get() == *texture.get())
			{
				texIndex = (float)i;
				break;
			}
		}

		if (texIndex == 0.0f)
		{
			// Save the texture in the texture slots array
			texIndex = (float)s_Data->TextureSlotIndex;
			s_Data->TextureSlots[s_Data->TextureSlotIndex] = texture;
			
			// Set the texture in the shader
			s_Data->QuadPipeline->GetSpecification().Shader->BindTexture(
				"u_Texture",
				s_Data->TextureSlotIndex,
				texture
			);

			// Increment the slot index
			s_Data->TextureSlotIndex++;
		}

		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, position);
		transform = glm::rotate(transform, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
		transform = glm::scale(transform, glm::vec3(size, 1.0f));

		for (uint32_t i = 0; i < s_Data->QuadVertexCount; i++)
		{
			s_Data->QuadVertexBufferDataPtr->Position = transform * s_Data->QuadVertexPositions[i];
			s_Data->QuadVertexBufferDataPtr->TexCoord = texCoords[i];
			s_Data->QuadVertexBufferDataPtr->Color = tintColor;
			s_Data->QuadVertexBufferDataPtr->TexIndex = texIndex;
			s_Data->QuadVertexBufferDataPtr->TilingFactor = tilingFactor;
			s_Data->QuadVertexBufferDataPtr++;
		}

		s_Data->IndexCount += s_Data->QuadIndexCount;

		s_Data->Stats.QuadCount++;
	}

	void Renderer2D::DrawRotatedQuad(
		const glm::vec2& position,
		const glm::vec2& size,
		float rotation,
		const Ref<SubTexture2D>& subtexture,
		float tilingFactor,
		const glm::vec4& tintColor
	)
	{
		DrawRotatedQuad(
			{ position.x, position.y, 0.0f },
			size,
			rotation,
			subtexture,
			tilingFactor,
			tintColor
		);
	}

	void Renderer2D::DrawRotatedQuad(
		const glm::vec3& position,
		const glm::vec2& size,
		float rotation,
		const Ref<SubTexture2D>& subtexture,
		float tilingFactor,
		const glm::vec4& tintColor
	)
	{
		HZ_PROFILE_FUNCTION()

		const Ref<Texture2D> texture = subtexture->GetTexture();
		const glm::vec2* texCoords = subtexture->GetTexCoords();
		float texIndex = 0.0f;

		if (s_Data->IndexCount >= Renderer2DData::MaxIndices)
			StartNewBatch();

		for (uint32_t i = 1; i < s_Data->TextureSlotIndex; i++)
		{
			if (*s_Data->TextureSlots[i].get() == *texture.get())
			{
				texIndex = (float)i;
				break;
			}
		}

		if (texIndex == 0.0f)
		{
			// Save the texture in the texture slots array
			texIndex = (float)s_Data->TextureSlotIndex;
			s_Data->TextureSlots[s_Data->TextureSlotIndex] = texture;

			// Set the texture in the shader
			s_Data->QuadPipeline->GetSpecification().Shader->BindTexture(
				"u_Texture",
				s_Data->TextureSlotIndex,
				texture
			);

			// Increment the slot index
			s_Data->TextureSlotIndex++;
		}

		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, position);
		transform = glm::rotate(transform, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
		transform = glm::scale(transform, glm::vec3(size, 1.0f));

		for (uint32_t i = 0; i < s_Data->QuadVertexCount; i++)
		{
			s_Data->QuadVertexBufferDataPtr->Position = transform * s_Data->QuadVertexPositions[i];
			s_Data->QuadVertexBufferDataPtr->TexCoord = texCoords[i];
			s_Data->QuadVertexBufferDataPtr->Color = tintColor;
			s_Data->QuadVertexBufferDataPtr->TexIndex = texIndex;
			s_Data->QuadVertexBufferDataPtr->TilingFactor = tilingFactor;
			s_Data->QuadVertexBufferDataPtr++;
		}

		s_Data->IndexCount += s_Data->QuadIndexCount;

		s_Data->Stats.QuadCount++;
	}
	
	void Renderer2D::ResetStatistics()
	{
		memset(&s_Data->Stats, 0, sizeof(Renderer2D::Statistics));
	}

	Renderer2D::Statistics Renderer2D::GetStatistics()
	{
		return s_Data->Stats;
	}

	void Renderer2D::StartNewBatch()
	{
		EndScene();

		// Increment the vertex buffer index
		uint32_t vbIndex = ++s_Data->VertexBufferIndex;

		// If the vertex buffer in the index has not been created yet
		if (s_Data->QuadVertexBuffers.size() <= s_Data->VertexBufferIndex)
		{
			// Create a new vertex buffer
			Ref<VertexBuffer> vBuffer = VertexBuffer::Create(s_Data->MaxVertices * sizeof(QuadVertex));
			vBuffer->SetLayout(s_Data->QuadVertexBuffers[0]->GetLayout());
			s_Data->QuadVertexBuffers.push_back(vBuffer);
		}
		
		s_Data->IndexCount = 0;
		s_Data->QuadVertexBufferData = (QuadVertex*)s_Data->QuadVertexBuffers[vbIndex]->Map();
		s_Data->QuadVertexBufferDataPtr = s_Data->QuadVertexBufferData;
	}

}
