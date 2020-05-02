#include "RendererTestLayer.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

RendererTestLayer::RendererTestLayer()
	: Layer("RendererTestLayer"), m_CameraController(1280.0f / 720.0f, true) {}

void RendererTestLayer::OnAttach()
{
	m_CheckerboardTex = Texture2D::Create("assets/Textures/Checkerboard_SemiTransparent.png");
	m_PikachuTex = Texture2D::Create("assets/Textures/Pikachu.png");
	m_EeveeTex = Texture2D::Create("assets/Textures/Eevee.png");

	// 1. Create Render Pipeline
	// 2. Create Render Command Buffer

	m_Shader = Shader::Create(
		"Texture", 
		"assets/Shaders/Compiled/Texture.vert.spv", 
		"assets/Shaders/Compiled/Texture.frag.spv"
	);

	m_Shader->BindTexture("u_Texture", m_CheckerboardTex);

	// NOTE: Should set layout in pipeline, or should get pipeline from shader binary?
	PipelineSpecification pipelineSpec;
	pipelineSpec.Shader = m_Shader;
	pipelineSpec.VertexBufferLayout = {
		{ ShaderDataType::Float3, "a_Position" },
		{ ShaderDataType::Float2, "a_TexCoord" },
		{ ShaderDataType::Float4, "a_Color" }
	};

	m_Pipeline = Pipeline::Create(pipelineSpec);

	struct Vertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoord;
		glm::vec4 Color;
	};

	Vertex vertices[4] =
	{
		{{  1.0f,  1.0f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }},
		{{ -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }},
		{{ -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }},
		{{  1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }}
	};

	uint32_t indices[6] = { 0, 1, 2, 2, 3, 0 };

	m_VertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));
	m_IndexBuffer = IndexBuffer::Create(indices, 6);

	FramebufferSpecification framebufferSpec;
	framebufferSpec.ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };

	RenderPassSpecification renderPassSpec;
	renderPassSpec.TargetFramebuffer = Framebuffer::Create(framebufferSpec);
	m_RenderPass = RenderPass::Create(renderPassSpec);
}

void RendererTestLayer::OnDetach() {}

void RendererTestLayer::OnUpdate(Timestep ts)
{
	// Update
	m_CameraController.OnUpdate(ts);

	Renderer::BeginRenderPass(m_RenderPass);
	Renderer2D::BeginScene(m_CameraController.GetCamera());

	//for (float y = -1.0f; y < 1.0f; y += 0.1f)
	//{
	//	for (float x = -1.0f; x < 1.0f; x += 0.1f)
	//	{
	//		glm::vec4 color = { (x + 10.0f) / 20.0f, (y + 10.0f) / 20.0f, 0.5f, 1.0f };
	//		Renderer2D::DrawQuad({ x, y }, { 0.09f, 0.09f }, color);
	//	}
	//}

	Renderer2D::DrawQuad({ -1.5f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });
	Renderer2D::DrawQuad({ 1.0f, 0.0f }, { 1.0f, 1.0f }, m_PikachuTex);
	Renderer2D::DrawQuad({ 1.0f, -2.0f }, { 1.0f, 1.0f }, m_EeveeTex);

	Renderer2D::EndScene();
	
	glm::mat4 viewProj = m_CameraController.GetCamera().GetViewProjectionMatrix();
	m_Shader->SetUniformBuffer("u_SceneData", &viewProj, sizeof(glm::mat4));
	
	Renderer::Submit(m_Pipeline, m_VertexBuffer, m_IndexBuffer);
	Renderer::EndRenderPass();
	
	Renderer::FlushCommandBuffer();
}

void RendererTestLayer::OnImGuiRender()
{
	//ImGui::Begin("Settings");
	//ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
	//ImGui::End();
}

void RendererTestLayer::OnEvent(Event& e)
{
	m_CameraController.OnEvent(e);
}
