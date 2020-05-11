#include <Hazel.h>
#include <Hazel/Core/EntryPoint.h>

#include "MainLayer.h"

class SpriteSheetScene : public Hazel::Application
{
public:
	SpriteSheetScene()
	{
		PushLayer(new MainLayer());
	}

	~SpriteSheetScene() {}
};

Hazel::Application* Hazel::CreateApplication()
{
	Hazel::RendererAPI::SetAPI(Hazel::RendererAPI::API::Vulkan);
	return new SpriteSheetScene();
}