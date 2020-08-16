#include "RendererTestLayer.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

struct Light
{
	glm::vec3 Direction;
	float Padding;
	glm::vec3 Radiance;
	float Multiplier = 1.0f;
};

Ref<Pipeline> g_MeshPipeline, g_AnimMeshPipeline;
Ref<Mesh> g_Mesh, g_AnimMesh;
Light g_Light;

Ref<TextureCube> m_DebugCube;
Ref<MaterialInstance> m_DebugCubeMat;
Ref<Pipeline> m_CubemapPipeline;

RendererTestLayer::RendererTestLayer()
	: Layer("RendererTestLayer"),
	  m_PerspCameraController(45.0f, 1280.0f, 720.0f, -10000.0f, 10000.0f) {}

void RendererTestLayer::OnAttach()
{
	m_CheckerboardTex = Texture2D::Create("assets/Textures/Checkerboard_SemiTransparent.png");
	m_PikachuTex = Texture2D::Create("assets/Textures/Pikachu.png");
	m_EeveeTex = Texture2D::Create("assets/Textures/Eevee.png");

	auto cubemapShader = Shader::Create(
		"Cubemap",
		"assets/Shaders/Compiled/Cubemap.vert.spv",
		"assets/Shaders/Compiled/Cubemap.frag.spv"
	);
	m_DebugCube = TextureCube::Create("assets/Environments/Arches_E_PineTree_Radiance.tga");
	m_DebugCubeMat = MaterialInstance::Create(Material::Create(cubemapShader));
	m_DebugCubeMat->Set("u_Cubemap", m_DebugCube);
	m_DebugCubeMat->Bind();

	PipelineSpecification cubemapPSpec;
	cubemapPSpec.Shader = cubemapShader;
	cubemapPSpec.VertexBufferLayout = {
		{ ShaderDataType::Float3, "a_Position" },
		{ ShaderDataType::Float2, "a_TexCoord" }
	};

	m_CubemapPipeline = Pipeline::Create(cubemapPSpec);



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

	/////////////////////////////////////////////////////////////////

	//g_Mesh = CreateRef<Mesh>("assets/Models/Cerberus/Cerberus.gltf");
	//g_Mesh = CreateRef<Mesh>("assets/Models/StingrayPBS1/StingrayPBS1.gltf");
	//g_Mesh = CreateRef<Mesh>("assets/Models/SciFi_Wall/SciFi_Wall.gltf");
	g_Mesh = CreateRef<Mesh>("assets/Models/WoodBarrel/WoodBarrel.gltf");
	g_AnimMesh = CreateRef<Mesh>("assets/Models/M1911/M1911.gltf");

	PipelineSpecification meshPipelineSpec;
	meshPipelineSpec.Shader = g_Mesh->GetMeshShader();
	meshPipelineSpec.VertexBufferLayout = g_Mesh->GetVertexBuffer()->GetLayout();

	g_MeshPipeline = Pipeline::Create(meshPipelineSpec);

	PipelineSpecification animMeshPipelineSpec;
	animMeshPipelineSpec.Shader = g_AnimMesh->GetMeshShader();
	animMeshPipelineSpec.VertexBufferLayout = g_AnimMesh->GetVertexBuffer()->GetLayout();

	g_AnimMeshPipeline = Pipeline::Create(animMeshPipelineSpec);

	g_Light.Direction = { -0.5f, -0.5f, 1.0f };
	g_Light.Radiance = { 1.0f, 1.0f, 1.0f };

	///////////////////////////////////////////////////////////////////

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
	m_IndexBuffer = IndexBuffer::Create(indices, _countof(indices) * sizeof(uint32_t));

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
	m_PerspCameraController.OnUpdate(ts);
	g_AnimMesh->OnUpdate(ts);
	g_Mesh->OnUpdate(ts);

	// Reset statistics
	Renderer2D::ResetStatistics();

	// Get a new Swapchain image
	Renderer::Prepare();

	Renderer::BeginRenderPass(m_RenderPass);
	//Renderer2D::BeginScene(m_PerspCameraController.GetCamera());
	
	//for (float y = -1.0f; y < 1.0f; y += 0.1f)
	//{
	//	for (float x = -1.0f; x < 1.0f; x += 0.1f)
	//	{
	//		glm::vec4 color = { (x + 10.0f) / 20.0f, 0.3f, (y + 10.0f) / 20.0f, 1.0f };
	//		Renderer2D::DrawQuad({ x, y }, { 0.09f, 0.09f }, color);
	//	}
	//}
	
	//Renderer2D::DrawQuad({ -1.5f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });
	//Renderer2D::DrawQuad({ 1.0f, 0.0f }, { 1.0f, 1.0f }, m_PikachuTex);
	//Renderer2D::DrawQuad({ 1.0f, -2.0f }, { 1.0f, 1.0f }, m_EeveeTex);
	//
	//static float rotation = 0.0f;
	//rotation += 0.9f * ts;
	//Renderer2D::DrawRotatedQuad({ 1.0f, 2.0f }, { 1.0f, 1.0f }, rotation, m_EeveeTex);
	//Renderer2D::DrawRotatedQuad({ 2.0f, 0.0f }, { 1.0f, 2.0f }, rotation, { 0.0f, 0.0f, 1.0f, 1.0f });
	//
	//Renderer2D::EndScene();
	
	glm::mat4 viewProj = m_PerspCameraController.GetCamera().GetViewProjectionMatrix();
	glm::vec3 camPos = m_PerspCameraController.GetCamera().GetPosition();
	m_Shader->SetUniformBuffer("u_SceneData", &viewProj, sizeof(glm::mat4));

	glm::mat4 invViewProj = glm::inverse(viewProj);
	m_CubemapPipeline->GetSpecification().Shader->SetUniformBuffer("u_SceneData", &invViewProj, sizeof(glm::mat4));
	Renderer::SubmitFullscreenQuad(m_CubemapPipeline, m_DebugCubeMat);

	glm::mat4 transform = glm::mat4(1.0f);
	transform = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f));
	g_Mesh->GetMeshShader()->SetUniformBufferParam("u_RenderData", "CameraPosition", &camPos, sizeof(glm::vec3));
	g_Mesh->GetMeshShader()->SetUniformBufferParam("u_RenderData", "Lights", &g_Light, sizeof(Light));
	g_Mesh->GetMeshShader()->SetUniformBufferParam("u_SceneData", "ViewProj", &viewProj, sizeof(glm::mat4));
	Renderer::SubmitMesh(g_MeshPipeline, g_Mesh, transform);
	
	transform = glm::scale(glm::mat4(1.0f), glm::vec3(35.0f));
	g_AnimMesh->GetMeshShader()->SetUniformBufferParam("u_RenderData", "CameraPosition", &camPos, sizeof(glm::vec3));
	g_AnimMesh->GetMeshShader()->SetUniformBufferParam("u_RenderData", "Lights", &g_Light, sizeof(Light));
	g_AnimMesh->GetMeshShader()->SetUniformBufferParam("u_SceneData", "ViewProj", &viewProj, sizeof(glm::mat4));
	Renderer::SubmitMesh(g_AnimMeshPipeline, g_AnimMesh, transform);

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
	m_PerspCameraController.OnEvent(e);
}
