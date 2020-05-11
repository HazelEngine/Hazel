#include "MainLayer.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

using namespace Hazel;

MainLayer::MainLayer()
	: Layer("MainLayer"), m_CameraController(1280.0f / 720.0f, true) {}

void MainLayer::OnAttach()
{
	// Load sprite sheet texture
	m_SpriteSheet = Texture2D::Create("assets/Textures/RPGpack_sheet_2X.png");

	// Create the subtextures
	m_Ground = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 1, 11 }, { 128, 128 });
	m_TopLeftGround = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0, 12 }, { 128, 128 });
	m_TopMidGround = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 1, 12 }, { 128, 128 });
	m_TopRightGround = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 2, 12 }, { 128, 128 });
	m_MidLeftGround = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0, 11 }, { 128, 128 });
	m_MidRightGround = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 2, 11 }, { 128, 128 });
	m_BottomLeftGround = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0, 10 }, { 128, 128 });
	m_BottomMidGround = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 1, 10 }, { 128, 128 });
	m_BottomRightGround = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 2, 10 }, { 128, 128 });
	
	m_HouseWall1 = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0, 7 }, { 128, 128 });
	m_HouseWall2 = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 2, 7 }, { 128, 128 });
	m_HouseRoof = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 2, 4 }, { 128, 128 }, { 2, 3 });
	m_HouseDoor = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 14, 2 }, { 128, 128 });
	
	m_Tree = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0, 1 }, { 128, 128 }, { 1, 2 });

	FramebufferSpecification framebufferSpec;
	framebufferSpec.ClearColor = { 0.54f, 0.9f, 1.0f, 1.0f };

	RenderPassSpecification renderPassSpec;
	renderPassSpec.TargetFramebuffer = Framebuffer::Create(framebufferSpec);
	m_RenderPass = RenderPass::Create(renderPassSpec);
}

void MainLayer::OnDetach() {}

void MainLayer::OnUpdate(Timestep ts)
{
	// Update
	m_CameraController.OnUpdate(ts);

	// Reset statistics
	Renderer2D::ResetStatistics();

	// Get a new Swapchain image
	Renderer::Prepare();

	Renderer::BeginRenderPass(m_RenderPass);
	Renderer2D::BeginScene(m_CameraController.GetCamera());

	// Render the ground
	Renderer2D::DrawQuad({ -3.0f,  2.0f }, { 1.0f, 1.0f }, m_TopLeftGround);
	Renderer2D::DrawQuad({ -2.0f,  2.0f }, { 1.0f, 1.0f }, m_TopMidGround);
	Renderer2D::DrawQuad({ -1.0f,  2.0f }, { 1.0f, 1.0f }, m_TopMidGround);
	Renderer2D::DrawQuad({  0.0f,  2.0f }, { 1.0f, 1.0f }, m_TopMidGround);
	Renderer2D::DrawQuad({  1.0f,  2.0f }, { 1.0f, 1.0f }, m_TopMidGround);
	Renderer2D::DrawQuad({  2.0f,  2.0f }, { 1.0f, 1.0f }, m_TopMidGround);
	Renderer2D::DrawQuad({  3.0f,  2.0f }, { 1.0f, 1.0f }, m_TopRightGround);

	Renderer2D::DrawQuad({ -3.0f,  1.0f }, { 1.0f, 1.0f }, m_MidLeftGround);
	Renderer2D::DrawQuad({ -2.0f,  1.0f }, { 1.0f, 1.0f }, m_Ground);
	Renderer2D::DrawQuad({ -1.0f,  1.0f }, { 1.0f, 1.0f }, m_Ground);
	Renderer2D::DrawQuad({  0.0f,  1.0f }, { 1.0f, 1.0f }, m_Ground);
	Renderer2D::DrawQuad({  1.0f,  1.0f }, { 1.0f, 1.0f }, m_Ground);
	Renderer2D::DrawQuad({  2.0f,  1.0f }, { 1.0f, 1.0f }, m_Ground);
	Renderer2D::DrawQuad({  3.0f,  1.0f }, { 1.0f, 1.0f }, m_MidRightGround);

	Renderer2D::DrawQuad({ -3.0f,  0.0f }, { 1.0f, 1.0f }, m_MidLeftGround);
	Renderer2D::DrawQuad({ -2.0f,  0.0f }, { 1.0f, 1.0f }, m_Ground);
	Renderer2D::DrawQuad({ -1.0f,  0.0f }, { 1.0f, 1.0f }, m_Ground);
	Renderer2D::DrawQuad({  0.0f,  0.0f }, { 1.0f, 1.0f }, m_Ground);
	Renderer2D::DrawQuad({  1.0f,  0.0f }, { 1.0f, 1.0f }, m_Ground);
	Renderer2D::DrawQuad({  2.0f,  0.0f }, { 1.0f, 1.0f }, m_Ground);
	Renderer2D::DrawQuad({  3.0f,  0.0f }, { 1.0f, 1.0f }, m_MidRightGround);

	Renderer2D::DrawQuad({ -3.0f, -1.0f }, { 1.0f, 1.0f }, m_MidLeftGround);
	Renderer2D::DrawQuad({ -2.0f, -1.0f }, { 1.0f, 1.0f }, m_Ground);
	Renderer2D::DrawQuad({ -1.0f, -1.0f }, { 1.0f, 1.0f }, m_Ground);
	Renderer2D::DrawQuad({  0.0f, -1.0f }, { 1.0f, 1.0f }, m_Ground);
	Renderer2D::DrawQuad({  1.0f, -1.0f }, { 1.0f, 1.0f }, m_Ground);
	Renderer2D::DrawQuad({  2.0f, -1.0f }, { 1.0f, 1.0f }, m_Ground);
	Renderer2D::DrawQuad({  3.0f, -1.0f }, { 1.0f, 1.0f }, m_MidRightGround);

	Renderer2D::DrawQuad({ -3.0f, -2.0f }, { 1.0f, 1.0f }, m_BottomLeftGround);
	Renderer2D::DrawQuad({ -2.0f, -2.0f }, { 1.0f, 1.0f }, m_BottomMidGround);
	Renderer2D::DrawQuad({ -1.0f, -2.0f }, { 1.0f, 1.0f }, m_BottomMidGround);
	Renderer2D::DrawQuad({  0.0f, -2.0f }, { 1.0f, 1.0f }, m_BottomMidGround);
	Renderer2D::DrawQuad({  1.0f, -2.0f }, { 1.0f, 1.0f }, m_BottomMidGround);
	Renderer2D::DrawQuad({  2.0f, -2.0f }, { 1.0f, 1.0f }, m_BottomMidGround);
	Renderer2D::DrawQuad({  3.0f, -2.0f }, { 1.0f, 1.0f }, m_BottomRightGround);
	
	// Render the house
	Renderer2D::DrawQuad({  1.0f, -1.0f }, { 1.0f, 1.0f }, m_HouseWall1);
	Renderer2D::DrawQuad({  2.0f, -1.0f }, { 1.0f, 1.0f }, m_HouseWall2);
	Renderer2D::DrawQuad({  1.0f,  0.0f }, { 1.0f, 1.0f }, m_HouseWall1);
	Renderer2D::DrawQuad({  2.0f,  0.0f }, { 1.0f, 1.0f }, m_HouseWall2);
	Renderer2D::DrawQuad({  1.5f, -1.0f }, { 1.0f, 1.0f }, m_HouseDoor);
	Renderer2D::DrawQuad({  1.5f,  1.0f }, { 2.0f, 3.0f }, m_HouseRoof);

	// Render the tree
	Renderer2D::DrawQuad({ -2.0f,  0.0f }, { 1.0f, 2.0f }, m_Tree);

	Renderer2D::EndScene();

	Renderer::EndRenderPass();
	Renderer::FlushCommandBuffer();
}

void MainLayer::OnImGuiRender() {}

void MainLayer::OnEvent(Event& e)
{
	m_CameraController.OnEvent(e);
}
