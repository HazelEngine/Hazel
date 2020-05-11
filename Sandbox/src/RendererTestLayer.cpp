#include "RendererTestLayer.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

Ref<Material> g_Material;
Ref<MaterialInstance> g_Instance1;
Ref<MaterialInstance> g_Instance2;

Ref<VertexBuffer> g_QuadVertBuff;
Ref<IndexBuffer>  g_QuadIndxBuff;
Ref<VertexBuffer> g_TriangleVertBuff;
Ref<IndexBuffer>  g_TriangleIndxBuff;

Ref<Texture2D>  g_CirclesTex;

RendererTestLayer::RendererTestLayer()
	: Layer("RendererTestLayer"),
	  m_OrthoCameraController(1280.0f / 720.0f, true),
	  m_PerspCameraController(45.0f, 1280.0f, 720.0f, 0.1f, 10000.0f) {}

void RendererTestLayer::OnAttach()
{
	m_CheckerboardTex = Texture2D::Create("assets/Textures/Checkerboard_SemiTransparent.png");
	g_CirclesTex = Texture2D::Create("assets/Textures/Checkerboard_Color.png");
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

	g_Material = Material::Create(m_Shader);
	glm::vec4 basecolor = { 0.0f, 1.0f, 0.0f, 1.0f };
	g_Material->Set("Albedo", basecolor);
	g_Material->Set("u_Texture", g_CirclesTex);
	g_Material->Bind();

	g_Instance1 = MaterialInstance::Create(g_Material);
	glm::vec4 color1 = { 1.0f, 0.0f, 0.0f, 1.0f };
	g_Instance1->Set("Albedo", color1);
	g_Instance1->Bind();

	g_Instance2 = MaterialInstance::Create(g_Material);
	glm::vec4 color2 = { 0.0f, 0.0f, 1.0f, 1.0f };
	g_Instance2->Set("Albedo", color2);
	g_Instance2->Bind();

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

	//////////////////////////////////////////////////////////////////////////////////////

	Vertex quadVert[4] =
	{
		{{ -2.5f,  1.0f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }},
		{{ -3.5f,  1.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }},
		{{ -3.5f, -1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }},
		{{ -2.5f, -1.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }}
	};

	g_QuadVertBuff = VertexBuffer::Create(quadVert, sizeof(quadVert));
	g_QuadIndxBuff = IndexBuffer::Create(indices, 6);

	Vertex triangleVert[3] =
	{
		{{ 2.5f, -1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }},
		{{ 4.5f, -1.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }},
		{{ 3.5f,  1.0f, 0.0f }, { 0.5f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }}
	};

	uint32_t triangleIndx[3] = { 0, 1, 2 };

	g_TriangleVertBuff = VertexBuffer::Create(triangleVert, sizeof(triangleVert));
	g_TriangleIndxBuff = IndexBuffer::Create(triangleIndx, 3);
}

void RendererTestLayer::OnDetach() {}

void RendererTestLayer::OnUpdate(Timestep ts)
{
	// Update
	m_OrthoCameraController.OnUpdate(ts);
	m_PerspCameraController.OnUpdate(ts);

	// Reset statistics
	Renderer2D::ResetStatistics();

	// Get a new Swapchain image
	Renderer::Prepare();

	Renderer::BeginRenderPass(m_RenderPass);
	Renderer2D::BeginScene(m_PerspCameraController.GetCamera());

	//for (float y = -1.0f; y < 1.0f; y += 0.1f)
	//{
	//	for (float x = -1.0f; x < 1.0f; x += 0.1f)
	//	{
	//		glm::vec4 color = { (x + 10.0f) / 20.0f, 0.3f, (y + 10.0f) / 20.0f, 1.0f };
	//		Renderer2D::DrawQuad({ x, y }, { 0.09f, 0.09f }, color);
	//	}
	//}

	Renderer2D::DrawQuad({ -1.5f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });
	Renderer2D::DrawQuad({ 1.0f, 0.0f }, { 1.0f, 1.0f }, m_PikachuTex);
	Renderer2D::DrawQuad({ 1.0f, -2.0f }, { 1.0f, 1.0f }, m_EeveeTex);
	
	static float rotation = 0.0f;
	rotation += 0.9f * ts;
	Renderer2D::DrawRotatedQuad({ 1.0f, 2.0f }, { 1.0f, 1.0f }, rotation, m_EeveeTex);
	Renderer2D::DrawRotatedQuad({ 2.0f, 0.0f }, { 1.0f, 2.0f }, rotation, { 0.0f, 0.0f, 1.0f, 1.0f });

	Renderer2D::EndScene();
	
	glm::mat4 viewProj = m_PerspCameraController.GetCamera().GetViewProjectionMatrix();
	m_Shader->SetUniformBuffer("u_SceneData", &viewProj, sizeof(glm::mat4));

	Renderer::Submit(m_Pipeline, m_VertexBuffer, m_IndexBuffer, g_Material);
	Renderer::Submit(m_Pipeline, g_TriangleVertBuff, g_TriangleIndxBuff, g_Instance1);
	Renderer::Submit(m_Pipeline, g_QuadVertBuff, g_QuadIndxBuff, g_Instance2);

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
	m_OrthoCameraController.OnEvent(e);
	m_PerspCameraController.OnEvent(e);
}
