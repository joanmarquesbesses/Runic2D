#include "R2Dpch.h"
#include "Scene.h"

#include "Runic2D/Systems/System.h"
#include "Runic2D/Systems/ScriptingSystem.h"
#include "Runic2D/Systems/PhysicsSystem.h"
#include "Runic2D/Systems/TransformSystem.h"
#include "Runic2D/Systems/UISystem.h"
#include "Runic2D/Systems/Render2DSystem.h"
#include "Runic2D/Systems/ParticleSystem.h"
#include "Runic2D/Systems/Animation2DSystem.h"
#include "Runic2D/Systems/TweenSystem.h"
#include "Runic2D/Systems/DebugSystem.h"

#include "Entity.h"
#include "Components/CoreComponents.h"
#include "Components/RenderComponents.h"
#include "Components/MotionComponents.h"
#include "Components/PhysicsComponents.h"
#include "Components/ScriptingComponents.h"
#include "Components/UIComponents.h"
#include "Components/ComponentRegistry.h"

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
		// 2D Animation
		AddSystem(CreateRef<Animation2DSystem>(), { SystemPhase::PostUpdate });
		// Tween System
		AddSystem(CreateRef<TweenSystem>(), { SystemPhase::PostUpdate });
		// Debug System
		AddSystem(CreateRef<DebugSystem>(), { SystemPhase::Render });
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
		R2D_PROFILE_SCOPE("Scene: OnFixedUpdate");

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
		R2D_PROFILE_SCOPE("Scene: OnUpdate");

		for (auto& logicSystem : m_LogicSystems) {
			logicSystem->OnUpdate(ts, this);
		}
		for (auto& physicsSystem : m_PhysicsSystems) {
			physicsSystem->OnUpdate(ts, this);
		}
		for (auto& postUpdateSystem : m_PostUpdateSystems) {
			postUpdateSystem->OnUpdate(ts, this);
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
		R2D_PROFILE_SCOPE("Scene: OnRender");

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

		auto animation2DSystem = GetSystem<Animation2DSystem>();
		if (animation2DSystem) {
			animation2DSystem->OnUpdate(ts, this);
		}
		
		auto render2DSystem = GetSystem<Render2DSystem>();
		if (render2DSystem) {
			render2DSystem->SetCustomCamera(camera.GetViewProjection());
			render2DSystem->OnRender(this);
			render2DSystem->ClearCustomCamera();
		}

		auto uiSystem = GetSystem<UISystem>();
		if (uiSystem) {
			uiSystem->OnRender(this);
		}

		auto debugSystem = GetSystem<DebugSystem>();
		if (debugSystem) {
			debugSystem->SetCustomCamera(camera.GetViewProjection());
			debugSystem->OnRender(this);
			debugSystem->DrawCameraBounds(this);
			debugSystem->ClearCustomCamera();
		}
	}

	void Scene::OnRuntimeStart()
	{
		for (size_t i = 0; i < m_LogicSystems.size(); i++) {
			m_LogicSystems[i]->OnStart(this);
		}
		for (auto& physicsSystem : m_PhysicsSystems) {
			physicsSystem->OnStart(this);
		}
		for (auto& postUpdateSystem : m_PostUpdateSystems) {
			postUpdateSystem->OnStart(this);
		}
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
}