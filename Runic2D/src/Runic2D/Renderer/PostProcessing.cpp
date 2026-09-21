#include "R2Dpch.h"
#include "PostProcessing.h"

#include "Runic2D/Renderer/RenderCommand.h"
#include "Runic2D/Renderer/Buffer.h"
#include "Runic2D/Assets/ResourceManager.h"

namespace Runic2D {

    Ref<FrameBuffer> PostProcessing::s_PingPongFBO[2];
    Ref<Shader> PostProcessing::s_PostProcessShader;
    Ref<Shader> PostProcessing::s_BlurShader;
    Ref<VertexArray> PostProcessing::s_FullscreenQuadVAO;

    static uint32_t s_EmptyVAO = 0;

    void PostProcessing::Init()
    {
        s_PostProcessShader = ResourceManager::Get<Shader>("Resources/Shaders/PostProcess.glsl");
        s_BlurShader = ResourceManager::Get<Shader>("Resources/Shaders/Blur.glsl");        
        // 1. FBOs HDR
        FrameBufferSpecification fbSpec;
        fbSpec.Attachments = { FramebufferTextureFormat::RGBA16F, FramebufferTextureFormat::Depth };
        fbSpec.Width = 1280;
        fbSpec.Height = 720;

        s_PingPongFBO[0] = FrameBuffer::Create(fbSpec);
        s_PingPongFBO[1] = FrameBuffer::Create(fbSpec);
        s_FullscreenQuadVAO = VertexArray::Create();

        float quadVertices[] = {
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f,
            -1.0f,  1.0f,  0.0f, 1.0f
        };

        Ref<VertexBuffer> quadVBO = VertexBuffer::Create(quadVertices, sizeof(quadVertices));
        quadVBO->SetLayout({
            { ShaderDataType::Float2, "a_Position" },
            { ShaderDataType::Float2, "a_TexCoord" }
            });

        s_FullscreenQuadVAO->AddVertexBuffer(quadVBO);

        uint32_t quadIndices[] = { 0, 1, 2, 2, 3, 0 };

        Ref<IndexBuffer> quadIBO = IndexBuffer::Create(quadIndices, sizeof(quadIndices) / sizeof(uint32_t));
        s_FullscreenQuadVAO->SetIndexBuffer(quadIBO);
    }

    void PostProcessing::Shutdown()
    {
        s_FullscreenQuadVAO = nullptr;
        s_PingPongFBO[0] = nullptr;
        s_PingPongFBO[1] = nullptr;
        s_PostProcessShader = nullptr;
    }

    void PostProcessing::OnWindowResize(uint32_t width, uint32_t height)
    {
        s_PingPongFBO[0]->Resize(width, height);
        s_PingPongFBO[1]->Resize(width, height);
    }

    void PostProcessing::Render(const Ref<Texture2D>& baseTexture)
    {
        // 0. GUARDEM ON ESTÀVEM DIBUIXANT (L'Editor)
        int originalFBO = RenderCommand::GetBoundFramebuffer();

        // PAS 1: BLOOM PING-PONG
        bool horizontal = true, first_iteration = true;
        int amount = 10;
        s_BlurShader->Bind();
        s_BlurShader->SetInt("u_Image", 0);
        for (int i = 0; i < amount; i++)
        {
            s_PingPongFBO[horizontal]->Bind();
            s_BlurShader->SetInt("u_Horizontal", horizontal);
            if (first_iteration) {
                baseTexture->Bind(0);
            }
            else {
                auto& fboSpec = s_PingPongFBO[!horizontal]->GetSpecification();
                Ref<Texture2D> pingPongTex = Texture2D::Create(
                    s_PingPongFBO[!horizontal]->GetColorAttachmentRendererID(),
                    fboSpec.Width, fboSpec.Height);
                pingPongTex->Bind(0);
            }
            RenderCommand::DrawIndexed(s_FullscreenQuadVAO);
            horizontal = !horizontal;
            first_iteration = false;
        }
        // 1. TORNEM A L'EDITOR (Sense fer cap Unbind!)
        RenderCommand::BindFramebuffer(originalFBO);

        RenderCommand::SetViewport(0, 0, baseTexture->GetWidth(), baseTexture->GetHeight());

        // PAS 2: COMPOSICIÓ FINAL
        s_PostProcessShader->Bind();
        s_PostProcessShader->SetInt("u_SceneTexture", 0);
        s_PostProcessShader->SetInt("u_BloomTexture", 1);
        baseTexture->Bind(0);

        auto& fboSpecFinal = s_PingPongFBO[!horizontal]->GetSpecification();
        Ref<Texture2D> bloomTex = Texture2D::Create(
            s_PingPongFBO[!horizontal]->GetColorAttachmentRendererID(),
            fboSpecFinal.Width, fboSpecFinal.Height);
        bloomTex->Bind(1);

        RenderCommand::DrawIndexed(s_FullscreenQuadVAO);
    }
}