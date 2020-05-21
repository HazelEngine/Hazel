#pragma once

#include <Hazel.h>
using namespace Hazel;

class RendererTestLayer : public Layer
{
public:
	RendererTestLayer();
	virtual ~RendererTestLayer() = default;

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(Timestep ts) override;
	void OnImGuiRender() override;
	void OnEvent(Event& e) override;

private:
	Ref<Pipeline> m_Pipeline;
	Ref<VertexBuffer> m_VertexBuffer;
	Ref<IndexBuffer> m_IndexBuffer;
	Ref<Shader> m_Shader;
	Ref<RenderPass> m_RenderPass;

	Ref<Texture2D> m_CheckerboardTex;
	Ref<Texture2D> m_PikachuTex, m_EeveeTex;
	
	PerspectiveCameraController m_PerspCameraController;

	glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };
};