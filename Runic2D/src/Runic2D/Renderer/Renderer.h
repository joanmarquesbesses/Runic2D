#pragma once

namespace Runic2D {

	enum class RendererAPI
	{
		None = 0,
		OpenGL = 1,
		Vulkan = 2,
		DirectX = 3
	};

	class Renderer
	{
	public:
		static RendererAPI GetAPI() { return s_RendererAPI; }
		static void SetAPI(RendererAPI api) { s_RendererAPI = api; }
	private:
		static RendererAPI s_RendererAPI;
	};

} // namespace Runic2D