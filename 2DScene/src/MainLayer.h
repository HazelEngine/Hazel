#pragma once

#include <Hazel.h>

class MainLayer : public Hazel::Layer
{
public:
	MainLayer();
	virtual ~MainLayer() = default;

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(Hazel::Timestep ts) override;
	void OnImGuiRender() override;
	void OnEvent(Hazel::Event& e) override;

private:
	Hazel::Ref<Hazel::RenderPass> m_RenderPass;

	Hazel::Ref<Hazel::Texture2D> m_SpriteSheet;
	Hazel::Ref<Hazel::SubTexture2D> m_Ground;
	Hazel::Ref<Hazel::SubTexture2D> m_TopLeftGround;
	Hazel::Ref<Hazel::SubTexture2D> m_TopMidGround;
	Hazel::Ref<Hazel::SubTexture2D> m_TopRightGround;
	Hazel::Ref<Hazel::SubTexture2D> m_MidLeftGround;
	Hazel::Ref<Hazel::SubTexture2D> m_MidRightGround;
	Hazel::Ref<Hazel::SubTexture2D> m_BottomLeftGround;
	Hazel::Ref<Hazel::SubTexture2D> m_BottomMidGround;
	Hazel::Ref<Hazel::SubTexture2D> m_BottomRightGround;
	Hazel::Ref<Hazel::SubTexture2D> m_HouseWall1, m_HouseWall2;
	Hazel::Ref<Hazel::SubTexture2D> m_HouseRoof;
	Hazel::Ref<Hazel::SubTexture2D> m_HouseDoor;
	Hazel::Ref<Hazel::SubTexture2D> m_Tree;

	Hazel::OrthographicCameraController m_CameraController;
};