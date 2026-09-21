#include "R2Dpch.h"
#include "Render2DSystem.h"

#include "Runic2D/Scene/Entity.h"
#include "Runic2D/Scene/Scene.h"
#include "Runic2D/Scene/Components/CoreComponents.h"
#include "Runic2D/Scene/Components/RenderComponents.h"
#include "Runic2D/Scene/Components/UIComponents.h"

#include "Runic2D/Renderer/Renderer2D.h"
#include "Runic2D/Renderer/RenderCommand.h"
#include "Runic2D/Renderer/PostProcessing.h"

namespace Runic2D {

	Render2DSystem::Render2DSystem()
	{
		// Lights
		FrameBufferSpecification fbSpec;
		fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::Depth };
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		m_LightmapFBO = FrameBuffer::Create(fbSpec);

		// HDR
		FrameBufferSpecification hdrSpec;
		hdrSpec.Attachments = { FramebufferTextureFormat::RGBA16F, FramebufferTextureFormat::Depth };
		hdrSpec.Width = 1280;
		hdrSpec.Height = 720;
		m_MainHDR_FBO = FrameBuffer::Create(hdrSpec);
	}
	void Render2DSystem::ResizeLightmap(uint32_t width, uint32_t height)
	{
		if (m_ViewportWidth == width && m_ViewportHeight == height) return;
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		m_LightmapFBO->Resize(width, height);
		m_MainHDR_FBO->Resize(width, height);

		PostProcessing::OnWindowResize(width, height);
	}

	void Render2DSystem::OnRender(Scene* scene)
	{
		R2D_PROFILE_SCOPE("Render System: OnRender");

		int editorFBO = RenderCommand::GetBoundFramebuffer();

		m_MainHDR_FBO->Bind();
		RenderCommand::Clear();

		bool hasCamera = false;
		glm::mat4 cameraViewProj;
		glm::mat4 cameraTransformMatrix;
		Camera* activeCamera = nullptr;

		if (m_UseCustomCamera)
		{
			hasCamera = true;
			cameraViewProj = m_CustomViewProj;
		}
		else
		{
			auto cameraEntity = scene->GetPrimaryCameraEntity();
			if (cameraEntity)
			{
				hasCamera = true;
				activeCamera = &cameraEntity.GetComponent<CameraComponent>().Camera;
				cameraTransformMatrix = cameraEntity.GetComponent<TransformComponent>().GetTransform();
			}
		}

		if (!hasCamera)
		{
			m_MainHDR_FBO->Unbind();
			RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
			RenderCommand::Clear();
			return;
		}

		auto BeginSceneWrapper = [&]()
			{
				if (m_UseCustomCamera) Renderer2D::BeginScene(cameraViewProj);
				else Renderer2D::BeginScene(*activeCamera, cameraTransformMatrix);
			};

		// === PASS 1: Main Scene ===
		BeginSceneWrapper();

		auto& registry = scene->GetEntityRegistry();

		auto view = registry.view<TransformComponent, SpriteRendererComponent>(entt::exclude<RectTransformComponent>);
		view.each([&](auto entityID, auto& transform, auto& sprite)
			{
				Entity e{ entityID, scene };
				glm::mat4 worldTransform = e.GetWorldTransform();
				Renderer2D::DrawSprite(worldTransform, sprite, (int)entityID);
			});

		auto circleView = registry.view<TransformComponent, CircleRendererComponent>(entt::exclude<RectTransformComponent>);
		circleView.each([&](auto entityID, auto& transform, auto& circle)
			{
				Entity e{ entityID, scene };
				glm::mat4 worldTransform = e.GetWorldTransform();
				Renderer2D::DrawCircle(worldTransform, circle.Color, circle.Thickness, circle.Fade, (int)entityID);
			});

		Renderer2D::EndScene();

		// === PASS 2: LIGHTMAP (Dark + Lights) ===
		int hdrFBO = RenderCommand::GetBoundFramebuffer();
		m_LightmapFBO->Bind();

		glm::vec4 ambientColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		auto ambientView = registry.view<AmbientLightComponent>();
		for (auto e : ambientView) {
			ambientColor = ambientView.get<AmbientLightComponent>(e).Color;
			break;
		}

		RenderCommand::SetClearColor(ambientColor);
		RenderCommand::Clear();
		RenderCommand::SetBlendMode(BlendMode::Additive);
		RenderCommand::SetDepthMask(false);

		BeginSceneWrapper();

		registry.view<TransformComponent, PointLight2DComponent>(entt::exclude<RectTransformComponent>).each([&](auto entityID, auto& transform, auto& light) {
			Entity e{ entityID, scene };
			glm::mat4 lightTransform = glm::scale(e.GetWorldTransform(), glm::vec3(light.Radius, light.Radius, 1.0f));
			Renderer2D::DrawPointLight(lightTransform, light, (int)entityID);
			});

		Renderer2D::EndScene();
		RenderCommand::SetDepthMask(true);

		// === PASS 3: MULTIPLY ===
		m_LightmapFBO->Unbind();
		m_MainHDR_FBO->Bind();
		RenderCommand::BindFramebuffer(hdrFBO); // Tornem a l'FBO HDR
		RenderCommand::SetViewport(0, 0, m_ViewportWidth, m_ViewportHeight);
		RenderCommand::SetBlendMode(BlendMode::Multiply);
		RenderCommand::ClearDepth();
		RenderCommand::EnableEntityIDWriting(false);

		Renderer2D::BeginScene(glm::mat4(1.0f));

		Ref<Texture2D> lightmapTexture = Texture2D::Create(m_LightmapFBO->GetColorAttachmentRendererID(), m_ViewportWidth, m_ViewportHeight);
		glm::mat4 quadTransform = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 1.0f));
		Renderer2D::DrawQuad(quadTransform, lightmapTexture);

		Renderer2D::EndScene();
		RenderCommand::EnableEntityIDWriting(true);

		// === PASS 4: TEXT (Per sobre de la llum) ===
		RenderCommand::SetBlendMode(BlendMode::Alpha);
		BeginSceneWrapper();

		registry.view<TransformComponent, TextComponent>(entt::exclude<RectTransformComponent>).each([&](auto entityID, auto& transform, auto& text)
			{
				if (!text.Visible) return;
				Entity e{ entityID, scene };
				glm::mat4 worldTransform = e.GetWorldTransform();
				Renderer2D::DrawString(text.GetText(), text.FontAsset, worldTransform, text.Color, text.Kerning, text.LineSpacing, (int)entityID, (int)text.TextAlignment);
			});

		Renderer2D::EndScene();

		// === PASS 5: POST-PROCESSING (Filtres finals) ========
		m_MainHDR_FBO->Unbind();

		RenderCommand::BindFramebuffer(editorFBO);
		RenderCommand::SetViewport(0, 0, m_ViewportWidth, m_ViewportHeight);
		RenderCommand::Clear();
		RenderCommand::SetDepthMask(false);

		Ref<Texture2D> hdrTexture = Texture2D::Create(m_MainHDR_FBO->GetColorAttachmentRendererID(), m_ViewportWidth, m_ViewportHeight);

		PostProcessing::Render(hdrTexture);

		RenderCommand::SetDepthMask(true);
	}
}

