#include "hzpch.h"
#include "VulkanRenderCommandBuffer.h"

#include "VulkanBuffer.h"
#include "VulkanShader.h"
#include "VulkanTexture.h"
#include "VulkanContext.h"
#include "VulkanPipeline.h"
#include "VulkanRenderPass.h"
#include "VulkanUtils.h"

namespace Hazel {

	VulkanRenderCommandBuffer::VulkanRenderCommandBuffer() {}

	VulkanRenderCommandBuffer::~VulkanRenderCommandBuffer() {}

	void VulkanRenderCommandBuffer::BeginRenderPass(const Ref<RenderPass>& renderPass)
	{
		VulkanRenderPass* vk_RenderPass = static_cast<VulkanRenderPass*>(renderPass.get());

		m_Queue.push_back([=](const VkCommandBuffer& drawCommandBuffer, const VkFramebuffer& framebuffer)
		{
			VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());

			auto& clearColor = vk_RenderPass->GetSpecification().TargetFramebuffer->GetSpecification().ClearColor;

			VkClearValue clearValues[2];
			clearValues[0].color = {{ clearColor.r, clearColor.g, clearColor.b, clearColor.a }};
			clearValues[1].depthStencil = { 1.0f, 0 };

			VkRenderPassBeginInfo renderPassBeginInfo = {};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.pNext = nullptr;
			renderPassBeginInfo.renderPass = vk_Context->GetRenderPass();
			renderPassBeginInfo.renderArea.offset.x = 0;
			renderPassBeginInfo.renderArea.offset.y = 0;
			renderPassBeginInfo.renderArea.extent.width = vk_Context->GetWidth();
			renderPassBeginInfo.renderArea.extent.height = vk_Context->GetHeight();
			renderPassBeginInfo.clearValueCount = 2;
			renderPassBeginInfo.pClearValues = clearValues;
			
			renderPassBeginInfo.framebuffer = framebuffer;

			// Start the first sub pass specified in our default render pass setup by the base class
			// This will clear the color and depth attachment
			vkCmdBeginRenderPass(drawCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		});
	}

	void VulkanRenderCommandBuffer::EndRenderPass()
	{
		m_Queue.push_back([=](const VkCommandBuffer& drawCommandBuffer, const VkFramebuffer& framebuffer)
		{
			vkCmdEndRenderPass(drawCommandBuffer);
		});
	}

	void VulkanRenderCommandBuffer::Submit(
		const Ref<Pipeline>& pipeline,
		const Ref<VertexBuffer>& vertexBuffer,
		const Ref<IndexBuffer>& indexBuffer,
		uint32_t indexCount
	)
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());

		m_Queue.push_back([=](const VkCommandBuffer& drawCommandBuffer, const VkFramebuffer& framebuffer)
		{
			// TODO: Viewport and scissor state should come from pipeline (?) potentially
			VkViewport viewport = {};
			viewport.x = 0;
			viewport.y = (float)vk_Context->GetHeight();
			viewport.width = (float)vk_Context->GetWidth();
			viewport.height = -(float)vk_Context->GetHeight();
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(drawCommandBuffer, 0, 1, &viewport);

			VkRect2D scissor = {};
			scissor.offset.x = 0;
			scissor.offset.y = 0;
			scissor.extent.width = vk_Context->GetWidth();
			scissor.extent.height = vk_Context->GetHeight();
			vkCmdSetScissor(drawCommandBuffer, 0, 1, &scissor);

			VulkanPipeline* vk_Pipeline = static_cast<VulkanPipeline*>(pipeline.get());
			VulkanShader* vk_Shader = dynamic_cast<VulkanShader*>(vk_Pipeline->GetSpecification().Shader.get());

			// If has a DescriptionSet, bind it
			if (vk_Shader->GetGlobalDescriptorSet())
			{
				auto textureSets = vk_Shader->GetTextureDescriptorSets();

				std::vector<VkDescriptorSet> sets;
				sets.push_back(vk_Shader->GetGlobalDescriptorSet());
				sets.insert(sets.end(), textureSets.begin(), textureSets.end());

				bool hasVSMaterialBuffer = vk_Shader->HasVSMaterialUniformBuffer();
				bool hasPSMaterialBuffer = vk_Shader->HasPSMaterialUniformBuffer();
				bool hasMaterialBuffer = hasVSMaterialBuffer || hasPSMaterialBuffer;
				uint32_t offset = 0;

				vkCmdBindDescriptorSets(
					drawCommandBuffer,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					vk_Shader->GetPipelineLayout(),
					0,
					sets.size(),
					sets.data(),
					hasMaterialBuffer ? 1 : 0,
					hasMaterialBuffer ? &offset : nullptr
				);
			}

			vkCmdBindPipeline(
				drawCommandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				vk_Pipeline->GetVulkanPipeline()
			);

			VkDeviceSize offsets[1] = { 0 };
			VulkanVertexBuffer* vk_VertexBuffer = static_cast<VulkanVertexBuffer*>(vertexBuffer.get());
			vkCmdBindVertexBuffers(drawCommandBuffer, 0, 1, &vk_VertexBuffer->GetDeviceBuffer(), offsets);

			VulkanIndexBuffer* vk_IndexBuffer = static_cast<VulkanIndexBuffer*>(indexBuffer.get());
			vkCmdBindIndexBuffer(drawCommandBuffer, vk_IndexBuffer->GetDeviceBuffer(), 0, VK_INDEX_TYPE_UINT32);

			// Draw
			vkCmdDrawIndexed(drawCommandBuffer, indexCount == 0 ? (indexBuffer->GetSize() / sizeof(uint32_t)) : indexCount, 1, 0, 0, 1);
		});
	}

	void VulkanRenderCommandBuffer::Submit(
		const Ref<Pipeline>& pipeline,
		const Ref<VertexBuffer>& vertexBuffer,
		const Ref<IndexBuffer>& indexBuffer,
		const Ref<Material>& material,
		uint32_t indexCount
	)
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());

		m_Queue.push_back([=](const VkCommandBuffer& drawCommandBuffer, const VkFramebuffer& framebuffer)
		{
			// TODO: Viewport and scissor state should come from pipeline (?) potentially
			VkViewport viewport = {};
			viewport.x = 0;
			viewport.y = (float)vk_Context->GetHeight();
			viewport.width = (float)vk_Context->GetWidth();
			viewport.height = -(float)vk_Context->GetHeight();
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(drawCommandBuffer, 0, 1, &viewport);

			VkRect2D scissor = {};
			scissor.offset.x = 0;
			scissor.offset.y = 0;
			scissor.extent.width = vk_Context->GetWidth();
			scissor.extent.height = vk_Context->GetHeight();
			vkCmdSetScissor(drawCommandBuffer, 0, 1, &scissor);

			VulkanPipeline* vk_Pipeline = static_cast<VulkanPipeline*>(pipeline.get());
			VulkanShader* vk_Shader = dynamic_cast<VulkanShader*>(vk_Pipeline->GetSpecification().Shader.get());

			// If has a DescriptionSet, bind it
			if (vk_Shader->GetGlobalDescriptorSet())
			{
				auto materialTextures = material->GetTextures();
				uint32_t index = 0;

				// If has textures set, bind it, otherwise, bind the default shader texture
				auto textureSets = vk_Shader->GetTextureDescriptorSets();
				for (auto it = materialTextures.begin(); it != materialTextures.end(); it++)
				{
					uint32_t index = vk_Shader->GetTextureDescriptorSetIndex(it->first);
					auto textureSet = vk_Shader->GetTexturePoolDescriptorSet(it->second);
					textureSets[index] = textureSet;
				}

				std::vector<VkDescriptorSet> sets;
				sets.push_back(vk_Shader->GetGlobalDescriptorSet());
				sets.insert(sets.end(), textureSets.begin(), textureSets.end());

				bool hasVSMaterialBuffer = vk_Shader->HasVSMaterialUniformBuffer();
				bool hasPSMaterialBuffer = vk_Shader->HasPSMaterialUniformBuffer();
				bool hasMaterialBuffer = hasVSMaterialBuffer || hasPSMaterialBuffer;
				uint32_t offset = material->GetUniformBufferIndex() * vk_Shader->GetMaterialUniformBufferAlignment();
				
				vkCmdBindDescriptorSets(
					drawCommandBuffer,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					vk_Shader->GetPipelineLayout(),
					0,
					sets.size(),
					sets.data(),
					hasMaterialBuffer ? 1 : 0,
					hasMaterialBuffer ? &offset : nullptr
				);
			}

			vkCmdBindPipeline(
				drawCommandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				vk_Pipeline->GetVulkanPipeline()
			);

			VkDeviceSize offsets[1] = { 0 };
			VulkanVertexBuffer* vk_VertexBuffer = static_cast<VulkanVertexBuffer*>(vertexBuffer.get());
			vkCmdBindVertexBuffers(drawCommandBuffer, 0, 1, &vk_VertexBuffer->GetDeviceBuffer(), offsets);

			VulkanIndexBuffer* vk_IndexBuffer = static_cast<VulkanIndexBuffer*>(indexBuffer.get());
			vkCmdBindIndexBuffer(drawCommandBuffer, vk_IndexBuffer->GetDeviceBuffer(), 0, VK_INDEX_TYPE_UINT32);

			// Draw
			vkCmdDrawIndexed(drawCommandBuffer, indexCount == 0 ? (indexBuffer->GetSize() / sizeof(uint32_t)) : indexCount, 1, 0, 0, 1);
		});
	}

	void VulkanRenderCommandBuffer::Submit(
		const Ref<Pipeline>& pipeline,
		const Ref<VertexBuffer>& vertexBuffer,
		const Ref<IndexBuffer>& indexBuffer,
		const Ref<MaterialInstance>& materialInstance,
		uint32_t indexCount
	)
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());

		m_Queue.push_back([=](const VkCommandBuffer& drawCommandBuffer, const VkFramebuffer& framebuffer)
		{
			// TODO: Viewport and scissor state should come from pipeline (?) potentially
			VkViewport viewport = {};
			viewport.x = 0;
			viewport.y = (float)vk_Context->GetHeight();
			viewport.width = (float)vk_Context->GetWidth();
			viewport.height = -(float)vk_Context->GetHeight();
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(drawCommandBuffer, 0, 1, &viewport);

			VkRect2D scissor = {};
			scissor.offset.x = 0;
			scissor.offset.y = 0;
			scissor.extent.width = vk_Context->GetWidth();
			scissor.extent.height = vk_Context->GetHeight();
			vkCmdSetScissor(drawCommandBuffer, 0, 1, &scissor);

			VulkanPipeline* vk_Pipeline = static_cast<VulkanPipeline*>(pipeline.get());
			VulkanShader* vk_Shader = dynamic_cast<VulkanShader*>(vk_Pipeline->GetSpecification().Shader.get());

			// If has a DescriptionSet, bind it
			if (vk_Shader->GetGlobalDescriptorSet())
			{
				auto instanceTextures = materialInstance->GetTextures();
				auto materialTextures = materialInstance->GetMaterial()->GetTextures();
				uint32_t index = 0;

				// If has textures set, bind it, if not, verify if the base material has set 
				// the textures, if so bind them, otherwise, bind the default shader texture
				auto textureSets = vk_Shader->GetTextureDescriptorSets();
				if (materialTextures.size() > 0)
				{
					for (auto it = materialTextures.begin(); it != materialTextures.end(); it++)
					{
						uint32_t index = vk_Shader->GetTextureDescriptorSetIndex(it->first);
						auto textureSet = vk_Shader->GetTexturePoolDescriptorSet(it->second);
						textureSets[index] = textureSet;
					}
				}
				if (instanceTextures.size() > 0)
				{
					for (auto it = instanceTextures.begin(); it != instanceTextures.end(); it++)
					{
						uint32_t index = vk_Shader->GetTextureDescriptorSetIndex(it->first);
						auto textureSet = vk_Shader->GetTexturePoolDescriptorSet(it->second);
						textureSets[index] = textureSet;
					}
				}

				std::vector<VkDescriptorSet> sets;
				sets.push_back(vk_Shader->GetGlobalDescriptorSet());
				sets.insert(sets.end(), textureSets.begin(), textureSets.end());

				bool hasVSMaterialBuffer = vk_Shader->HasVSMaterialUniformBuffer();
				bool hasPSMaterialBuffer = vk_Shader->HasPSMaterialUniformBuffer();
				bool hasMaterialBuffer   = hasVSMaterialBuffer || hasPSMaterialBuffer;
				uint32_t offset = materialInstance->GetUniformBufferIndex() * vk_Shader->GetMaterialUniformBufferAlignment();
				
				vkCmdBindDescriptorSets(
					drawCommandBuffer,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					vk_Shader->GetPipelineLayout(),
					0,
					sets.size(),
					sets.data(),
					hasMaterialBuffer ? 1 : 0,
					hasMaterialBuffer ? &offset : nullptr
				);
			}

			vkCmdBindPipeline(
				drawCommandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				vk_Pipeline->GetVulkanPipeline()
			);

			VkDeviceSize offsets[1] = { 0 };
			VulkanVertexBuffer* vk_VertexBuffer = static_cast<VulkanVertexBuffer*>(vertexBuffer.get());
			vkCmdBindVertexBuffers(drawCommandBuffer, 0, 1, &vk_VertexBuffer->GetDeviceBuffer(), offsets);

			VulkanIndexBuffer* vk_IndexBuffer = static_cast<VulkanIndexBuffer*>(indexBuffer.get());
			vkCmdBindIndexBuffer(drawCommandBuffer, vk_IndexBuffer->GetDeviceBuffer(), 0, VK_INDEX_TYPE_UINT32);

			// Draw
			vkCmdDrawIndexed(drawCommandBuffer, indexCount == 0 ? (indexBuffer->GetSize() / sizeof(uint32_t)) : indexCount, 1, 0, 0, 1);
		});
	}

	void VulkanRenderCommandBuffer::SubmitMesh(
		const Ref<Pipeline>& pipeline, 
		const Ref<Mesh>& mesh,
		const glm::mat4& transform,
		const Ref<MaterialInstance>& overrideMaterial
	)
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
		VulkanShader* vk_Shader = dynamic_cast<VulkanShader*>(mesh->GetMeshShader().get());

		m_Queue.push_back([=](const VkCommandBuffer& drawCommandBuffer, const VkFramebuffer& framebuffer)
			{
				// Set bone matrices
				if (mesh->IsAnimated())
				{
					uint32_t size = mesh->m_BoneTransforms.size() * sizeof(glm::mat4);
					auto transforms = mesh->m_BoneTransforms.data();
					vk_Shader->SetUniformBuffer("u_AnimData", transforms, size);
				}

				// TODO: Viewport and scissor state should come from pipeline (?) potentially
				VkViewport viewport = {};
				viewport.x = 0;
				viewport.y = (float)vk_Context->GetHeight();
				viewport.width = (float)vk_Context->GetWidth();
				viewport.height = -(float)vk_Context->GetHeight();
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;
				vkCmdSetViewport(drawCommandBuffer, 0, 1, &viewport);

				VkRect2D scissor = {};
				scissor.offset.x = 0;
				scissor.offset.y = 0;
				scissor.extent.width = vk_Context->GetWidth();
				scissor.extent.height = vk_Context->GetHeight();
				vkCmdSetScissor(drawCommandBuffer, 0, 1, &scissor);

				VulkanPipeline* vk_Pipeline = static_cast<VulkanPipeline*>(pipeline.get());

				vkCmdBindPipeline(
					drawCommandBuffer,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					vk_Pipeline->GetVulkanPipeline()
				);

				VkDeviceSize offsets[1] = { 0 };
				VulkanVertexBuffer* vk_VertexBuffer = static_cast<VulkanVertexBuffer*>(mesh->GetVertexBuffer().get());
				vkCmdBindVertexBuffers(drawCommandBuffer, 0, 1, &vk_VertexBuffer->GetDeviceBuffer(), offsets);

				VulkanIndexBuffer* vk_IndexBuffer = static_cast<VulkanIndexBuffer*>(mesh->GetIndexBuffer().get());
				vkCmdBindIndexBuffer(drawCommandBuffer, vk_IndexBuffer->GetDeviceBuffer(), 0, VK_INDEX_TYPE_UINT32);

				auto& materials = mesh->GetMaterials();
				for (Submesh& submesh : mesh->GetSubmeshes())
				{
					// Material
					auto material = materials[submesh.MaterialIndex];

					// If has a DescriptionSet, bind it
					if (vk_Shader->GetGlobalDescriptorSet())
					{
						auto textures = material->GetTextures();

						// If has textures set, bind it, if not, verify if the base material has set 
						// the textures, if so bind them, otherwise, bind the default shader texture
						auto textureSets = vk_Shader->GetTextureDescriptorSets();
						if (textures.size() > 0)
						{
							for (auto it = textures.begin(); it != textures.end(); it++)
							{
								uint32_t index = vk_Shader->GetTextureDescriptorSetIndex(it->first);
								auto textureSet = vk_Shader->GetTexturePoolDescriptorSet(it->second);
								textureSets[index] = textureSet;
							}
						}

						std::vector<VkDescriptorSet> sets;
						sets.push_back(vk_Shader->GetGlobalDescriptorSet());
						sets.insert(sets.end(), textureSets.begin(), textureSets.end());

						bool hasVSMaterialBuffer = vk_Shader->HasVSMaterialUniformBuffer();
						bool hasPSMaterialBuffer = vk_Shader->HasPSMaterialUniformBuffer();
						bool hasMaterialBuffer = hasVSMaterialBuffer || hasPSMaterialBuffer;
						uint32_t offset = material->GetUniformBufferIndex() * vk_Shader->GetMaterialUniformBufferAlignment();

						vkCmdBindDescriptorSets(
							drawCommandBuffer,
							VK_PIPELINE_BIND_POINT_GRAPHICS,
							vk_Shader->GetPipelineLayout(),
							0,
							sets.size(),
							sets.data(),
							hasMaterialBuffer ? 1 : 0,
							hasMaterialBuffer ? &offset : nullptr
						);
					}

					glm::mat4 trans = transform * submesh.Transform;
					vk_Shader->SetUniformBufferParam("u_SceneData", "Transform", &trans, sizeof(glm::mat4));

					// Draw
					vkCmdDrawIndexed(drawCommandBuffer, submesh.IndexCount, 1, submesh.BaseIndex * sizeof(uint32_t), submesh.BaseVertex, 1);
				}
			});
	}

	void VulkanRenderCommandBuffer::Flush()
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
		auto& device = vk_Context->GetDevice();

		VkCommandBufferBeginInfo cmdBufInfo = {};
		cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		auto drawCmdBuffer = vk_Context->GetCurrentDrawCmdBuffer();
		auto framebuffer = vk_Context->GetCurrentFramebuffer();

		VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffer, &cmdBufInfo));
		
		for (auto& func : m_Queue)
			func(drawCmdBuffer, framebuffer);
		
		// Ending the render pass will add an implicit barrier transitioning the frame buffer color attachment
		// to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR for presenting it to the windowing system
		VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffer));

		m_Queue.clear();
	}
	
}
