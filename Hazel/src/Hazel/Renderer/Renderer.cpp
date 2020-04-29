#include "hzpch.h"
#include "Renderer.h"

#include "Hazel/Renderer/Renderer2D.h"
#include "Hazel/Renderer/RenderCommandBuffer.h"

#include "Platform/OpenGL/OpenGLShader.h"

namespace Hazel {

	Scope<Renderer::SceneData> Renderer::s_SceneData = CreateScope<Renderer::SceneData>();

	GraphicsContext* Renderer::s_Context = nullptr;
	
	static Ref<RenderCommandBuffer> s_RenderCommandBuffer;
	
	void Renderer::Init(const Scope<GraphicsContext>& context)
	{
		s_Context = context.get();

		RendererAPI::InitAPI();

		s_RenderCommandBuffer = RenderCommandBuffer::Create();
		
		//RenderCommand::Init();
		Renderer2D::Init();
	}

	void Renderer::BeginScene(OrthographicCamera& camera)
	{
		s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}

	void Renderer::EndScene() {}

	void Renderer::BeginRenderPass(const Ref<RenderPass>& renderPass)
	{
		s_Context->Prepare();
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

}