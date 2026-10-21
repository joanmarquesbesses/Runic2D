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
#include "Runic2D/Renderer/TiledMapAsset.h"

#include "Runic2D/Assets/ResourceManager.h"

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

		m_LightmapTexture = Texture2D::Create(m_LightmapFBO->GetColorAttachmentRendererID(), 1280, 720);
		m_MainHDRTexture = Texture2D::Create(m_MainHDR_FBO->GetColorAttachmentRendererID(), 1280, 720);
	}

	void Render2DSystem::ResizeLightmap(uint32_t width, uint32_t height)
	{
		if (m_ViewportWidth == width && m_ViewportHeight == height) return;
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		m_LightmapFBO->Resize(width, height);
		m_MainHDR_FBO->Resize(width, height);

		m_LightmapTexture = Texture2D::Create(m_LightmapFBO->GetColorAttachmentRendererID(), width, height);
		m_MainHDRTexture = Texture2D::Create(m_MainHDR_FBO->GetColorAttachmentRendererID(), width, height);

		PostProcessing::OnWindowResize(width, height);
	}

	void Render2DSystem::RenderCamera::Begin() const
	{
		if (IsCustom) Renderer2D::BeginScene(ViewProj);
		else if (CameraPtr) Renderer2D::BeginScene(*CameraPtr, Transform);
	}

	Render2DSystem::RenderCamera Render2DSystem::GetActiveCamera(Scene* scene)
	{
		RenderCamera cam;
		if (m_UseCustomCamera)
		{
			cam.Valid = true;
			cam.IsCustom = true;
			cam.ViewProj = m_CustomViewProj;
		}
		else
		{
			auto cameraEntity = scene->GetPrimaryCameraEntity();
			if (cameraEntity)
			{
				cam.Valid = true;
				cam.CameraPtr = &cameraEntity.GetComponent<CameraComponent>().Camera;
				cam.Transform = cameraEntity.GetComponent<TransformComponent>().GetTransform();
			}
		}
		return cam;
	}

	void Render2DSystem::OnRender(Scene* scene)
	{
		R2D_PROFILE_SCOPE("Render System: OnRender");
		int editorFBO = RenderCommand::GetBoundFramebuffer();
		RenderCamera camera = GetActiveCamera(scene);
		if (!camera.Valid)
		{
			m_MainHDR_FBO->Unbind();
			RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
			RenderCommand::Clear();
			return;
		}
		
		m_MainHDR_FBO->Bind();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		RenderCommand::Clear();
		m_MainHDR_FBO->ClearAttachment(1, -1);
		// PASS 1: Geometria de l'Escena (Sprites, Cercle, Tilemap)
		RenderSceneGeometry(scene, camera);
		// PASS 2: Mapa de Llums i Ombres i PASS 3: Multiplicar llum sobre l'escena
		if (m_LightingEnabled)
		{
			RenderLightmap(scene, camera);
			ApplyLightmapToScene();
		}
		// PASS 4: Textos
		RenderText(scene, camera);
		// PASS 5: Post-Processat (Bloom) i enviament al framebuffer final
		RenderPostProcessing(editorFBO);
	}

	void Render2DSystem::RenderSceneGeometry(Scene* scene, const RenderCamera& camera)
	{
		camera.Begin();
		auto& registry = scene->GetEntityRegistry();
		// Sprites
		auto view = registry.view<TransformComponent, SpriteRendererComponent>(entt::exclude<RectTransformComponent>);
		view.each([&](auto entityID, auto& transform, auto& sprite)
			{
				Entity e{ entityID, scene };
				glm::mat4 worldTransform = e.GetWorldTransform();
				Renderer2D::DrawSprite(worldTransform, sprite, (int)entityID);
			});
		// Cercles
		auto circleView = registry.view<TransformComponent, CircleRendererComponent>(entt::exclude<RectTransformComponent>);
		circleView.each([&](auto entityID, auto& transform, auto& circle)
			{
				Entity e{ entityID, scene };
				glm::mat4 worldTransform = e.GetWorldTransform();
				Renderer2D::DrawCircle(worldTransform, circle.Color, circle.Thickness, circle.Fade, (int)entityID);
			});
		// Tilemaps
		RenderTilemaps(scene);

		Renderer2D::EndScene();
	}

	void Render2DSystem::RenderTilemaps(Scene* scene)
	{
		auto& registry = scene->GetEntityRegistry();

		auto tilemapView = registry.view<TransformComponent, TilemapComponent>(entt::exclude<RectTransformComponent>);
		tilemapView.each([&](auto entityID, auto& transform, auto& tilemap)
			{
				if (tilemap.MapHandle == 0) return;

				auto map = ResourceManager::Get<TiledMapAsset>(tilemap.MapHandle);
				if (!map || map->TilesetTextureHandle == 0) return;

				auto texture = ResourceManager::Get<Texture2D>(map->TilesetTextureHandle);
				if (!texture) return;

				Entity e{ entityID, scene };

				glm::mat4 baseTransform = e.GetWorldTransform();
				float tileWidthUnits = 1.0f;
				float tileHeightUnits = 1.0f;
				float layerZOffset = 0.0f;

				for (const auto& layer : map->TileLayers)
				{
					for (int y = 0; y < map->MapHeight; y++)
					{
						for (int x = 0; x < map->MapWidth; x++)
						{
							int i = x + (y * map->MapWidth);
							if (layer.Data.empty() || i >= layer.Data.size()) continue;

							unsigned int rawTileID = layer.Data[i];
							if (rawTileID == 0) continue;

							bool flipX = (rawTileID & 0x80000000) != 0;
							bool flipY = (rawTileID & 0x40000000) != 0;
							unsigned int tileID = rawTileID & 0x1FFFFFFF;
							int localID = tileID - 1;

							if (map->TileCache.find(localID) == map->TileCache.end())
							{
								int col = localID % map->TilesetColumns;
								int row = localID / map->TilesetColumns;
								int totalRows = texture->GetHeight() / map->TileHeight;
								int openGLRow = (totalRows - 1) - row;
								map->TileCache[localID] = SubTexture2D::CreateFromCoords(texture, { col, openGLRow }, { (float)map->TileWidth, (float)map->TileHeight });
							}

							auto subTex = map->TileCache[localID];

							glm::vec3 pos(x * tileWidthUnits, -y * tileHeightUnits, layerZOffset);
							glm::mat4 tileTransform = glm::translate(baseTransform, pos) * glm::scale(glm::mat4(1.0f), glm::vec3(tileWidthUnits, tileHeightUnits, 1.0f));

							Renderer2D::DrawQuad(tileTransform, subTex, 1.0f, glm::vec4(1.0f), (int)entityID, flipX, flipY);
						}
					}
					layerZOffset += 0.1f;
				}
			});
	}

	void Render2DSystem::RenderLightmap(Scene* scene, const RenderCamera& camera)
	{
		m_LightmapFBO->Bind();

		auto& registry = scene->GetEntityRegistry();
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

		auto lightView = registry.view<TransformComponent, PointLight2DComponent>(entt::exclude<RectTransformComponent>);	
		lightView.each([&](auto lightEntity, auto& lightTc, auto& light)
			{
				Entity lightE{ lightEntity, scene };
				glm::mat4 lightWorldTx = lightE.GetWorldTransform();
				glm::vec3 lightPos = glm::vec3(lightWorldTx[3]);
				// Ombres amb Stencil Buffer
				if (m_ShadowsEnabled)
				{
					RenderCommand::ClearStencil();
					RenderCommand::BeginStencilWrite();
					RenderShadowsForLight(scene, lightPos, light.Radius, camera);
					RenderCommand::BeginStencilTest();
				}
				// Dibuixar la llum pròpiament dita
				camera.Begin();

				glm::mat4 lightTransform = glm::scale(lightE.GetWorldTransform(), glm::vec3(light.Radius, light.Radius, 1.0f));

				Renderer2D::DrawPointLight(lightTransform, light, (int)lightEntity);
				Renderer2D::EndScene();
			});

		RenderCommand::DisableStencil();
		RenderCommand::SetDepthMask(true);
	}

	void Render2DSystem::RenderShadowsForLight(Scene* scene, const glm::vec3& lightPos, float lightRadius, const RenderCamera& camera)
	{
		camera.Begin();

		auto& registry = scene->GetEntityRegistry();

		auto casterView = registry.view<TransformComponent, ShadowCaster2DComponent>();
		casterView.each([&](auto casterEntity, auto& casterTc, auto& caster)
			{
				if (!caster.CastShadows) return;
				
				Entity casterE{ casterEntity, scene };
				
				glm::mat4 casterWorldTx = casterE.GetWorldTransform();
				glm::vec3 casterPos = glm::vec3(casterWorldTx[3]);

				float maxScale = std::max(casterTc.GetScale().x, casterTc.GetScale().y);
				if (glm::distance(glm::vec2(lightPos), glm::vec2(casterPos)) > (lightRadius + maxScale))
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

				int cornerCount = (int)corners.size();
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
						glm::vec3 ext_curr = glm::vec3(glm::vec2(p_curr) + dir_curr * lightRadius * 1.5f, p_curr.z);
						glm::vec3 ext_next = glm::vec3(glm::vec2(p_next) + dir_next * lightRadius * 1.5f, p_next.z);
						Renderer2D::DrawShadowPolygon(p_curr, p_next, ext_next, ext_curr);
					}
				}
			});

		Renderer2D::EndScene();
	}

	void Render2DSystem::ApplyLightmapToScene()
	{
		m_LightmapFBO->Unbind();

		Ref<Texture2D> blurredLightmap = m_LightmapTexture;

		if (m_ShadowBlurIterations > 0)
		{
			blurredLightmap = PostProcessing::ApplyBlur(m_LightmapTexture, m_ShadowBlurIterations);
		}

		m_MainHDR_FBO->Bind();

		RenderCommand::SetViewport(0, 0, m_ViewportWidth, m_ViewportHeight);
		RenderCommand::SetBlendMode(BlendMode::Multiply);
		RenderCommand::ClearDepth();
		RenderCommand::EnableEntityIDWriting(false);

		Renderer2D::BeginScene(glm::mat4(1.0f));

		glm::mat4 quadTransform = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 1.0f));

		Renderer2D::DrawQuad(quadTransform, blurredLightmap);
		Renderer2D::EndScene();

		RenderCommand::EnableEntityIDWriting(true);
	}

	void Render2DSystem::RenderText(Scene* scene, const RenderCamera& camera)
	{
		RenderCommand::SetBlendMode(BlendMode::Alpha);

		camera.Begin();

		auto& registry = scene->GetEntityRegistry();
		registry.view<TransformComponent, TextComponent>(entt::exclude<RectTransformComponent>).each([&](auto entityID, auto& transform, auto& text)
			{
				if (!text.Visible) return;
				Entity e{ entityID, scene };
				glm::mat4 worldTransform = e.GetWorldTransform();
				Renderer2D::DrawString(text.GetText(), text.FontAsset, worldTransform, text.Color, text.Kerning, text.LineSpacing, (int)entityID, (int)text.TextAlignment);
			});

		Renderer2D::EndScene();
	}

	void Render2DSystem::RenderPostProcessing(int destinationFBO)
	{
		m_MainHDR_FBO->Unbind();

		RenderCommand::BindFramebuffer(destinationFBO);
		RenderCommand::SetViewport(0, 0, m_ViewportWidth, m_ViewportHeight);
		RenderCommand::Clear();
		RenderCommand::SetDepthMask(false);

		if (m_BloomEnabled && m_BloomIterations > 0)
		{
			PostProcessing::Render(m_MainHDRTexture, m_BloomIterations, m_BloomIntensity, m_BloomThreshold);
		}
		else
		{
			Renderer2D::BeginScene(glm::mat4(1.0f));
			glm::mat4 quadTransform = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 1.0f));
			Renderer2D::DrawQuad(quadTransform, m_MainHDRTexture);
			Renderer2D::EndScene();
		}

		RenderCommand::SetDepthMask(true);

		m_MainHDR_FBO->CopyEntityIDsTo(destinationFBO, m_ViewportWidth, m_ViewportHeight);
	}
}

