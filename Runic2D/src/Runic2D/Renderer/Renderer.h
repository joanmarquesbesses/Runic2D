#pragma once

#include "RenderCommand.h"

#include "OrthographicCamera.h"

#include "Shader.h"

namespace Runic2D {

	class Renderer
	{
	public:
		static void Init();

		static void BeginScene(OrthographicCamera& Camera);
		static void EndScene();

		static void Submit(const Ref<Shader>& shader, const Ref<class VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f));

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	private:
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		static SceneData* s_SceneData;
	};

} // namespace Runic2D