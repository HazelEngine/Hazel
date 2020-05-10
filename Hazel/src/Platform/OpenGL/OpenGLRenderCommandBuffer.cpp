#include "hzpch.h"
#include "OpenGLRenderCommandBuffer.h"

#include "OpenGLBuffer.h"
#include "OpenGLTexture.h"
#include "OpenGLShader.h"
#include "OpenGLPipeline.h"
#include "OpenGLRenderPass.h"

#include <glad/glad.h>

namespace Hazel {
	
	OpenGLRenderCommandBuffer::OpenGLRenderCommandBuffer() {}

	OpenGLRenderCommandBuffer::~OpenGLRenderCommandBuffer() {}

	void OpenGLRenderCommandBuffer::BeginRenderPass(const Ref<RenderPass>& renderPass)
	{
		OpenGLRenderPass* gl_RenderPass = static_cast<OpenGLRenderPass*>(renderPass.get());
		m_Queue.push_back([=]()
		{
			// TODO: Bind framebuffer here

			auto& clearColor = gl_RenderPass->GetSpecification().TargetFramebuffer->GetSpecification().ClearColor;
			glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		});
	}

	void OpenGLRenderCommandBuffer::EndRenderPass() {}

	void OpenGLRenderCommandBuffer::Submit(
		const Ref<Pipeline>& pipeline,
		const Ref<VertexBuffer>& vertexBuffer,
		const Ref<IndexBuffer>& indexBuffer,
		uint32_t indexCount
	)
	{
		OpenGLPipeline* gl_Pipeline = static_cast<OpenGLPipeline*>(pipeline.get());
		OpenGLShader* gl_Shader = static_cast<OpenGLShader*>(pipeline->GetSpecification().Shader.get());
		OpenGLVertexBuffer* gl_VertexBuffer = static_cast<OpenGLVertexBuffer*>(vertexBuffer.get());
		OpenGLIndexBuffer* gl_IndexBuffer = static_cast<OpenGLIndexBuffer*>(indexBuffer.get());

		m_Queue.push_back([=]()
		{
			// TODO: Should get pipeline info from Pipeline object
			// Set pipeline state
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glUseProgram(gl_Shader->GetRendererId());

			uint32_t slot = 0;
			for (auto texture : gl_Shader->GetTextures())
			{
				OpenGLTexture2D* gl_Texture = static_cast<OpenGLTexture2D*>(texture.get());
				gl_Shader->SetInt(gl_Texture->GetSamplerName(), slot);
				texture->Bind(slot);
				slot++;
			}

			for (auto ubuffer : gl_Shader->GetUniformBuffers())
			{
				OpenGLUniformBuffer* gl_UBuffer = static_cast<OpenGLUniformBuffer*>(ubuffer.get());
				ubuffer->Bind();
				glBindBufferBase(GL_UNIFORM_BUFFER, gl_UBuffer->GetBinding(), gl_UBuffer->GetRendererId());
				ubuffer->Unbind();
			}

			glBindVertexArray(gl_Pipeline->GetVertexArrayRendererId());
			glBindBuffer(GL_ARRAY_BUFFER, gl_VertexBuffer->GetRendererId());

			uint32_t layoutIndex = 0;
			for (const auto& element : pipeline->GetSpecification().VertexBufferLayout)
			{
				glEnableVertexAttribArray(layoutIndex);
				glVertexAttribPointer(
					layoutIndex,
					element.GetComponentCount(),
					ShaderDataTypeToOpenGLBaseType(element.Type),
					element.Normalized ? GL_TRUE : GL_FALSE,
					pipeline->GetSpecification().VertexBufferLayout.GetStride(),
					(const void*)(intptr_t)element.Offset
				);
				layoutIndex++;
			}

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_IndexBuffer->GetRendererId());
			glDrawElements(
				GL_TRIANGLES,
				indexCount == 0 ? indexBuffer->GetCount() : indexCount,
				GL_UNSIGNED_INT,
				nullptr
			);
		});
	}

	void OpenGLRenderCommandBuffer::Submit(
		const Ref<Pipeline>& pipeline, 
		const Ref<VertexBuffer>& vertexBuffer,
		const Ref<IndexBuffer>& indexBuffer,
		const Ref<Material>& material, 
		uint32_t indexCount
	)
	{
	}

	void OpenGLRenderCommandBuffer::Submit(
		const Ref<Pipeline>& pipeline,
		const Ref<VertexBuffer>& vertexBuffer,
		const Ref<IndexBuffer>& indexBuffer,
		const Ref<MaterialInstance>& materialInstance,
		uint32_t indexCount
	)
	{
	}

	void OpenGLRenderCommandBuffer::Flush()
	{
		for (auto& func : m_Queue)
			func();

		m_Queue.clear();
	}
	
}
