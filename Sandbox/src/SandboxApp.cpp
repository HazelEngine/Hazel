#include <Hazel.h>
#include <Hazel/Core/EntryPoint.h>

#include "RendererTestLayer.h"

class Sandbox : public Hazel::Application
{
public:
	Sandbox()
	{
		PushLayer(new RendererTestLayer());
	}

	~Sandbox() {}
};

Hazel::Application* Hazel::CreateApplication()
{
	Hazel::RendererAPI::SetAPI(Hazel::RendererAPI::API::Vulkan);
	return new Sandbox();
}