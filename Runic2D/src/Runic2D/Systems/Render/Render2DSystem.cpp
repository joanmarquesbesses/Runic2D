#include "R2Dpch.h"
#include "Render2DSystem.h"

#include "Runic2D/Scene/Entity.h"
#include "Runic2D/Scene/Scene.h"
#include "Runic2D/Scene/Components/CoreComponents.h"
#include "Runic2D/Scene/Components/RenderComponents.h"
#include "Runic2D/Scene/Components/UIComponents.h"
#include "Runic2D/Scene/Components/PhysicsComponents.h"

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
		hdrSpec.Attachments = { FramebufferTextureFormat::RGBA16F, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
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
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		RenderCommand::Clear();
		m_MainHDR_FBO->ClearAttachment(1, -1);

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

		// === PASS 2: LIGHTMAP (Dark + Lights + SHADOWS) ===
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

		// Llistes de la fase d'Il·luminació
		auto lightView = registry.view<TransformComponent, PointLight2DComponent>(entt::exclude<RectTransformComponent>);
		auto casterView = registry.view<TransformComponent, ShadowCaster2DComponent>();

		lightView.each([&](auto lightEntity, auto& lightTc, auto& light)
			{
				Entity lightE{ lightEntity, scene };
				glm::mat4 lightWorldTx = lightE.GetWorldTransform();
				glm::vec3 lightPos = glm::vec3(lightWorldTx[3]);

				RenderCommand::ClearStencil();
				RenderCommand::BeginStencilWrite();

				BeginSceneWrapper();

				casterView.each([&](auto casterEntity, auto& casterTc, auto& caster)
					{
						if (!caster.CastShadows) return;

						Entity casterE{ casterEntity, scene };
						glm::mat4 casterWorldTx = casterE.GetWorldTransform();
						glm::vec3 casterPos = glm::vec3(casterWorldTx[3]);
						float maxScale = std::max(casterTc.GetScale().x, casterTc.GetScale().y);

						if (glm::distance(glm::vec2(lightPos), glm::vec2(casterPos)) > (light.Radius + maxScale))
							return;

						glm::mat4 transform = casterE.GetWorldTransform();
						std::vector<glm::vec3> corners;

						if (!caster.CustomShape.empty())
						{
							for (const auto& point : caster.CustomShape) {
								corners.push_back(transform * glm::vec4(point.x, point.y, 0.0f, 1.0f));
							}
						}
						else if (casterE.HasComponent<PolygonCollider2DComponent>() && !casterE.GetComponent<PolygonCollider2DComponent>().Vertices.empty())
						{
							auto& pc = casterE.GetComponent<PolygonCollider2DComponent>();
							for (const auto& point : pc.Vertices) {
								corners.push_back(transform * glm::vec4(point.x + pc.Offset.x, point.y + pc.Offset.y, 0.0f, 1.0f));
							}
						}
						else
						{
							glm::vec2 offset = { 0.0f, 0.0f };
							glm::vec2 size = { 1.0f, 1.0f };
							if (casterE.HasComponent<BoxCollider2DComponent>()) {
								auto& bc = casterE.GetComponent<BoxCollider2DComponent>();
								offset = bc.Offset;
								size = bc.Size;
							}
							corners.push_back(transform * glm::vec4(offset.x - size.x * 0.5f, offset.y - size.y * 0.5f, 0.0f, 1.0f));
							corners.push_back(transform * glm::vec4(offset.x + size.x * 0.5f, offset.y - size.y * 0.5f, 0.0f, 1.0f));
							corners.push_back(transform * glm::vec4(offset.x + size.x * 0.5f, offset.y + size.y * 0.5f, 0.0f, 1.0f));
							corners.push_back(transform * glm::vec4(offset.x - size.x * 0.5f, offset.y + size.y * 0.5f, 0.0f, 1.0f));
						}
						int cornerCount = corners.size();

						float signedArea = 0.0f;
						for (int i = 0; i < cornerCount; i++) {
							glm::vec3 p1 = corners[i];
							glm::vec3 p2 = corners[(i + 1) % cornerCount];
							signedArea += (p2.x - p1.x) * (p2.y + p1.y);
						}
						bool isClockwise = (signedArea > 0.0f);
						for (int i = 0; i < cornerCount; i++)
						{
							glm::vec3 p_curr = corners[i];
							glm::vec3 p_next = corners[(i + 1) % cornerCount];
							glm::vec2 diff_curr = glm::vec2(p_curr) - glm::vec2(lightPos);
							glm::vec2 diff_next = glm::vec2(p_next) - glm::vec2(lightPos);
							if (glm::length(diff_next) < 0.001f || glm::length(diff_curr) < 0.001f) continue;

							glm::vec2 edgeNormal = { p_next.y - p_curr.y, p_curr.x - p_next.x };

							if (isClockwise) edgeNormal = -edgeNormal;
							glm::vec2 lightToEdge = glm::vec2((p_curr + p_next) * 0.5f) - glm::vec2(lightPos);
							if (glm::dot(edgeNormal, lightToEdge) > 0.0f)
							{
								glm::vec2 dir_curr = glm::normalize(diff_curr);
								glm::vec2 dir_next = glm::normalize(diff_next);
								glm::vec3 ext_curr = glm::vec3(glm::vec2(p_curr) + dir_curr * light.Radius * 1.5f, p_curr.z);
								glm::vec3 ext_next = glm::vec3(glm::vec2(p_next) + dir_next * light.Radius * 1.5f, p_next.z);
								Renderer2D::DrawShadowPolygon(p_curr, p_next, ext_next, ext_curr);
							}
						}
					});

				Renderer2D::EndScene();
				RenderCommand::BeginStencilTest();

				BeginSceneWrapper();

				glm::mat4 lightTransform = glm::scale(lightE.GetWorldTransform(), glm::vec3(light.Radius, light.Radius, 1.0f));

				Renderer2D::DrawPointLight(lightTransform, light, (int)lightEntity);
				Renderer2D::EndScene();
			});

		RenderCommand::DisableStencil();
		RenderCommand::SetDepthMask(true);

		// === PASS 3: MULTIPLY ===
		m_LightmapFBO->Unbind();

		Ref<Texture2D> rawLightmap = Texture2D::Create(m_LightmapFBO->GetColorAttachmentRendererID(), m_ViewportWidth, m_ViewportHeight);
		Ref<Texture2D> blurredLightmap = rawLightmap;

		if (m_ShadowBlurIterations > 0)
		{
			blurredLightmap = PostProcessing::ApplyBlur(rawLightmap, m_ShadowBlurIterations);
		}

		m_MainHDR_FBO->Bind();

		RenderCommand::BindFramebuffer(hdrFBO);
		RenderCommand::SetViewport(0, 0, m_ViewportWidth, m_ViewportHeight);
		RenderCommand::SetBlendMode(BlendMode::Multiply);
		RenderCommand::ClearDepth();
		RenderCommand::EnableEntityIDWriting(false);

		Renderer2D::BeginScene(glm::mat4(1.0f));

		glm::mat4 quadTransform = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 1.0f));


		Renderer2D::DrawQuad(quadTransform, blurredLightmap);
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

		m_MainHDR_FBO->CopyEntityIDsTo(editorFBO, m_ViewportWidth, m_ViewportHeight);
	}
}

