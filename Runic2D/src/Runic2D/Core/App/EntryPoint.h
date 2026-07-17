#pragma once

#ifdef R2D_PLATFORM_WINDOWS

extern "C" {
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

extern Runic2D::Application* Runic2D::CreateApplication();

int main(int argc, char** argv)
{
	Runic2D::Log::Init();

	auto app = Runic2D::CreateApplication();

	app->Run();

	delete app;

	return 0;
}

#endif // RUNIC2D_PLATFORM_WINDOWS

