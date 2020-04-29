#pragma once

#include <Hazel/Renderer/Shader.h>
#include <Hazel/Renderer/Pipeline.h>
#include <Hazel/Renderer/RenderPass.h>
#include <Hazel/Renderer/RenderCommand.h>
#include <Hazel/Renderer/GraphicsContext.h>
#include <Hazel/Camera/OrthographicCamera.h>

namespace Hazel {

	class HAZEL_API Renderer
	{
	public:
		static void Init(const Scope<GraphicsContext>& context);

		static void BeginScene(OrthographicCamera& camera);
		static void EndScene();

		static void BeginRenderPass(const Ref<RenderPass>& renderPass);
		static void EndRenderPass();

		static void Submit(
			const Ref<Pipeline>& pipeline,
			const Ref<VertexBuffer>& vertexBuffer,
			const Ref<IndexBuffer>& indexBuffer,
			uint32_t indexCount = 0
		);
		
		static void Submit(
			const Ref<Shader>& shader,
			const Ref<VertexArray>& vertexArray,
			const glm::mat4& transform = glm::mat4(1.0f)
		);

		static void FlushCommandBuffer();

		static void OnWindowResize(uint32_t width, uint32_t height);

		inline static GraphicsContext* GetContext() { return s_Context; }
		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

	private:
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		static Scope<SceneData> s_SceneData;

		static GraphicsContext* s_Context;
	};

}