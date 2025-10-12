#pragma once

#ifdef RUNIC2D_PLATFORM_WINDOWS

extern Runic2D::Application* Runic2D::CreateApplication();

int main(int argc, char** argv)
{
	Runic2D::Log::Init();

	R2D_PROFILE_BEGIN_SESSION("Startup", "Runic2DProfile-Startup.json");
	auto app = Runic2D::CreateApplication();
	R2D_PROFILE_END_SESSION();

	R2D_PROFILE_BEGIN_SESSION("Runtime", "Runic2DProfile-Runtime.json");
	app->Run();
	R2D_PROFILE_END_SESSION();

	R2D_PROFILE_BEGIN_SESSION("Shutdown", "Runic2DProfile-Shutdown.json");
	delete app;
	R2D_PROFILE_END_SESSION();

	return 0;
}

#endif // RUNIC2D_PLATFORM_WINDOWS
