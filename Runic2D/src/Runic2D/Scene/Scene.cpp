#include "R2Dpch.h"
#include "Scene.h"

#include "Runic2D/Renderer/Renderer2D.h"
#include "Runic2D/Renderer/RenderCommand.h"
#include "Runic2D/Math/Math.h"

#include "Runic2D/Core/Application.h"
#include "Runic2D/Core/Input.h"
#include "Runic2D/Core/JobSystem.h"

#include "Runic2D/Systems/System.h"
#include "Runic2D/Systems/ScriptingSystem.h"
#include "Runic2D/Systems/PhysicsSystem.h"
#include "Runic2D/Systems/TransformSystem.h"
#include "Runic2D/Systems/UISystem.h"
#include "Runic2D/Systems/Render2DSystem.h"
#include "Runic2D/Systems/ParticleSystem.h"

#include "Entity.h"
#include "Component.h"
#include "ComponentRegistry.h"
#include "Tween.h"

namespace Runic2D {
	
	Scene::Scene()
	{
		m_Registry.on_construct<CameraComponent>().connect<&Scene::OnCameraComponentConstruct>(this);

		// Scripting
		AddSystem(CreateRef<ScriptingSystem>(), { SystemPhase::Logic });
		// Physiscs
		AddSystem(CreateRef<PhysicsSystem>(), { SystemPhase::Physics });
		// TransformSystem
		AddSystem(CreateRef<TransformSystem>(), { SystemPhase::PostUpdate });
		// 2D Render
		AddSystem(CreateRef<Render2DSystem>(), { SystemPhase::Render });
		// Particles
		AddSystem(CreateRef<ParticleSystem>(), { SystemPhase::Logic, SystemPhase::Render });
		// UI System
		AddSystem(CreateRef<UISystem>(), { SystemPhase::Logic, SystemPhase::Render });
	}

	Scene::~Scene()
	{
		m_Registry.on_construct<CameraComponent>().disconnect<&Scene::OnCameraComponentConstruct>(this);
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	template<typename Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		auto view = src.view<Component>();
		for (auto e : view)
		{
			UUID uuid = src.get<IDComponent>(e).ID;
			R2D_CORE_ASSERT(enttMap.find(uuid) != enttMap.end());
			entt::entity dstEnttID = enttMap.at(uuid);

			auto& component = src.get<Component>(e);
			dst.emplace_or_replace<Component>(dstEnttID, component);
		}
	}

	template<typename Component>
	static void CopyComponentIfExists(Entity dst, Entity src)
	{
		if (src.HasComponent<Component>())
			dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
	}

	Ref<Scene> Scene::Copy(Ref<Scene> other)
	{
		Ref<Scene> newScene = CreateRef<Scene>();

		newScene->m_ViewportWidth = other->m_ViewportWidth;
		newScene->m_ViewportHeight = other->m_ViewportHeight;

		auto& srcSceneRegistry = other->m_Registry;
		auto& dstSceneRegistry = newScene->m_Registry;

		std::unordered_map<UUID, entt::entity> enttMap;

		auto idView = srcSceneRegistry.view<IDComponent>();
		std::vector<entt::entity> entitiesToCopy;
		entitiesToCopy.reserve(idView.size());
		for (auto e : idView) entitiesToCopy.push_back(e);

		for (auto it = entitiesToCopy.rbegin(); it != entitiesToCopy.rend(); ++it)
		{
			entt::entity e = *it;
			UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
			const auto& name = srcSceneRegistry.get<TagComponent>(e).Tag;
			Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
			enttMap[uuid] = (entt::entity)newEntity;
		}

		CopyComponent<TransformComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<SpriteRendererComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<CircleRendererComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<CameraComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<NativeScriptComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<Rigidbody2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<BoxCollider2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<RelationshipComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<CircleCollider2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<TextComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<AnimationComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<RectTransformComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<ButtonComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);

		for (auto e : idView)
		{
			UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;

			Entity sourceEntity = { e, other.get() };
			Entity targetEntity = { enttMap[uuid], newScene.get() };

			for (const auto& desc : ComponentRegistry::GetAll())
			{
				if (desc.IsEngineComponent) continue;

				if (desc.HasOnEntity(sourceEntity) && desc.CopyComponent)
				{
					desc.CopyComponent(sourceEntity, targetEntity);
				}
			}
		}

		auto relationshipView = dstSceneRegistry.view<RelationshipComponent>();
		for (auto e : relationshipView)
		{
			auto& newRC = dstSceneRegistry.get<RelationshipComponent>(e);

			auto remapEntity = [&](entt::entity oldHandle) -> entt::entity {
				if (oldHandle == entt::null) return entt::null;
				UUID uuid = srcSceneRegistry.get<IDComponent>(oldHandle).ID;
				return enttMap.at(uuid);
				};

			newRC.Parent = remapEntity(newRC.Parent);
			newRC.NextSibling = remapEntity(newRC.NextSibling);
			newRC.PrevSibling = remapEntity(newRC.PrevSibling);
			newRC.FirstChild = remapEntity(newRC.FirstChild);
		}

		return newScene;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		if (entity.HasComponent<RelationshipComponent>())
		{
			auto& rc = entity.GetComponent<RelationshipComponent>();
			entt::entity currentChild = rc.FirstChild;
			while (currentChild != entt::null)
			{
				entt::entity nextChild = entt::null;

				Entity childEntity{ currentChild, this };
				if (childEntity.HasComponent<RelationshipComponent>())
					nextChild = childEntity.GetComponent<RelationshipComponent>().NextSibling;

				DestroyEntity(childEntity);
				currentChild = nextChild;
			}
		}

		entity.Unparent(false);

		m_Registry.destroy(entity);
	}

	void Scene::SubmitForDestruction(Entity entity)
	{
		m_DestructionQueue.push_back((entt::entity)entity);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };

		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		entity.AddComponent<RelationshipComponent>();

		return entity;
	}

	Entity Scene::GetEntityByUUID(UUID uuid)
	{
		auto view = m_Registry.view<IDComponent>();
		for (auto entity : view)
		{
			const auto& idComponent = view.get<IDComponent>(entity);
			if (idComponent.ID == uuid)
				return { entity, this };
		}
		return {};
	}

	void Scene::OnFixedUpdateRunTime(Timestep ts)
	{
		if (!m_IsPaused)
		{
			for (auto& logicSystem : m_LogicSystems) {
				logicSystem->OnFixedUpdate(ts, this);
			}
			for (auto& physicsSystem : m_PhysicsSystems) {
				physicsSystem->OnFixedUpdate(ts, this);
			}
		}
	}

	void Scene::OnUpdateRunTime(Timestep ts)
	{
		for (auto& logicSystem : m_LogicSystems) {
			logicSystem->OnUpdate(ts, this);
		}
		for (auto& physicsSystem : m_PhysicsSystems) {
			physicsSystem->OnUpdate(ts, this);
		}
		for (auto& postUpdateSystem : m_PostUpdateSystems) {
			postUpdateSystem->OnUpdate(ts, this);
		}

		UpdateTweens(ts);

		if (!m_IsPaused)
		{
			UpdateAnimation(ts);
		}

		for (auto e : m_DestructionQueue)
		{
			if (m_Registry.valid(e))
			{
				Entity entity{ e, this };
				DestroyEntity(entity);
			}
		}
		m_DestructionQueue.clear();
	}

	void Scene::OnRenderRuntime()
	{
		for (auto& renderSystem : m_RenderSystems) {
			renderSystem->OnRender(this);
		}
	}

	void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
	{
		auto transformSystem = GetSystem<TransformSystem>();
		if (transformSystem) {
			transformSystem->OnUpdate(ts, this);
		}

		UpdateAnimation(ts);
		
		auto render2DSystem = GetSystem<Render2DSystem>();
		if (render2DSystem) {
			render2DSystem->OnRender(this);
		}

		// Draw Camera Bounds
		SceneCamera* mainCamera = nullptr;
		glm::mat4 cameraTransform;

		m_Registry.view<TransformComponent, CameraComponent>().each([&](auto entity, auto& tc, auto& cc)
		{
			if (cc.Primary)
			{
				mainCamera = &cc.Camera;
				cameraTransform = tc.GetTransform();
			}
		});

		if (mainCamera)
		{
			float orthoSize = mainCamera->GetOrthographicSize();
			float height = orthoSize;

			float aspectRatio = mainCamera->GetAspectRatio();
			float width = height * aspectRatio;

			glm::mat4 debugTransform = cameraTransform * glm::scale(glm::mat4(1.0f), { width, height, 1.0f });

			Renderer2D::DrawRect(debugTransform, { 0.0f, 1.0f, 0.0f, 1.0f });
		}

		Renderer2D::EndScene();

		GetSystem<UISystem>()->OnRender(this);
	}

	void Scene::OnRuntimeStart()
	{
		for (auto& logicSystem : m_LogicSystems) {
			logicSystem->OnStart(this);
		}
		for (auto& physicsSystem : m_PhysicsSystems) {
			physicsSystem->OnStart(this);
		}

		m_Registry.view<AnimationComponent>().each([=](auto entity, auto& anim)
		{
			anim.Animations.clear();

			for (auto& profile : anim.Profiles)
			{
				if (profile.AtlasTexture)
				{
					int numCols = (int)(profile.AtlasTexture->GetWidth() / profile.TileSize.x);
					int col = profile.StartFrame % numCols;
					int row = profile.StartFrame / numCols;

					float startX = col * profile.TileSize.x;
					float startY = row * profile.TileSize.y;

					Ref<Animation2D> animAsset = Animation2D::CreateFromAtlas(
						profile.AtlasTexture,
						profile.TileSize,
						{ startX, startY },
						profile.FrameCount,
						profile.FramesPerRow,
						profile.FrameTime
					);

					anim.Animations[profile.Name] = animAsset;
				}
			}

			if (!anim.Animations.empty())
			{
				if (!anim.CurrentStateName.empty() && anim.Animations.find(anim.CurrentStateName) != anim.Animations.end())
				{
					anim.CurrentAnimation = anim.Animations[anim.CurrentStateName];
				}
				else if (anim.Animations.find("Idle") != anim.Animations.end())
				{
					anim.CurrentAnimation = anim.Animations["Idle"];
					anim.CurrentStateName = "Idle";
				}
				else
				{
					anim.CurrentAnimation = anim.Animations.begin()->second;
					anim.CurrentStateName = anim.Animations.begin()->first;
				}

				anim.Playing = true;
			}
		});
	}

	void Scene::OnRuntimeStop()
	{
		for (auto& logicSystem : m_LogicSystems) {
			logicSystem->OnStop(this);
		}
		for (auto& physicsSystem : m_PhysicsSystems) {
			physicsSystem->OnStop(this);
		}
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_ViewportHeight = height;
		m_ViewportWidth = width;

		// Invalidate all UI transforms because viewport size changed
		m_Registry.view<RectTransformComponent>().each([&](auto entity, auto& rect) {
			rect.m_IsDirty = true;
		});

		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view) {
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (!cameraComponent.FixedAspectRatio) {
				cameraComponent.Camera.SetViewportSize(width, height);
			}
		}
	}

	void Scene::DuplicateEntity(Entity entity)
	{
		Entity newEntity = CreateEntity(entity.GetComponent<TagComponent>().Tag);

		CopyEntity(entity, newEntity);

		if (entity.HasComponent<RelationshipComponent>())
		{
			auto& rc = entity.GetComponent<RelationshipComponent>();
			if (rc.Parent != entt::null)
			{
				Entity parent = { rc.Parent, this };

				// 1. Configurem el component del fill
				auto& childRC = newEntity.GetOrAddComponent<RelationshipComponent>();
				childRC.Parent = rc.Parent;

				// 2. Configurem el component del pare
				auto& parentRC = parent.GetComponent<RelationshipComponent>(); // El pare ja té component segur

				// 3. Afegim al final de la llista de fills del pare
				if (parentRC.FirstChild == entt::null)
				{
					parentRC.FirstChild = (entt::entity)newEntity;
				}
				else
				{
					// Busquem l'últim germà
					entt::entity prevNode = parentRC.FirstChild;
					while (true)
					{
						Entity prevEntity{ prevNode, this };
						auto& prevRC = prevEntity.GetComponent<RelationshipComponent>();
						if (prevRC.NextSibling == entt::null)
						{
							prevRC.NextSibling = (entt::entity)newEntity;
							childRC.PrevSibling = prevNode;
							break;
						}
						prevNode = prevRC.NextSibling;
					}
				}
				parentRC.ChildrenCount++;
			}
		}

		std::function<void(Entity, Entity)> CopyChildrenRec = [&](Entity srcParent, Entity dstParent)
			{
				if (!srcParent.HasComponent<RelationshipComponent>())
					return;

				auto& srcRC = srcParent.GetComponent<RelationshipComponent>();
				entt::entity currentChildHandle = srcRC.FirstChild;

				while (currentChildHandle != entt::null)
				{
					Entity srcChild = { currentChildHandle, this };
					Entity dstChild = CreateEntity(srcChild.GetComponent<TagComponent>().Tag);

					CopyEntity(srcChild, dstChild);

					auto& childRC = dstChild.GetOrAddComponent<RelationshipComponent>();
					childRC.Parent = (entt::entity)dstParent;

					auto& parentRC = dstParent.GetOrAddComponent<RelationshipComponent>();

					if (parentRC.FirstChild == entt::null)
					{
						parentRC.FirstChild = (entt::entity)dstChild;
					}
					else
					{
						entt::entity prevNode = parentRC.FirstChild;
						while (true)
						{
							Entity prevEntity{ prevNode, this };
							auto& prevRC = prevEntity.GetComponent<RelationshipComponent>();
							if (prevRC.NextSibling == entt::null)
							{
								prevRC.NextSibling = (entt::entity)dstChild;
								childRC.PrevSibling = prevNode;
								break;
							}
							prevNode = prevRC.NextSibling;
						}
					}
					parentRC.ChildrenCount++;

					CopyChildrenRec(srcChild, dstChild);

					currentChildHandle = srcChild.GetComponent<RelationshipComponent>().NextSibling;
				}
			};

		CopyChildrenRec(entity, newEntity);
	}

	void Scene::CopyEntity(Entity src, Entity dst)
	{
		CopyComponentIfExists<TransformComponent>(dst, src);
		CopyComponentIfExists<SpriteRendererComponent>(dst, src);
		CopyComponentIfExists<CircleRendererComponent>(dst, src);
		CopyComponentIfExists<CameraComponent>(dst, src);
		CopyComponentIfExists<NativeScriptComponent>(dst, src);
		CopyComponentIfExists<Rigidbody2DComponent>(dst, src);
		CopyComponentIfExists<BoxCollider2DComponent>(dst, src);
		CopyComponentIfExists<CircleCollider2DComponent>(dst, src);
		CopyComponentIfExists<TextComponent>(dst, src);
		CopyComponentIfExists<AnimationComponent>(dst, src);
		CopyComponentIfExists<RectTransformComponent>(dst, src);
		CopyComponentIfExists<ButtonComponent>(dst, src);
	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view) {
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (cameraComponent.Primary) {
				return Entity{ entity, this };
			}
		}
		return Entity{};
	}

	void Scene::OnCameraComponentConstruct(entt::registry& registry, entt::entity entity)
	{
		auto& component = registry.get<CameraComponent>(entity);

		if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
			component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}

	Entity Scene::FindEntityByName(std::string_view name)
	{
		auto view = m_Registry.view<TagComponent>();
		for (auto entityID : view)
		{
			const auto& tc = view.get<TagComponent>(entityID);
			if (tc.Tag == name)
				return Entity{ entityID, this };
		}
		return {};
	}

	void Scene::OnRenderOverlay(const glm::mat4& viewProjection)
	{
		Renderer2D::BeginScene(viewProjection);
		
		auto viewbc = m_Registry.view<TransformComponent, BoxCollider2DComponent>();

		viewbc.each([&](auto entity, auto& tc, auto& bc2d)
			{
				glm::vec3 scale = tc.GetScale() * glm::vec3(bc2d.Size, 1.0f);

				glm::mat4 transform = glm::translate(glm::mat4(1.0f), tc.GetTranslation())
					* glm::rotate(glm::mat4(1.0f), tc.GetRotation().z, glm::vec3(0.0f, 0.0f, 1.0f))
					* glm::translate(glm::mat4(1.0f), glm::vec3(bc2d.Offset, 0.001f))
					* glm::scale(glm::mat4(1.0f), scale);

				Renderer2D::DrawRect(transform, glm::vec4(0, 1, 0, 1));
			});
		
		auto viewcc = m_Registry.view<TransformComponent, CircleCollider2DComponent>();

		viewcc.each([&](auto entity, auto& tc, auto& cc2d)
		{
			float scale = std::max(tc.GetScale().x, tc.GetScale().y) * cc2d.Radius * 2.0f;

			glm::mat4 transform = glm::translate(glm::mat4(1.0f), tc.GetTranslation())
				* glm::rotate(glm::mat4(1.0f), tc.GetRotation().z, glm::vec3(0.0f, 0.0f, 1.0f))
				* glm::translate(glm::mat4(1.0f), glm::vec3(cc2d.Offset, 0.001f))
				* glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, 1.0f));

			Renderer2D::DrawCircle(transform, glm::vec4(0, 1, 0, 1), 0.05f, 0.01f, (int)entity);
		});

		Renderer2D::EndScene();
	}

	void Scene::UpdateAnimation(Timestep ts)
	{
		// Collect entities into an indexable array so Dispatch can slice the work
		struct AnimEntry
		{
			AnimationComponent* Anim;
			SpriteRendererComponent* Sprite;
		};

		auto view = m_Registry.view<AnimationComponent, SpriteRendererComponent>();
		std::vector<AnimEntry> entries;
		entries.reserve(view.size_hint());

		view.each([&](auto entityID, auto& anim, auto& sprite)
		{
			entries.emplace_back(&anim, &sprite);
		});

		if (entries.empty()) return;

		// Phase 1 (single-thread): Rebuild the animation asset map if it was cleared.
		// This must be single-threaded because it can modify shared Ref<> resources.
		for (auto& entry : entries)
		{
			auto& anim = *entry.Anim;
			if (anim.Animations.empty() && !anim.Profiles.empty())
			{
				for (auto& profile : anim.Profiles)
				{
					if (profile.AtlasTexture)
					{
						int numCols = (int)(profile.AtlasTexture->GetWidth() / profile.TileSize.x);
						if (numCols <= 0) numCols = 1;
						int col = profile.StartFrame % numCols;
						int row = profile.StartFrame / numCols;
						float startX = (float)col * profile.TileSize.x;
						float startY = (float)row * profile.TileSize.y;

						Ref<Animation2D> animAsset = Animation2D::CreateFromAtlas(
							profile.AtlasTexture, profile.TileSize, { startX, startY },
							profile.FrameCount, profile.FramesPerRow, profile.FrameTime
						);
						anim.Animations[profile.Name] = animAsset;
					}
				}

				if (!anim.Animations.empty()) {
					auto it = anim.Animations.find(anim.CurrentStateName);
					if (it != anim.Animations.end()) {
						anim.CurrentAnimation = it->second;
					} else {
						anim.CurrentAnimation = anim.Animations.begin()->second;
						anim.CurrentStateName = anim.Animations.begin()->first;
					}
				}
			}
		}

		// Phase 2 (multi-thread): Advance the animation timer and update the frame.
		// Each entity owns its own data, so parallel writes are 100% safe.
		const uint32_t count = (uint32_t)entries.size();
		const uint32_t groupSize = 64; // Tune this based on entity counts

		auto stats = JobSystem::Dispatch(count, groupSize, [&entries, ts](uint32_t start, uint32_t end)
		{
			for (uint32_t i = start; i < end; i++)
			{
				auto& anim = *entries[i].Anim;
				auto& sprite = *entries[i].Sprite;

				if (!anim.CurrentAnimation) continue;

				if (anim.Playing) {
					anim.TimeAccumulator += ts;
					while (anim.TimeAccumulator >= anim.CurrentAnimation->GetFrameTime())
					{
						anim.TimeAccumulator -= anim.CurrentAnimation->GetFrameTime();
						anim.CurrentFrameIndex++;

						if (anim.CurrentFrameIndex >= anim.CurrentAnimation->GetFrameCount())
						{
							if (anim.Loop) anim.CurrentFrameIndex = 0;
							else {
								anim.CurrentFrameIndex = anim.CurrentAnimation->GetFrameCount() - 1;
								anim.Playing = false;
							}
						}
					}
					sprite.SubTexture = anim.CurrentAnimation->GetFrame(anim.CurrentFrameIndex);
				}
				else {
					if (anim.CurrentFrameIndex >= anim.CurrentAnimation->GetFrameCount()) {
						anim.CurrentFrameIndex = 0;
					}
					sprite.SubTexture = anim.CurrentAnimation->GetFrame(anim.CurrentFrameIndex);
				}
			}
		});

		// Only wait if jobs were actually dispatched to worker threads
		if (stats.GroupsDispatched > 0)
		{
			JobSystem::Wait();
		}
	}

	SceneStats Scene::GetStats() const
	{
		SceneStats stats;

		// Comptem totes les entitats vives
		stats.TotalEntities = (uint32_t)GetSizeOfAllEntities();

		// Comptem quants scripts natius s'estan actualitzant
		stats.ScriptUpdates = (uint32_t)m_Registry.view<NativeScriptComponent>().size();

		// Les partícules vives
		auto particlesSystem = GetSystem<ParticleSystem>();
		if (particlesSystem) {
			stats.ActiveParticles = (uint32_t)particlesSystem->GetActiveParticleCount();
		}

		return stats;
	}

	void Scene::OnRenderDebugOverlay()
	{
		if (!m_ShowDebugOverlay) return;

		Renderer2D::SetRecordStats(false);

		auto stats = Renderer2D::GetStats();
		auto sceneStats = GetStats();

		// Overlay virtual viewport (1080p height)
		float aspectRatio = (float)m_ViewportWidth / (float)m_ViewportHeight;
		float refHeight = 1080.0f;
		float refWidth = refHeight * aspectRatio;

		glm::mat4 projection = glm::ortho(0.0f, refWidth, 0.0f, refHeight, -1.0f, 1.0f);
		Renderer2D::BeginScene(projection);

		std::string debugStr = "FPS: " + std::to_string((int)Application::Get().GetAverageFPS()) + "\n";
		debugStr += "Renderer Stats:\n";
		debugStr += "  Draw Calls: " + std::to_string(stats.DrawCalls) + "\n";
		debugStr += "  Quads: " + std::to_string(stats.QuadCount) + "\n";
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

	void Scene::UpdateTweens(Timestep ts)
	{
		std::vector<entt::entity> toRemove;
		std::vector<entt::entity> toDestroy;

		auto view = m_Registry.view<TweenComponent>();
		for (auto entityID : view)
		{
			Entity entity = { entityID, this };
			auto& tc = entity.GetComponent<TweenComponent>();

			if (!tc.IsPlaying) continue;

			bool allFinished = true;
			for (auto& tween : tc.Tweens) 
			{
				if (tween.Finished) continue;

				tween.TimeElapsed += ts;
				float t = glm::clamp(tween.TimeElapsed / tween.Duration, 0.0f, 1.0f);
				float easedT = Easing::Interpolate(t, tween.Easing);

				glm::vec4 currentVal = glm::mix(tween.StartValue, tween.EndValue, easedT);

				bool needsInvalidation = false;
				switch (tween.Target)
				{
				case TweenTarget::Position:
					if (entity.HasComponent<TransformComponent>()) entity.GetComponent<TransformComponent>().SetTranslation(glm::vec3(currentVal));
					if (entity.HasComponent<RectTransformComponent>()) entity.GetComponent<RectTransformComponent>().SetPosition(glm::vec2(currentVal));
					needsInvalidation = true;
					break;
				case TweenTarget::Scale:
					if (entity.HasComponent<TransformComponent>()) entity.GetComponent<TransformComponent>().SetScale(glm::vec3(currentVal));
					if (entity.HasComponent<RectTransformComponent>()) entity.GetComponent<RectTransformComponent>().SetScale(glm::vec2(currentVal));
					needsInvalidation = true;
					break;
				case TweenTarget::Rotation:
					if (entity.HasComponent<TransformComponent>()) entity.GetComponent<TransformComponent>().SetRotation(glm::vec3(currentVal));
					if (entity.HasComponent<RectTransformComponent>()) entity.GetComponent<RectTransformComponent>().SetRotation(currentVal.x);
					needsInvalidation = true;
					break;
				case TweenTarget::Color:
					if (entity.HasComponent<SpriteRendererComponent>()) entity.GetComponent<SpriteRendererComponent>().Color = currentVal;
					else if (entity.HasComponent<CircleRendererComponent>()) entity.GetComponent<CircleRendererComponent>().Color = currentVal;
					else if (entity.HasComponent<TextComponent>()) entity.GetComponent<TextComponent>().Color = currentVal;
					break;
				}

				if (needsInvalidation)
					entity.InvalidateTransform();

				if (t >= 1.0f)
				{
					if (tween.PingPong)
					{
						tween.TimeElapsed = 0.0f;
						std::swap(tween.StartValue, tween.EndValue);
						allFinished = false;
					}
					else
					{
						tween.Finished = true;
					}
				}
				else
				{
					allFinished = false;
				}
			} 

			if (allFinished)
			{
				if (tc.OnComplete)
					tc.OnComplete(entity);

				if (tc.DestroyOnComplete)
					toDestroy.push_back(entityID);
				else
					toRemove.push_back(entityID);
			}
		}

		for (auto e : toRemove) m_Registry.remove<TweenComponent>(e);
		for (auto e : toDestroy) m_Registry.destroy(e);
	}
}