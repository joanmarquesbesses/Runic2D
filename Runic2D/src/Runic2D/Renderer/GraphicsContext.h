#pragma once

namespace Runic2D {

	class GraphicsContext
	{
	public:
		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;
	};

} // namespace Runic2D
