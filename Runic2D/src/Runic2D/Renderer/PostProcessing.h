#pragma once

#include "Runic2D/Renderer/FrameBuffer.h"
#include "Runic2D/Renderer/Shader.h"
#include "Runic2D/Renderer/VertexArray.h"
#include "Runic2D/Renderer/Texture.h"

namespace Runic2D {

    class RUNIC_API PostProcessing
    {
    public:
        static void Init();
        static void Shutdown();

        static void OnWindowResize(uint32_t width, uint32_t height);

        static Ref<Texture2D> ApplyBlur(const Ref<Texture2D>& texture, int iterations = 10);

        static void Render(const Ref<Texture2D>& baseTexture);

    private:
        static Ref<FrameBuffer> s_PingPongFBO[2];

        static Ref<Shader> s_PostProcessShader;
        static Ref<Shader> s_BlurShader;

        static Ref<VertexArray> s_FullscreenQuadVAO;
    };

}