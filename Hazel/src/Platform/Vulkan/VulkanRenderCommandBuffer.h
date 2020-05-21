#pragma once

#include <Hazel/Renderer/RenderCommandBuffer.h>

#include <vulkan/vulkan.hpp>

namespace Hazel {

	class HAZEL_API VulkanRenderCommandBuffer : public RenderCommandBuffer
	{
	public:
		VulkanRenderCommandBuffer();
		virtual ~VulkanRenderCommandBuffer();

		virtual void BeginRenderPass(const Ref<RenderPass>& renderPass) override;
		virtual void EndRenderPass() override;

		virtual void Submit(
			const Ref<Pipeline>& pipeline,
			const Ref<VertexBuffer>& vertexBuffer,
			const Ref<IndexBuffer>& indexBuffer,
			uint32_t indexCount = 0
		) override;

		virtual void Submit(
			const Ref<Pipeline>& pipeline,
			const Ref<VertexBuffer>& vertexBuffer,
			const Ref<IndexBuffer>& indexBuffer,
			const Ref<Material>& material,
			uint32_t indexCount = 0
		) override;

		virtual void Submit(
			const Ref<Pipeline>& pipeline,
			const Ref<VertexBuffer>& vertexBuffer,
			const Ref<IndexBuffer>& indexBuffer,
			const Ref<MaterialInstance>& materialInstance,
			uint32_t indexCount = 0
		) override;

		virtual void SubmitMesh(
			const Ref<Pipeline>& pipeline,
			const Ref<Mesh>& mesh,
			const glm::mat4& transform,
			const Ref<MaterialInstance>& overrideMaterial
		) override;

		virtual void Flush() override;

	private:
		std::vector<std::function<void (const VkCommandBuffer&, const VkFramebuffer&)>> m_Queue;
	};
	
}
