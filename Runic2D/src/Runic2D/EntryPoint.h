#pragma once

#ifdef RUNIC2D_PLATFORM_WINDOWS

extern Runic2D::Application* Runic2D::CreateApplication();

int main(int argc, char** argv)
{

	auto app = Runic2D::CreateApplication();
	app->Run();
	delete app;
	return 0;
}

#endif // RUNIC2D_PLATFORM_WINDOWS
