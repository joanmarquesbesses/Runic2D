#pragma once

#include "Core.h"

namespace Runic2D
{
	class RUNIC2D_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	//  To be defined in the client
	Application* CreateApplication();

}