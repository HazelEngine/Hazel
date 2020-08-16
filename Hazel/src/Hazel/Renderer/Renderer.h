#pragma once

#include <Hazel/Renderer/Mesh.h>
#include <Hazel/Renderer/Shader.h>
#include <Hazel/Renderer/Material.h>
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

		static void Prepare();

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
			const Ref<Pipeline>& pipeline,
			const Ref<VertexBuffer>& vertexBuffer,
			const Ref<IndexBuffer>& indexBuffer,
			const Ref<Material>& material,
			uint32_t indexCount = 0
		);

		static void Submit(
			const Ref<Pipeline>& pipeline,
			const Ref<VertexBuffer>& vertexBuffer,
			const Ref<IndexBuffer>& indexBuffer,
			const Ref<MaterialInstance>& materialInstance,
			uint32_t indexCount = 0
		);
		
		static void Submit(
			const Ref<Shader>& shader,
			const Ref<VertexArray>& vertexArray,
			const glm::mat4& transform = glm::mat4(1.0f)
		);

		static void SubmitFullscreenQuad(
			const Ref<Pipeline>& pipeline,
			const Ref<MaterialInstance>& material,
			const glm::mat4& transform = glm::mat4(1.0f)
		);

		static void SubmitMesh(
			const Ref<Pipeline>& pipeline,
			const Ref<Mesh>& mesh,
			const glm::mat4& transform,
			const Ref<MaterialInstance>& overrideMaterial = nullptr
		);

		static void FlushCommandBuffer();

		static void OnWindowResize(uint32_t width, uint32_t height);

		static const Scope<ShaderLibrary>& GetShaderLibrary();

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