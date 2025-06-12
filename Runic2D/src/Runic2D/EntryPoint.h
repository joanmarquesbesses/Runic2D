#pragma once

#ifdef RUNIC2D_PLATFORM_WINDOWS

extern Runic2D::Application* Runic2D::CreateApplication();

int main(int argc, char** argv)
{
	Runic2D::Log::Init();
	RUNIC2D_CORE_TRACE("Runic2D Engine Initialized!");
	int a = 5;
	RUNIC2D_INFO("Application started with {0} arguments", a);
	// Initialize the application
	auto app = Runic2D::CreateApplication();
	app->Run();
	delete app;
	return 0;
}

#endif // RUNIC2D_PLATFORM_WINDOWS
