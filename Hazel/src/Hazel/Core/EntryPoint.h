#pragma once

#ifdef HZ_PLATFORM_WINDOWS

	extern Hazel::Application* CreateApplication();

	int main()
	{
		Hazel::Log::Init();
		HZ_CORE_WARN("Initialized log system!")

		HZ_PROFILE_BEGIN_SESSION("Startup", "HZProfile_Startup.json")
		auto app = Hazel::CreateApplication();
		HZ_PROFILE_END_SESSION()

		HZ_PROFILE_BEGIN_SESSION("Runtime", "HZProfile_Runtime.json")
		app->Run();
		HZ_PROFILE_END_SESSION()

		HZ_PROFILE_BEGIN_SESSION("Shutdown", "HZProfile_Shutdown.json")
		delete app;
		HZ_PROFILE_END_SESSION()
	}
#else
	#error Hazel only support Windows!
#endif