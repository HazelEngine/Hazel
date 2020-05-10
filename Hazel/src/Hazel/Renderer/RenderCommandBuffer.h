#pragma once

#include <Hazel.h>

namespace Hazel {

	class HAZEL_API RenderCommandBuffer
	{
	public:
		virtual ~RenderCommandBuffer() = default;

		virtual void BeginRenderPass(const Ref<RenderPass>& renderPass) = 0;
		virtual void EndRenderPass() = 0;

		virtual void Submit(
			const Ref<Pipeline>& pipeline,
			const Ref<VertexBuffer>& vertexBuffer,
			const Ref<IndexBuffer>& indexBuffer,
			uint32_t indexCount = 0
		) = 0;

		virtual void Submit(
			const Ref<Pipeline>& pipeline,
			const Ref<VertexBuffer>& vertexBuffer,
			const Ref<IndexBuffer>& indexBuffer,
			const Ref<Material>& material,
			uint32_t indexCount = 0
		) = 0;

		virtual void Submit(
			const Ref<Pipeline>& pipeline,
			const Ref<VertexBuffer>& vertexBuffer,
			const Ref<IndexBuffer>& indexBuffer,
			const Ref<MaterialInstance>& materialInstance,
			uint32_t indexCount = 0
		) = 0;

		virtual void Flush() = 0;
		
		static Ref<RenderCommandBuffer> Create();
	};
	
}
