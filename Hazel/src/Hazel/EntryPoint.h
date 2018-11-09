#pragma once

#ifdef HZ_PLATFORM_WINDOWS

	extern Hazel::Application* CreateApplication();

	int main()
	{
		auto app = Hazel::CreateApplication();
		app->Run();
		delete app;
	}
#else
	#error Hazel only support Windows!
#endif