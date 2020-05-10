#pragma once

#include <Hazel/Renderer/RenderCommandBuffer.h>

namespace Hazel {

	class HAZEL_API OpenGLRenderCommandBuffer : public RenderCommandBuffer
	{
	public:
		OpenGLRenderCommandBuffer();
		virtual ~OpenGLRenderCommandBuffer();

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

		virtual void Flush() override;

	private:
		std::vector<std::function<void ()>> m_Queue;
	};
	
}
