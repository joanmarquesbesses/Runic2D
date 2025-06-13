#include "R2Dpch.h"
#include "Application.h"

#include "Runic2D/Events/ApplicationEvent.h"
#include "Runic2D/Log.h"

namespace Runic2D {
	Application::Application()
	{
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		WindowResizeEvent e(1280, 720);
		if (e.IsInCategory(EventCategoryApplication)) {
			RUNIC2D_TRACE(e);
		}
		if (e.IsInCategory(EventCategoryInput)) {
			RUNIC2D_TRACE(e);
		}

		while (true);
	}
}
