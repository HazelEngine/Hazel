#pragma once

#ifdef HZ_PLATFORM_WINDOWS

	extern Hazel::Application* CreateApplication();

	int main()
	{
		Hazel::Log::Init();
		HZ_CORE_WARN("Initialized log system!")

		auto app = Hazel::CreateApplication();
		app->Run();
		delete app;
	}
#else
	#error Hazel only support Windows!
#endif