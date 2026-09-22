#include "R2Dpch.h"
#include "DebugSystem.h"

#include "Runic2D/Scene/Scene.h"
#include "Runic2D/Scene/Components/CoreComponents.h"
#include "Runic2D/Scene/Components/ScriptingComponents.h"
#include "Runic2D/Scene/Components/PhysicsComponents.h"

#include "Runic2D/Renderer/Renderer2D.h"
#include "Runic2D/Systems/Render/ParticleSystem.h"
#include "Runic2D/Systems/AI/FlockingSystem.h"

#include "Runic2D/Core/App/Application.h"

namespace Runic2D {

	DebugStats DebugSystem::GetStats(Scene* scene) const
	{
		DebugStats stats;
		stats.TotalEntities = (uint32_t)scene->GetSizeOfAllEntities();
		stats.ScriptUpdates = (uint32_t)scene->GetEntityRegistry().view<NativeScriptComponent>().size();

		if (auto particlesSystem = scene->GetSystem<ParticleSystem>()) {
			stats.ActiveParticles = (uint32_t)particlesSystem->GetActiveParticleCount();
		}
		return stats;
	}

	void DebugSystem::OnRender(Scene* scene)
	{
        if (m_ShowColliders)
        {
			if (m_UseCustomCamera) {
				Renderer2D::BeginScene(m_CustomViewProj);
			}
			else {
				Entity cam = scene->GetPrimaryCameraEntity();
				if (cam) {
					auto& camera = cam.GetComponent<CameraComponent>().Camera;
					auto& tc = cam.GetComponent<TransformComponent>();
					Renderer2D::BeginScene(camera, tc.GetTransform());
				}
			}

			auto& registry = scene->GetEntityRegistry();

			auto viewbc = registry.view<TransformComponent, BoxCollider2DComponent>();
			viewbc.each([&](auto entityID, auto& tc, auto& bc2d)
				{
					Entity e{ entityID, scene };

					glm::mat4 worldTransform = e.GetWorldTransform();

					glm::mat4 colliderTransform = worldTransform
						* glm::translate(glm::mat4(1.0f), glm::vec3(bc2d.Offset, 0.001f))
						* glm::scale(glm::mat4(1.0f), glm::vec3(bc2d.Size, 1.0f));
					Renderer2D::DrawRect(colliderTransform, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
				});

			auto viewcc = registry.view<TransformComponent, CircleCollider2DComponent>();
			viewcc.each([&](auto entityID, auto& tc, auto& cc2d)
				{
					Entity e{ entityID, scene };

					glm::mat4 worldTransform = e.GetWorldTransform();

					glm::mat4 colliderTransform = worldTransform
						* glm::translate(glm::mat4(1.0f), glm::vec3(cc2d.Offset, 0.001f))
						* glm::scale(glm::mat4(1.0f), glm::vec3(cc2d.Radius * 2.0f));
					Renderer2D::DrawCircle(colliderTransform, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), 0.05f, 0.01f, (int)entityID);
				});

			auto viewShadow = registry.view<TransformComponent, ShadowCaster2DComponent>();
			viewShadow.each([&](auto entity, auto& tc, auto& caster)
				{
					if (caster.CustomShape.empty()) return;
					Entity e{ entity, scene };
					glm::mat4 worldTransform = e.GetWorldTransform();
					glm::vec4 magentaColor = { 1.0f, 0.0f, 1.0f, 1.0f };
					for (size_t i = 0; i < caster.CustomShape.size(); i++)
					{
						glm::vec3 p0 = worldTransform * glm::vec4(caster.CustomShape[i], 0.0f, 1.0f);
						glm::vec3 p1 = worldTransform * glm::vec4(caster.CustomShape[(i + 1) % caster.CustomShape.size()], 0.0f, 1.0f);
						Renderer2D::DrawLine(p0, p1, magentaColor);
					}
				});

			auto viewpc = registry.view<TransformComponent, PolygonCollider2DComponent>();
			viewpc.each([&](auto entityID, auto& tc, auto& pc2d)
				{
					if (pc2d.Vertices.size() < 2) return; 
					Entity e{ entityID, scene };
					glm::mat4 worldTransform = e.GetWorldTransform();

					glm::mat4 colliderTransform = worldTransform
						* glm::translate(glm::mat4(1.0f), glm::vec3(pc2d.Offset, 0.001f));
					glm::vec4 greenColor = { 0.0f, 1.0f, 0.0f, 1.0f }; 
					for (size_t i = 0; i < pc2d.Vertices.size(); i++)
					{
						glm::vec3 p0 = colliderTransform * glm::vec4(pc2d.Vertices[i], 0.0f, 1.0f);
						glm::vec3 p1 = colliderTransform * glm::vec4(pc2d.Vertices[(i + 1) % pc2d.Vertices.size()], 0.0f, 1.0f);
						Renderer2D::DrawLine(p0, p1, greenColor);
					}
				});

			Renderer2D::EndScene();
        }

        if (m_ShowStats)
        {
			Renderer2D::SetRecordStats(false);

			auto sceneStats = GetStats(scene);
			auto renderStats = Renderer2D::GetStats();
			
			// Overlay virtual viewport (1080p height)
			float aspectRatio = static_cast<float>(scene->GetViewportWidth()) / static_cast<float>(scene->GetViewportHeight());
			float refHeight = 1080.0f;
			float refWidth = refHeight * aspectRatio;

			glm::mat4 projection = glm::ortho(0.0f, refWidth, 0.0f, refHeight, -1.0f, 1.0f);
			Renderer2D::BeginScene(projection);

			std::string debugStr = "FPS: " + std::to_string((int)Application::Get().GetAverageFPS()) + "\n";
			debugStr += "Renderer Stats:\n";
			debugStr += "  Draw Calls: " + std::to_string(renderStats.DrawCalls) + "\n";
			debugStr += "  Quads: " + std::to_string(renderStats.QuadCount) + "\n";
			debugStr += "\nScene Stats:\n";
			debugStr += "  Entities: " + std::to_string(sceneStats.TotalEntities) + "\n";
			debugStr += "  Scripts: " + std::to_string(sceneStats.ScriptUpdates) + "\n";
			debugStr += "  Particles: " + std::to_string(sceneStats.ActiveParticles);

			float fontSize = 32.0f;
			glm::mat4 transform = glm::translate(glm::mat4(1.0f), { 20.0f, refHeight - 100.0f, 0.0f });
			transform = glm::scale(transform, { fontSize, fontSize, 1.0f });

			Renderer2D::DrawString(debugStr, Font::GetDefault(), transform, { 1.0f, 1.0f, 1.0f, 1.0f }, 0.0f, 0.0f, -1, 0);

			Renderer2D::EndScene();
			Renderer2D::SetRecordStats(true);
        }

		if (s_ShowFlockingGrid)
		{
			if (m_UseCustomCamera) {
				Renderer2D::BeginScene(m_CustomViewProj);
			}
			else {
				Entity cam = scene->GetPrimaryCameraEntity();
				if (cam) {
					auto& camera = cam.GetComponent<CameraComponent>().Camera;
					auto& tc = cam.GetComponent<TransformComponent>();
					Renderer2D::BeginScene(camera, tc.GetTransform());
				}
			}

			auto flockingSystem = scene->GetSystem<FlockingSystem>();
			if (flockingSystem)
			{
				const auto& spatialHash = flockingSystem->GetSpatialHash();
				float cellSize = spatialHash.GetCellSize();
				for (const auto& [key, entities] : spatialHash.GetGrid())
				{
					if (entities.empty()) continue; 
					int cellX, cellY;
					spatialHash.GetCellCoordinates(key, cellX, cellY);

					glm::vec2 center = { (cellX * cellSize) + (cellSize * 0.5f),
										 (cellY * cellSize) + (cellSize * 0.5f) };

					glm::vec3 pos = { center.x, center.y, 0.5f };

					glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * glm::scale(glm::mat4(1.0f), { cellSize, cellSize, 1.0f });
					Renderer2D::DrawRect(transform, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
				}
			}

			Renderer2D::EndScene();
		}
	}

	void DebugSystem::DrawCameraBounds(Scene* scene)
	{
		if (!m_UseCustomCamera) return;

		Entity cam = scene->GetPrimaryCameraEntity();
		if (!cam) return;
			
		auto& camera = cam.GetComponent<CameraComponent>().Camera;
		auto& tc = cam.GetComponent<TransformComponent>();

		Renderer2D::BeginScene(m_CustomViewProj);

		float orthoSize = camera.GetOrthographicSize();
		float height = orthoSize;

		float aspectRatio = camera.GetAspectRatio();
		float width = height * aspectRatio;

		glm::mat4 debugTransform = tc.GetTransform() * glm::scale(glm::mat4(1.0f), { width, height, 1.0f });

		Renderer2D::DrawRect(debugTransform, { 0.0f, 1.0f, 0.0f, 1.0f });

		Renderer2D::EndScene();
	}
}
