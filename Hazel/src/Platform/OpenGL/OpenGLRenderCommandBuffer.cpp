#include "hzpch.h"
#include "OpenGLRenderCommandBuffer.h"

#include "OpenGLBuffer.h"
#include "OpenGLTexture.h"
#include "OpenGLShader.h"
#include "OpenGLPipeline.h"
#include "OpenGLRenderPass.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

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
			for (auto texture : gl_Shader->GetTexturesVector())
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

				GLenum baseType = ShaderDataTypeToOpenGLBaseType(element.Type);
				if (baseType == GL_INT)
				{
					glVertexAttribIPointer(
						layoutIndex,
						element.GetComponentCount(),
						baseType,
						pipeline->GetSpecification().VertexBufferLayout.GetStride(),
						(const void*)(intptr_t)element.Offset
					);
				}
				else
				{
					glVertexAttribPointer(
						layoutIndex,
						element.GetComponentCount(),
						baseType,
						element.Normalized ? GL_TRUE : GL_FALSE,
						pipeline->GetSpecification().VertexBufferLayout.GetStride(),
						(const void*)(intptr_t)element.Offset
					);
				}

				layoutIndex++;
			}

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_IndexBuffer->GetRendererId());
			glDrawElements(
				GL_TRIANGLES,
				indexCount == 0 ? (indexBuffer->GetSize() / sizeof(uint32_t)) : indexCount,
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
				auto shaderTextures = gl_Shader->GetTextures();
				auto materialTextures = material->GetTextures();

				for (auto it = shaderTextures.begin(); it != shaderTextures.end(); it++)
				{
					// Assign material texture, if exists
					auto texture = materialTextures[it->first];

					// If not, assign the texture in the shader
					if (texture == nullptr) texture = it->second;

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

				// Bind Material uniform buffer
				if (gl_Shader->HasVSMaterialUniformBuffer() || gl_Shader->HasPSMaterialUniformBuffer())
				{
					auto ubuffer = gl_Shader->GetMaterialUniformBuffer();
					auto gl_UBuffer = dynamic_cast<OpenGLUniformBuffer*>(ubuffer);

					uint32_t size = gl_Shader->GetMaterialUniformBufferAlignment();
					uint32_t offset = material->GetUniformBufferIndex() * size;

					ubuffer->Bind();
					glBindBufferRange(GL_UNIFORM_BUFFER, gl_UBuffer->GetBinding(), gl_UBuffer->GetRendererId(), offset, size);
					ubuffer->Unbind();
				}

				glBindVertexArray(gl_Pipeline->GetVertexArrayRendererId());
				glBindBuffer(GL_ARRAY_BUFFER, gl_VertexBuffer->GetRendererId());

				uint32_t layoutIndex = 0;
				for (const auto& element : pipeline->GetSpecification().VertexBufferLayout)
				{
					glEnableVertexAttribArray(layoutIndex);

					GLenum baseType = ShaderDataTypeToOpenGLBaseType(element.Type);
					if (baseType == GL_INT)
					{
						glVertexAttribIPointer(
							layoutIndex,
							element.GetComponentCount(),
							baseType,
							pipeline->GetSpecification().VertexBufferLayout.GetStride(),
							(const void*)(intptr_t)element.Offset
						);
					}
					else
					{
						glVertexAttribPointer(
							layoutIndex,
							element.GetComponentCount(),
							baseType,
							element.Normalized ? GL_TRUE : GL_FALSE,
							pipeline->GetSpecification().VertexBufferLayout.GetStride(),
							(const void*)(intptr_t)element.Offset
						);
					}

					layoutIndex++;
				}

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_IndexBuffer->GetRendererId());
				glDrawElements(
					GL_TRIANGLES,
					indexCount == 0 ? (indexBuffer->GetSize() / sizeof(uint32_t)) : indexCount,
					GL_UNSIGNED_INT,
					nullptr
				);
			});
	}

	void OpenGLRenderCommandBuffer::Submit(
		const Ref<Pipeline>& pipeline,
		const Ref<VertexBuffer>& vertexBuffer,
		const Ref<IndexBuffer>& indexBuffer,
		const Ref<MaterialInstance>& materialInstance,
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
				auto shaderTextures = gl_Shader->GetTextures();
				auto materialTextures = materialInstance->GetMaterial()->GetTextures();
				auto instanceTextures = materialInstance->GetTextures();

				for (auto it = shaderTextures.begin(); it != shaderTextures.end(); it++)
				{
					// Assign instance texture, if exists
					auto texture = instanceTextures[it->first];

					// If not, assign the texture in the base material, if exists
					if (texture == nullptr) texture = materialTextures[it->first];

					// If not, assign the texture in the shader
					if (texture == nullptr) texture = it->second;

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

				// Bind Material uniform buffer
				if (gl_Shader->HasVSMaterialUniformBuffer() || gl_Shader->HasPSMaterialUniformBuffer())
				{
					auto ubuffer = gl_Shader->GetMaterialUniformBuffer();
					auto gl_UBuffer = dynamic_cast<OpenGLUniformBuffer*>(ubuffer);

					uint32_t size = gl_Shader->GetMaterialUniformBufferAlignment();
					uint32_t offset = materialInstance->GetUniformBufferIndex() * size;

					ubuffer->Bind();
					glBindBufferRange(GL_UNIFORM_BUFFER, gl_UBuffer->GetBinding(), gl_UBuffer->GetRendererId(), offset, size);
					ubuffer->Unbind();
				}

				glBindVertexArray(gl_Pipeline->GetVertexArrayRendererId());
				glBindBuffer(GL_ARRAY_BUFFER, gl_VertexBuffer->GetRendererId());

				uint32_t layoutIndex = 0;
				for (const auto& element : pipeline->GetSpecification().VertexBufferLayout)
				{
					glEnableVertexAttribArray(layoutIndex);

					GLenum baseType = ShaderDataTypeToOpenGLBaseType(element.Type);
					if (baseType == GL_INT)
					{
						glVertexAttribIPointer(
							layoutIndex,
							element.GetComponentCount(),
							baseType,
							pipeline->GetSpecification().VertexBufferLayout.GetStride(),
							(const void*)(intptr_t)element.Offset
						);
					}
					else
					{
						glVertexAttribPointer(
							layoutIndex,
							element.GetComponentCount(),
							baseType,
							element.Normalized ? GL_TRUE : GL_FALSE,
							pipeline->GetSpecification().VertexBufferLayout.GetStride(),
							(const void*)(intptr_t)element.Offset
						);
					}

					layoutIndex++;
				}

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_IndexBuffer->GetRendererId());
				glDrawElements(
					GL_TRIANGLES,
					indexCount == 0 ? (indexBuffer->GetSize() / sizeof(uint32_t)) : indexCount,
					GL_UNSIGNED_INT,
					nullptr
				);
			});
	}

	void OpenGLRenderCommandBuffer::SubmitMesh(
		const Ref<Pipeline>& pipeline,
		const Ref<Mesh>& mesh,
		const glm::mat4& transform,
		const Ref<MaterialInstance>& overrideMaterial
	)
	{
		OpenGLPipeline* gl_Pipeline = static_cast<OpenGLPipeline*>(pipeline.get());
		OpenGLShader* gl_Shader = static_cast<OpenGLShader*>(mesh->GetMeshShader().get());
		OpenGLVertexBuffer* gl_VertexBuffer = static_cast<OpenGLVertexBuffer*>(mesh->GetVertexBuffer().get());
		OpenGLIndexBuffer* gl_IndexBuffer = static_cast<OpenGLIndexBuffer*>(mesh->GetIndexBuffer().get());

		m_Queue.push_back([=]()
			{
				// Set bone matrices
				if (mesh->IsAnimated())
				{
					uint32_t size = mesh->m_BoneTransforms.size() * sizeof(glm::mat4);
					auto boneTransforms = mesh->m_BoneTransforms.data();
					gl_Shader->SetUniformBuffer("u_AnimData", boneTransforms, size);
				}

				// TODO: Should get pipeline info from Pipeline object
				// Set pipeline state
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_LESS);

				glUseProgram(gl_Shader->GetRendererId());

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

					GLenum baseType = ShaderDataTypeToOpenGLBaseType(element.Type);
					if (baseType == GL_INT)
					{
						glVertexAttribIPointer(
							layoutIndex,
							element.GetComponentCount(),
							baseType,
							pipeline->GetSpecification().VertexBufferLayout.GetStride(),
							(const void*)(intptr_t)element.Offset
						);
					}
					else
					{
						glVertexAttribPointer(
							layoutIndex,
							element.GetComponentCount(),
							baseType,
							element.Normalized ? GL_TRUE : GL_FALSE,
							pipeline->GetSpecification().VertexBufferLayout.GetStride(),
							(const void*)(intptr_t)element.Offset
						);
					}

					layoutIndex++;
				}

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_IndexBuffer->GetRendererId());
				
				auto& materials = mesh->GetMaterials();
				for (Submesh& submesh : mesh->GetSubmeshes())
				{
					auto material = materials[submesh.MaterialIndex];
					auto shaderTextures = gl_Shader->GetTextures();
					auto materialTextures = material->GetTextures();
					uint32_t slot = 0;

					// TODO: Should bind textures to correspondents bindings
					// Now, we are binding sequencially
					for (auto it = shaderTextures.begin(); it != shaderTextures.end(); it++)
					{
						// Assign material texture, if exists
						auto texture = materialTextures[it->first];

						// If not, assign the texture in the shader
						if (texture == nullptr) texture = it->second;

						OpenGLTexture2D* gl_Texture = static_cast<OpenGLTexture2D*>(texture.get());
						gl_Shader->SetInt(gl_Texture->GetSamplerName(), slot);
						texture->Bind(slot);
						slot++;
					}

					// Bind Material uniform buffer
					if (gl_Shader->HasVSMaterialUniformBuffer() || gl_Shader->HasPSMaterialUniformBuffer())
					{
						auto ubuffer = gl_Shader->GetMaterialUniformBuffer();
						auto gl_UBuffer = dynamic_cast<OpenGLUniformBuffer*>(ubuffer);

						uint32_t size = gl_Shader->GetMaterialUniformBufferAlignment();
						uint32_t offset = material->GetUniformBufferIndex() * size;

						ubuffer->Bind();
						glBindBufferRange(GL_UNIFORM_BUFFER, gl_UBuffer->GetBinding(), gl_UBuffer->GetRendererId(), offset, size);
						ubuffer->Unbind();
					}

					glm::mat4 trans = transform * submesh.Transform;
					gl_Shader->SetUniformBufferParam("u_SceneData", "Transform", &trans, sizeof(glm::mat4));

					glDrawElementsBaseVertex(
						GL_TRIANGLES,
						submesh.IndexCount,
						GL_UNSIGNED_INT,
						(void*)(submesh.BaseIndex * sizeof(uint32_t)),
						submesh.BaseVertex
					);
				}
			});
	}

	void OpenGLRenderCommandBuffer::Flush()
	{
		for (auto& func : m_Queue)
			func();

		m_Queue.clear();
	}
	
}
