#include <Hazel.h>

#include "imgui.h"

class ExampleLayer : public Hazel::Layer
{
public:
	ExampleLayer() : Layer("Example")
	{
	}

	void OnUpdate() override
	{
		if (Hazel::Input::IsKeyPressed(HZ_KEY_TAB))
		{
			HZ_TRACE("Tab key is pressed!")
		}
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Test");
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Hello World");
		ImGui::End();
	}

	void OnEvent(Hazel::Event& event) override
	{
		//HZ_TRACE("{0}", event)
	}
};

class Sandbox : public Hazel::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}

	~Sandbox()
	{
	}
};

Hazel::Application* Hazel::CreateApplication()
{
	return new Sandbox();
}