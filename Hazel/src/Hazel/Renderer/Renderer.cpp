#include "hzpch.h"
#include "Renderer.h"

#include "Hazel/Renderer/Renderer2D.h"
#include "Hazel/Renderer/RenderCommandBuffer.h"

#include "Platform/OpenGL/OpenGLShader.h"

namespace Hazel {

	struct RendererData
	{
		Scope<ShaderLibrary> ShaderLibrary;

		Ref<Pipeline> FullscreenQuadPipeline;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<IndexBuffer> QuadIndexBuffer;
		uint32_t QuadIndexCount;
	};

	static RendererData s_Data;

	Scope<Renderer::SceneData> Renderer::s_SceneData = CreateScope<Renderer::SceneData>();

	GraphicsContext* Renderer::s_Context = nullptr;
	
	static Ref<RenderCommandBuffer> s_RenderCommandBuffer;
	
	void Renderer::Init(const Scope<GraphicsContext>& context)
	{
		s_Context = context.get();
		s_Data.ShaderLibrary = CreateScope<ShaderLibrary>();

		RendererAPI::InitAPI();

		// Load static and skinned PBR Renderer shaders
		Renderer::GetShaderLibrary()->Load(
			"PBR_Static",
			"assets/Shaders/Compiled/PBR_Static.vert.spv",
			"assets/Shaders/Compiled/PBR.frag.spv"
		);
		Renderer::GetShaderLibrary()->Load(
			"PBR_Anim",
			"assets/Shaders/Compiled/PBR_Anim.vert.spv",
			"assets/Shaders/Compiled/PBR.frag.spv"
		);

		s_RenderCommandBuffer = RenderCommandBuffer::Create();

		// Create fullscreen quad

		float x = -1, y = -1;
		float width = 2, height = 2;

		struct QuadVertex
		{
			glm::vec3 Position;
			glm::vec2 TexCoord;
		};

		QuadVertex* data = new QuadVertex[4];

		data[0].Position = { x, y, 0.1f };
		data[0].TexCoord = { 0.0f, 0.0f };

		data[1].Position = { x + width, y, 0.1f };
		data[1].TexCoord = { 1.0f, 0.0f };

		data[2].Position = { x + width, y + height, 0.1f };
		data[2].TexCoord = { 1.0f, 1.0f };

		data[3].Position = { x, y + height, 0.1f };
		data[3].TexCoord = { 0.0f, 1.0f };

		s_Data.QuadVertexBuffer = VertexBuffer::Create(data, 4 * sizeof(QuadVertex));
		s_Data.QuadVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		});

		uint32_t indices[6] = { 0, 1, 2, 2, 3, 0 };
		s_Data.QuadIndexBuffer = IndexBuffer::Create(indices, 6 * sizeof(uint32_t));
		s_Data.QuadIndexCount = 6;

		//PipelineSpecification spec;
		//spec.Shader = shader;
		//spec.VertexBufferLayout = {
		//	{ ShaderDataType::Float3, "a_Position" },
		//	{ ShaderDataType::Float2, "a_TexCoord" }
		//};
		//s_Data.FullscreenQuadPipeline = Pipeline::Create(spec);
		
		//RenderCommand::Init();
		Renderer2D::Init();
	}

	void Renderer::Prepare()
	{
		s_Context->Prepare();
	}

	void Renderer::BeginScene(OrthographicCamera& camera)
	{
		s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}

	void Renderer::EndScene() {}

	void Renderer::BeginRenderPass(const Ref<RenderPass>& renderPass)
	{
		s_RenderCommandBuffer->BeginRenderPass(renderPass);
	}

	void Renderer::EndRenderPass()
	{
		s_RenderCommandBuffer->EndRenderPass();
	}

	void Renderer::Submit(
		const Ref<Pipeline>& pipeline,
		const Ref<VertexBuffer>& vertexBuffer,
		const Ref<IndexBuffer>& indexBuffer,
		uint32_t indexCount
	)
	{
		s_RenderCommandBuffer->Submit(pipeline, vertexBuffer, indexBuffer, indexCount);
	}

	void Renderer::Submit(
		const Ref<Pipeline>& pipeline,
		const Ref<VertexBuffer>& vertexBuffer,
		const Ref<IndexBuffer>& indexBuffer,
		const Ref<Material>& material,
		uint32_t indexCount
	)
	{
		s_RenderCommandBuffer->Submit(pipeline, vertexBuffer, indexBuffer, material, indexCount);
	}

	void Renderer::Submit(
		const Ref<Pipeline>& pipeline,
		const Ref<VertexBuffer>& vertexBuffer,
		const Ref<IndexBuffer>& indexBuffer,
		const Ref<MaterialInstance>& materialInstance,
		uint32_t indexCount
	)
	{
		s_RenderCommandBuffer->Submit(pipeline, vertexBuffer, indexBuffer, materialInstance, indexCount);
	}

	void Renderer::Submit(
		const Ref<Shader>& shader,
		const Ref<VertexArray>& vertexArray,
		const glm::mat4& transform
	)
	{
		auto glShader = std::dynamic_pointer_cast<OpenGLShader>(shader);

		glShader->Bind();
		glShader->UploadUniformMat4("u_SceneData.ViewProjection", s_SceneData->ViewProjectionMatrix);
		glShader->UploadUniformMat4("u_SceneData.Transform", transform);

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

	void Renderer::SubmitFullscreenQuad(
		const Ref<Pipeline>& pipeline,
		const Ref<MaterialInstance>& material,
		const glm::mat4& transform
	)
	{
		Submit(
			pipeline,
			s_Data.QuadVertexBuffer,
			s_Data.QuadIndexBuffer,
			material,
			s_Data.QuadIndexCount
		);
	}

	void Renderer::SubmitMesh(
		const Ref<Pipeline>& pipeline,
		const Ref<Mesh>& mesh,
		const glm::mat4& transform,
		const Ref<MaterialInstance>& overrideMaterial
	)
	{
		s_RenderCommandBuffer->SubmitMesh(pipeline, mesh, transform, overrideMaterial);
	}
	
	void Renderer::FlushCommandBuffer()
	{
		s_RenderCommandBuffer->Flush();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
		RenderCommand::Resize(0, 0, width, height);
		//Renderer2D::OnResize();
	}

	const Scope<ShaderLibrary>& Renderer::GetShaderLibrary()
	{
		return s_Data.ShaderLibrary;
	}

}