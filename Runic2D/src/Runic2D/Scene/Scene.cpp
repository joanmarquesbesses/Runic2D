#include "R2Dpch.h"
#include "Scene.h"

#include "Runic2D/Renderer/Renderer2D.h"
#include "Runic2D/Renderer/RenderCommand.h"
#include "Runic2D/Math/Math.h"

#include "Runic2D/Core/Application.h"
#include "Runic2D/Core/Input.h"
#include "Runic2D/Core/JobSystem.h"

#include "Entity.h"
#include "Component.h"
#include "ComponentRegistry.h"
#include "Tween.h"

namespace Runic2D {

	static b2BodyType Rigidbody2DTypeToBox2D(Rigidbody2DComponent::BodyType bodyType)
	{
		switch (bodyType)
		{
		case Rigidbody2DComponent::BodyType::Static:    return b2_staticBody;
		case Rigidbody2DComponent::BodyType::Dynamic:   return b2_dynamicBody;
		case Rigidbody2DComponent::BodyType::Kinematic: return b2_kinematicBody;
		}
		return b2_staticBody;
	}

	Scene::Scene()
	{
		m_Registry.on_construct<CameraComponent>().connect<&Scene::OnCameraComponentConstruct>(this);
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

		if (entity.HasComponent<NativeScriptComponent>())
		{
			auto& nsc = entity.GetComponent<NativeScriptComponent>();

			if (nsc.Instance)
			{
				nsc.Instance->OnDestroy();
				nsc.DestroyScript(&nsc);
			}
		}

		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

			if (B2_IS_NON_NULL(rb2d.RuntimeBody) && m_PhysicsWorld.index1 != 0)
			{
				b2DestroyBody(rb2d.RuntimeBody);
				rb2d.RuntimeBody = b2_nullBodyId;
			}
		}

		UnparentEntity(entity);

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
			UpdateScriptsFixed(ts);
			UpdatePhysics(ts);
		}
	}

	void Scene::OnUpdateRunTime(Timestep ts)
	{
		// Physics Interpolation
		if (!m_IsPaused && B2_IS_NON_NULL(m_PhysicsWorld))
		{
			float alpha = Application::Get().GetFixedUpdateAlpha();
			m_Registry.view<TransformComponent, Rigidbody2DComponent>().each([&](auto entity, auto& transform, auto& rb)
			{
				if (B2_IS_NON_NULL(rb.RuntimeBody))
				{
					b2Vec2 currentPos = b2Body_GetPosition(rb.RuntimeBody);
					float currentRot = b2Rot_GetAngle(b2Body_GetRotation(rb.RuntimeBody));

					transform.Translation.x = glm::mix(rb.PreviousTranslation.x, currentPos.x, alpha);
					transform.Translation.y = glm::mix(rb.PreviousTranslation.y, currentPos.y, alpha);
				
					transform.Rotation.z = glm::mix(rb.PreviousRotation, currentRot, alpha);
					
					transform.IsDirty = true;
				}
			});
		}

		UpdateScripts(ts);
		UpdateTweens(ts);

		if (!m_IsPaused)
		{
			m_ParticleSystem.OnUpdate(ts);
			UpdateAnimation(ts);
		}

		UpdateUIInteraction();

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

	void Scene::UpdateScripts(Timestep ts)
	{
		m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
		{
			//Script instantiation for created entities in runtime
			if (!nsc.Instance) {
				nsc.Instance = nsc.InstantiateScript();
				nsc.Instance->m_Entity = Entity{ entity, this };
				nsc.Instance->OnCreate();
			}

			//Update Script
			if (nsc.Instance)
			{
				if (!m_IsPaused || nsc.Instance->UpdateWhenPaused())
					nsc.Instance->OnUpdate(ts);
			}
		});
	}

	void Scene::UpdateScriptsFixed(Timestep ts)
	{
		m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
		{
			if (nsc.Instance)
			{
				if (!m_IsPaused || nsc.Instance->UpdateWhenPaused())
					nsc.Instance->OnFixedUpdate(ts);
			}
		});
	}

	void Scene::UpdatePhysics(Timestep ts)
	{
		//Physics Step
		if (B2_IS_NON_NULL(m_PhysicsWorld))
		{
			// Store previous state for interpolation before stepping the world
			auto rbView = m_Registry.view<Rigidbody2DComponent>();
			for (auto e : rbView)
			{
				auto& rb2d = rbView.get<Rigidbody2DComponent>(e);
				if (B2_IS_NON_NULL(rb2d.RuntimeBody) && rb2d.InterpolationInitialized)
				{
					b2Vec2 pos = b2Body_GetPosition(rb2d.RuntimeBody);
					rb2d.PreviousTranslation = { pos.x, pos.y };
					rb2d.PreviousRotation = b2Rot_GetAngle(b2Body_GetRotation(rb2d.RuntimeBody));
				}
			}

			b2World_Step(m_PhysicsWorld, ts, 4);

			for (auto e : rbView)
			{
				auto& rb2d = rbView.get<Rigidbody2DComponent>(e);

				if (B2_IS_NULL(rb2d.RuntimeBody))
					continue;

				b2BodyId bodyId = rb2d.RuntimeBody;
				b2Body_SetAwake(bodyId, true);
			}

			b2ContactEvents events = b2World_GetContactEvents(m_PhysicsWorld);

			for (int i = 0; i < events.beginCount; ++i)
			{
				b2ContactBeginTouchEvent* event = events.beginEvents + i;

				b2ShapeId shapeA = event->shapeIdA;
				b2ShapeId shapeB = event->shapeIdB;

				uint64_t uuidA = (uintptr_t)b2Shape_GetUserData(shapeA);
				uint64_t uuidB = (uintptr_t)b2Shape_GetUserData(shapeB);

				Entity entityA = GetEntityByUUID(uuidA);
				Entity entityB = GetEntityByUUID(uuidB);

				if (entityA && entityA.HasComponent<NativeScriptComponent>())
				{
					auto& script = entityA.GetComponent<NativeScriptComponent>();
					if (script.Instance) script.Instance->OnCollision(entityB);
				}

				if (entityB && entityB.HasComponent<NativeScriptComponent>())
				{
					auto& script = entityB.GetComponent<NativeScriptComponent>();
					if (script.Instance) script.Instance->OnCollision(entityA);
				}
			}

			b2SensorEvents sensorEvents = b2World_GetSensorEvents(m_PhysicsWorld);

			for (int i = 0; i < sensorEvents.beginCount; ++i)
			{
				b2SensorBeginTouchEvent* event = sensorEvents.beginEvents + i;

				b2ShapeId shapeA = event->sensorShapeId;
				b2ShapeId shapeB = event->visitorShapeId;

				uint64_t uuidA = (uintptr_t)b2Shape_GetUserData(shapeA);
				uint64_t uuidB = (uintptr_t)b2Shape_GetUserData(shapeB);

				Entity entityA = GetEntityByUUID(uuidA);
				Entity entityB = GetEntityByUUID(uuidB);

				if (entityA && entityA.HasComponent<NativeScriptComponent>())
				{
					auto& script = entityA.GetComponent<NativeScriptComponent>();
					if (script.Instance) script.Instance->OnSensor(entityB);
				}

				if (entityB && entityB.HasComponent<NativeScriptComponent>())
				{
					auto& script = entityB.GetComponent<NativeScriptComponent>();
					if (script.Instance) script.Instance->OnSensor(entityA);
				}
			}
		}
	}

	void Scene::OnRenderRuntime()
	{
		//Find Main Camera
		auto cameraEntity = GetPrimaryCameraEntity();
		if (!cameraEntity) return;

		auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
		auto& camTransform = cameraEntity.GetComponent<TransformComponent>();

		// Render Scene
		Renderer2D::BeginScene(camera, camTransform.GetTransform());

		auto view = m_Registry.view<TransformComponent, SpriteRendererComponent>(entt::exclude<RectTransformComponent>);

		view.each([&](auto entityID, auto& transform, auto& sprite)
			{
				Entity e{ entityID, this };
				glm::mat4 worldTransform = GetWorldTransform(transform, e);
				Renderer2D::DrawSprite(worldTransform, sprite, (int)entityID);
			});

		auto circleView = m_Registry.view<TransformComponent, CircleRendererComponent>(entt::exclude<RectTransformComponent>);

		circleView.each([&](auto entityID, auto& transform, auto& circle)
			{
				Entity e{ entityID, this };
				glm::mat4 worldTransform = GetWorldTransform(transform, e);
				Renderer2D::DrawCircle(worldTransform, circle.Color, circle.Thickness, circle.Fade, (int)entityID);
			});

		m_Registry.view<TransformComponent, TextComponent>(entt::exclude<RectTransformComponent>).each([&](auto entityID, auto& transform, auto& text)
			{
				if (!text.Visible)
					return;
				Entity e{ entityID, this };
				glm::mat4 worldTransform = GetWorldTransform(transform, e);
				Renderer2D::DrawString(text.GetText(), text.FontAsset, worldTransform, text.Color, text.Kerning, text.LineSpacing, (int)entityID, (int)text.TextAlignment);
			});

		m_ParticleSystem.OnRender();

		Renderer2D::EndScene();

		OnRenderDebugOverlay();
	}

	void Scene::OnRenderUI()
	{
		float aspectRatio = (float)m_ViewportWidth / (float)m_ViewportHeight;
		
		float refHeight = 1080.0f;
		float refWidth = refHeight * aspectRatio;
		
		glm::vec2 virtualViewportSize = { refWidth, refHeight };
		glm::vec2 virtualViewportPivot = { 0.0f, 0.0f }; // Bottom-left origin

		glm::mat4 projection = glm::ortho(0.0f, refWidth, 0.0f, refHeight, -1.0f, 1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		
		RenderCommand::ClearDepth();
		Renderer2D::BeginScene(projection * view);

		glm::mat4 globalTransform = glm::mat4(1.0f); // Identity, we work in virtual pixels!

		struct UIRenderCommand {
			int ZIndex;
			std::function<void()> RenderCall;
		};
		std::vector<UIRenderCommand> renderCommands;

		auto processEntity = [&](Entity e, const glm::mat4& parentWorldTransform, const glm::vec2& parentSize, const glm::vec2& parentPivot, auto& processEntityRef) -> void
		{
			if (!e.HasComponent<RectTransformComponent>()) return;

			auto& rectTransform = e.GetComponent<RectTransformComponent>();
			glm::mat4 worldTransform, meshTransform;
			if (rectTransform.m_IsDirty) {
				rectTransform.CalculateTransforms(parentWorldTransform, parentSize, parentPivot, worldTransform, meshTransform);
				rectTransform.WorldTransform = worldTransform;
				rectTransform.ComputedMeshTransform = meshTransform;
				rectTransform.m_IsDirty = false;
			} else {
				worldTransform = rectTransform.WorldTransform;
				meshTransform = rectTransform.ComputedMeshTransform;
			}

			entt::entity entityID = e;

			if (e.HasComponent<SpriteRendererComponent>()) {
				renderCommands.push_back({ rectTransform.ZIndex, [this, meshTransform, entityID]() { 
					auto& sprite = m_Registry.get<SpriteRendererComponent>(entityID);
					Renderer2D::DrawSprite(meshTransform, sprite, (int)(uint32_t)entityID); 
				} });
			}
			if (e.HasComponent<CircleRendererComponent>()) {
				renderCommands.push_back({ rectTransform.ZIndex, [this, meshTransform, entityID]() { 
					auto& circle = m_Registry.get<CircleRendererComponent>(entityID);
					Renderer2D::DrawCircle(meshTransform, circle.Color, circle.Thickness, circle.Fade, (int)(uint32_t)entityID); 
				} });
			}
			if (e.HasComponent<TextComponent>()) {
				renderCommands.push_back({ rectTransform.ZIndex, [this, worldTransform, rectTransform, entityID]() {
					auto& text = m_Registry.get<TextComponent>(entityID);

					float fontSize = rectTransform.Size.y;
					float textWidth = text.GetTextWidth();
					float actualTextWidth = textWidth * fontSize;

					float boxLeftX = -rectTransform.Size.x * rectTransform.Pivot.x;

					float offsetX = boxLeftX; 

					if (text.TextAlignment == TextComponent::Alignment::Center) {
						offsetX += (rectTransform.Size.x - actualTextWidth) * 0.5f;
					}
					else if (text.TextAlignment == TextComponent::Alignment::Right) {
						offsetX += (rectTransform.Size.x - actualTextWidth);
					}

					int numLines = 1;
					for (char c : text.GetText()) {
						if (c == '\n') numLines++;
					}

					float boxBottomY = -rectTransform.Size.y * rectTransform.Pivot.y;
					float extraHeight = (numLines - 1) * (fontSize + (text.LineSpacing * fontSize));
					float verticalShift = extraHeight * (1.0f - rectTransform.Pivot.y);
					float offsetY = boxBottomY + (fontSize * 0.25f) + verticalShift;

					glm::mat4 finalTextTransform = glm::translate(worldTransform, glm::vec3(offsetX, offsetY, 0.0f));
					finalTextTransform = glm::scale(finalTextTransform, glm::vec3(fontSize, fontSize, 1.0f));

					Renderer2D::DrawString(text.GetText(), text.FontAsset, finalTextTransform, text.Color, text.Kerning, text.LineSpacing, (int)(uint32_t)entityID, (int)text.TextAlignment);
				} });
			}

			if (e.HasComponent<RelationshipComponent>()) {
				auto& relationship = e.GetComponent<RelationshipComponent>();
				entt::entity currentChild = relationship.FirstChild;
				while (currentChild != entt::null) {
					Entity childEntity{ currentChild, this };
					processEntityRef(childEntity, worldTransform, rectTransform.Size, rectTransform.Pivot, processEntityRef);
					currentChild = childEntity.GetComponent<RelationshipComponent>().NextSibling;
				}
			}
		};

		m_Registry.view<RectTransformComponent>().each([&](auto entityID, auto& rect)
		{
			Entity e{ entityID, this };
			bool isRoot = true;
			if (e.HasComponent<RelationshipComponent>()) {
				auto parent = e.GetComponent<RelationshipComponent>().Parent;
				if (parent != entt::null) {
					Entity parentEntity{ parent, this };
					if (parentEntity.HasComponent<RectTransformComponent>()) {
						isRoot = false;
					}
				}
			}
			
			if (isRoot) {
				processEntity(e, globalTransform, virtualViewportSize, virtualViewportPivot, processEntity);
			}
		});

		std::sort(renderCommands.begin(), renderCommands.end(), [](const UIRenderCommand& a, const UIRenderCommand& b) {
			return a.ZIndex < b.ZIndex;
		});

		int lastZIndex = INT_MIN;
		for (const auto& cmd : renderCommands) {
			if (cmd.ZIndex != lastZIndex && lastZIndex != INT_MIN) {
				Renderer2D::NextBatch(Renderer2D::FlushReason::UIZIndexChange);
			}
			cmd.RenderCall();
			lastZIndex = cmd.ZIndex;
		}

		Renderer2D::EndScene();

		OnRenderDebugOverlay();
	}

	void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
	{
		UpdateAnimation(ts);
		Renderer2D::BeginScene(camera);

		auto view = m_Registry.view<TransformComponent, SpriteRendererComponent>(entt::exclude<RectTransformComponent>);

		view.each([&](auto entityID, auto& transform, auto& sprite)
		{
			Entity e{ entityID, this };
			glm::mat4 worldTransform = GetWorldTransform(transform, e);
			Renderer2D::DrawSprite(worldTransform, sprite, (int)entityID);
		});

		auto circleView = m_Registry.view<TransformComponent, CircleRendererComponent>(entt::exclude<RectTransformComponent>);

		circleView.each([&](auto entityID, auto& transform, auto& circle)
		{
			Entity e{ entityID, this };
			glm::mat4 worldTransform = GetWorldTransform(transform, e);
			Renderer2D::DrawCircle(worldTransform, circle.Color, circle.Thickness, circle.Fade, (int)entityID);
		});

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

		m_Registry.view<TransformComponent, TextComponent>(entt::exclude<RectTransformComponent>).each([&](auto entityID, auto& transform, auto& text)
		{
			Entity e{ entityID, this };
			glm::mat4 worldTransform = GetWorldTransform(transform, e);
			Renderer2D::DrawString(text.GetText(), text.FontAsset, worldTransform, text.Color, text.Kerning, text.LineSpacing, (int)entityID, (int)text.TextAlignment);
		});

		Renderer2D::EndScene();

		OnRenderUI();
	}

	void Scene::UpdateUIInteraction()
	{
		glm::vec2 mouseUI = GetMousePositionInUISpace();

		bool mouseDown = Input::IsMouseButtonPressed(MouseButton::Left);

		auto view = m_Registry.view<ButtonComponent, RectTransformComponent>();

		view.each([&](entt::entity e, ButtonComponent& btn, RectTransformComponent& rect)
		{
			glm::mat4 inverseMesh = glm::inverse(rect.ComputedMeshTransform);
			glm::vec4 localMouse = inverseMesh * glm::vec4(mouseUI.x, mouseUI.y, 0.0f, 1.0f);

			bool hovered = (localMouse.x >= -0.5f && localMouse.x <= 0.5f &&
				localMouse.y >= -0.5f && localMouse.y <= 0.5f);

			ButtonComponent::State prevState = btn.CurrentState;

			if (!hovered)
			{
				if (prevState != ButtonComponent::State::Normal && btn.OnUnhover)
					btn.OnUnhover();

				btn.CurrentState = ButtonComponent::State::Normal;
			}
			else if (mouseDown)
			{
				if (prevState == ButtonComponent::State::Normal && btn.OnHover)
					btn.OnHover();

				btn.CurrentState = ButtonComponent::State::Pressed;
			}
			else
			{
				if (prevState == ButtonComponent::State::Normal && btn.OnHover)
					btn.OnHover();

				if (prevState == ButtonComponent::State::Pressed && btn.OnClick)
					btn.OnClick();

				btn.CurrentState = ButtonComponent::State::Hovered;
			}

			if (btn.CurrentState != prevState)
			{
				glm::vec4 targetColor = btn.NormalColor;
				if (btn.CurrentState == ButtonComponent::State::Hovered) targetColor = btn.HoveredColor;
				if (btn.CurrentState == ButtonComponent::State::Pressed) targetColor = btn.PressedColor;

				if (m_Registry.all_of<SpriteRendererComponent>(e))
				{
					Tween::ClearTarget({ e, this }, TweenTarget::Color);
					Tween::To({ e, this }, TweenTarget::Color, targetColor, 0.15f, EaseType::EaseOutQuad);
				}
			}
		});
	}

	glm::vec2 Scene::GetMousePositionInUISpace()
	{
		float W = (float)m_ViewportWidth;
		float H = (float)m_ViewportHeight;
		if (W <= 0 || H <= 0) return { -1.0f, -1.0f };

		// Posicio del mouse relativa al viewport (l'Editor configura m_ViewportBoundsMin)
		glm::vec2 mouse = Input::GetMousePosition();
		mouse.x -= m_ViewportBoundsMin.x;
		mouse.y -= m_ViewportBoundsMin.y;

		// Convertim a NDC [-1, 1]
		float ndcX = (2.0f * mouse.x) / W - 1.0f;
		float ndcY = 1.0f - (2.0f * mouse.y) / H;

		// Projeccio UI (mateixa que a OnRenderUI)
		float aspectRatio = W / H;
		float refHeight = 1080.0f;
		float refWidth = refHeight * aspectRatio;

		// NDC -> coordenades UI virtuals
		glm::mat4 projection = glm::ortho(0.0f, refWidth, 0.0f, refHeight, -1.0f, 1.0f);
		glm::vec4 worldPos = glm::inverse(projection) * glm::vec4(ndcX, ndcY, 0.0f, 1.0f);

		return { worldPos.x, worldPos.y };
	}

	void Scene::OnRuntimeStart()
	{
		b2WorldDef worldDef = b2DefaultWorldDef();
		worldDef.gravity = { 0.0f, -9.8f }; 
		m_PhysicsWorld = b2CreateWorld(&worldDef);

		auto view = m_Registry.view<Rigidbody2DComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
			InstantiatePhysics(entity);
		}

		m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
		{
			//Script instantiation
			if (!nsc.Instance) {
				nsc.Instance = nsc.InstantiateScript();
				nsc.Instance->m_Entity = Entity{ entity, this };
				nsc.Instance->OnCreate();
			}
		});

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
		if (B2_IS_NON_NULL(m_PhysicsWorld))
		{
			b2DestroyWorld(m_PhysicsWorld);
			m_PhysicsWorld = b2_nullWorldId;
		}

		auto view = m_Registry.view<Rigidbody2DComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
			rb2d.RuntimeBody = b2_nullBodyId;

			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
				bc2d.RuntimeShape = b2_nullShapeId;
			}

			if (entity.HasComponent<CircleCollider2DComponent>())
			{
				auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();
				cc2d.RuntimeShape = b2_nullShapeId;
			}
		}

		//Destroy script instances
		m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
		{
			if (nsc.Instance)
			{
				nsc.Instance->OnDestroy();
				nsc.DestroyScript(&nsc);  
				nsc.Instance = nullptr;    
			}
		});
	}

	void Scene::InstantiatePhysics(Entity entity)
	{
		auto& transform = entity.GetComponent<TransformComponent>();
		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

		glm::mat4 wolrdTransform = GetWorldTransform(entity);

		b2BodyDef bodyDef = b2DefaultBodyDef();
		bodyDef.type = Rigidbody2DTypeToBox2D(rb2d.Type);
		bodyDef.position = { wolrdTransform[3].x, wolrdTransform[3].y };
		bodyDef.rotation = b2MakeRot(transform.Rotation.z);
		bodyDef.enableSleep = true;
		bodyDef.motionLocks.angularZ = rb2d.FixedRotation;
		bodyDef.gravityScale = rb2d.GravityScale;

		b2BodyId bodyId = b2CreateBody(m_PhysicsWorld, &bodyDef);
		rb2d.RuntimeBody = bodyId;

		b2Vec2 spawnPos = b2Body_GetPosition(bodyId);
		rb2d.PreviousTranslation = { spawnPos.x, spawnPos.y };
		rb2d.PreviousRotation = b2Rot_GetAngle(b2Body_GetRotation(bodyId));
		rb2d.InterpolationInitialized = true;

		if (entity.HasComponent<BoxCollider2DComponent>())
		{
			auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

			b2ShapeDef shapeDef = b2DefaultShapeDef();
			shapeDef.density = bc2d.Density;

			shapeDef.userData = (void*)(uintptr_t)entity.GetUUID();
			shapeDef.isSensor = bc2d.IsSensor;
			shapeDef.filter.categoryBits = bc2d.CategoryBits;
			shapeDef.filter.maskBits = bc2d.MaskBits;
			shapeDef.filter.groupIndex = bc2d.GroupIndex;
			shapeDef.enableSensorEvents = bc2d.EnableSensorEvents;
			shapeDef.enableContactEvents = bc2d.EnableContactEvents;

			float hx = std::abs(bc2d.Size.x * transform.Scale.x) * 0.5f;
			float hy = std::abs(bc2d.Size.y * transform.Scale.y) * 0.5f;

			b2Polygon boxPolygon = b2MakeOffsetBox(hx, hy, { bc2d.Offset.x, bc2d.Offset.y }, b2MakeRot(0.0f));

			b2ShapeId shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &boxPolygon);
			b2Shape_SetFriction(shapeId, bc2d.Friction);
			b2Shape_SetRestitution(shapeId, bc2d.Restitution);
			bc2d.RuntimeShape = shapeId;
		}

		if (entity.HasComponent<CircleCollider2DComponent>())
		{
			auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();

			b2ShapeDef shapeDef = b2DefaultShapeDef();
			shapeDef.density = cc2d.Density;

			shapeDef.userData = (void*)(uintptr_t)entity.GetUUID();
			shapeDef.isSensor = cc2d.IsSensor;
			shapeDef.filter.categoryBits = cc2d.CategoryBits;
			shapeDef.filter.maskBits = cc2d.MaskBits;
			shapeDef.filter.groupIndex = cc2d.GroupIndex;
			shapeDef.enableSensorEvents = cc2d.EnableSensorEvents;
			shapeDef.enableContactEvents = cc2d.EnableContactEvents;

			float maxScale = std::max(transform.Scale.x, transform.Scale.y);
			float radius = cc2d.Radius * maxScale;

			b2Circle circle;
			circle.center = { cc2d.Offset.x, cc2d.Offset.y };
			circle.radius = radius;

			b2ShapeId shapeId = b2CreateCircleShape(bodyId, &shapeDef, &circle);
			b2Shape_SetFriction(shapeId, cc2d.Friction);
			b2Shape_SetRestitution(shapeId, cc2d.Restitution);
			cc2d.RuntimeShape = shapeId;
		}

		if (rb2d.Type == Rigidbody2DComponent::BodyType::Dynamic)
		{
			b2Body_ApplyMassFromShapes(bodyId);
			b2Body_SetAwake(bodyId, true);
		}
	}

	void Scene::UpdateEntityColliders(Entity entity)
	{
		if (!entity.HasComponent<Rigidbody2DComponent>()) return;

		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		auto& transform = entity.GetComponent<TransformComponent>();

		if (B2_IS_NULL(rb2d.RuntimeBody)) return;

		b2BodyId bodyId = rb2d.RuntimeBody;

		if (entity.HasComponent<BoxCollider2DComponent>())
		{
			auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
			if (B2_IS_NON_NULL(bc2d.RuntimeShape))
			{
				b2DestroyShape(bc2d.RuntimeShape, false);
			}

			b2ShapeDef shapeDef = b2DefaultShapeDef();
			shapeDef.density = bc2d.Density;
			shapeDef.userData = (void*)(uintptr_t)entity.GetUUID();
			shapeDef.filter.categoryBits = bc2d.CategoryBits; 
			shapeDef.filter.maskBits = bc2d.MaskBits;         
			shapeDef.filter.groupIndex = bc2d.GroupIndex;
			shapeDef.isSensor = bc2d.IsSensor;
			shapeDef.enableSensorEvents = bc2d.EnableSensorEvents;
			shapeDef.enableContactEvents = bc2d.EnableContactEvents;

			float hx = std::abs(bc2d.Size.x * transform.Scale.x) * 0.5f;
			float hy = std::abs(bc2d.Size.y * transform.Scale.y) * 0.5f;

			b2Polygon boxPolygon = b2MakeOffsetBox(hx, hy, { bc2d.Offset.x, bc2d.Offset.y }, b2MakeRot(0.0f));

			b2ShapeId shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &boxPolygon);

			b2Shape_SetFriction(shapeId, bc2d.Friction);
			b2Shape_SetRestitution(shapeId, bc2d.Restitution);

			bc2d.RuntimeShape = shapeId;
		}

		if (entity.HasComponent<CircleCollider2DComponent>())
		{
			auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();

			if (B2_IS_NON_NULL(cc2d.RuntimeShape))
			{
				b2DestroyShape(cc2d.RuntimeShape, false);
			}

			b2ShapeDef shapeDef = b2DefaultShapeDef();
			shapeDef.density = cc2d.Density;
			shapeDef.userData = (void*)(uintptr_t)entity.GetUUID();
			shapeDef.filter.categoryBits = cc2d.CategoryBits;
			shapeDef.filter.maskBits = cc2d.MaskBits;
			shapeDef.filter.groupIndex = cc2d.GroupIndex;
			shapeDef.isSensor = cc2d.IsSensor;
			shapeDef.enableSensorEvents = cc2d.EnableSensorEvents;
			shapeDef.enableContactEvents = cc2d.EnableContactEvents;

			float maxScale = std::max(std::abs(transform.Scale.x), std::abs(transform.Scale.y));
			float radius = cc2d.Radius * maxScale;

			b2Circle circle;
			circle.center = { cc2d.Offset.x, cc2d.Offset.y };
			circle.radius = radius;

			b2ShapeId shapeId = b2CreateCircleShape(bodyId, &shapeDef, &circle);

			b2Shape_SetFriction(shapeId, cc2d.Friction);
			b2Shape_SetRestitution(shapeId, cc2d.Restitution);

			cc2d.RuntimeShape = shapeId;
		}

		if (rb2d.Type == Rigidbody2DComponent::BodyType::Dynamic)
		{
			b2Body_ApplyMassFromShapes(bodyId);
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

	void Scene::SetCollisionEnabled(Entity entity, bool enabled)
	{
		if (!entity.HasComponent<BoxCollider2DComponent>()) return;

		auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

		if (b2Shape_IsValid(bc2d.RuntimeShape))
		{
			b2Filter filter = b2Shape_GetFilter(bc2d.RuntimeShape);

			filter.maskBits = enabled ? 0xFFFFFFFF : 0x00000000;

			b2Shape_SetFilter(bc2d.RuntimeShape, filter);
		}
	}

	void Scene::OnCameraComponentConstruct(entt::registry& registry, entt::entity entity)
	{
		auto& component = registry.get<CameraComponent>(entity);

		if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
			component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}

	void Scene::ParentEntity(Entity entity, Entity parent)
	{
		if (entity == parent) return;

		glm::mat4 oldWorldTransform = GetWorldTransform(entity);

		UnparentEntity(entity);

		auto& childRC = entity.GetComponent<RelationshipComponent>();
		childRC.Parent = (entt::entity)parent;

		auto& parentRC = parent.GetComponent<RelationshipComponent>();

		if (parentRC.FirstChild == entt::null)
		{
			parentRC.FirstChild = (entt::entity)entity;
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
					prevRC.NextSibling = (entt::entity)entity;
					childRC.PrevSibling = prevNode;
					break;
				}
				prevNode = prevRC.NextSibling;
			}
		}

		parentRC.ChildrenCount++;

		glm::mat4 parentWorldTransform = GetWorldTransform(parent);
		glm::mat4 newLocalTransform = glm::inverse(parentWorldTransform) * oldWorldTransform;

		auto& tc = entity.GetComponent<TransformComponent>();
		Math::DecomposeTransform(newLocalTransform, tc.Translation, tc.Rotation, tc.Scale);
		InvalidateTransform(entity);
	}

	void Scene::UnparentEntity(Entity entity, bool convertToWorldSpace)
	{
		auto& childRC = entity.GetComponent<RelationshipComponent>();
		if (childRC.Parent == entt::null)
			return;

		glm::mat4 worldTransform = glm::mat4(1.0f);
		if (convertToWorldSpace) {
			worldTransform = GetWorldTransform(entity);
		}

		Entity parent{ childRC.Parent, this };
		auto& parentRC = parent.GetComponent<RelationshipComponent>();

		if (childRC.PrevSibling != entt::null)
		{
			Entity prevEntity{ childRC.PrevSibling, this };
			prevEntity.GetComponent<RelationshipComponent>().NextSibling = childRC.NextSibling;
		}

		if (childRC.NextSibling != entt::null)
		{
			Entity nextEntity{ childRC.NextSibling, this };
			nextEntity.GetComponent<RelationshipComponent>().PrevSibling = childRC.PrevSibling;
		}

		if (parentRC.FirstChild == (entt::entity)entity)
		{
			parentRC.FirstChild = childRC.NextSibling;
		}

		parentRC.ChildrenCount--;

		childRC.Parent = entt::null;
		childRC.NextSibling = entt::null;
		childRC.PrevSibling = entt::null;

		if (convertToWorldSpace) {
			auto& tc = entity.GetComponent<TransformComponent>();
			Math::DecomposeTransform(worldTransform, tc.Translation, tc.Rotation, tc.Scale);
			InvalidateTransform(entity);
		}
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

	glm::mat4 Scene::GetWorldTransform(Entity entity)
	{
		glm::mat4 transform = entity.GetComponent<TransformComponent>().GetTransform();

		if (entity.HasComponent<RelationshipComponent>())
		{
			entt::entity parentID = entity.GetComponent<RelationshipComponent>().Parent;
			if (parentID != entt::null)
			{
				Entity parent{ parentID, this };
				transform = GetWorldTransform(parent) * transform;
			}
		}

		return transform;
	}

	glm::mat4 Scene::GetWorldTransform(const TransformComponent& transform, Entity entity)
	{
		if (!transform.IsDirty)
			return transform.WorldTransform;

		glm::mat4 worldTransform = transform.GetTransform();

		if (entity.HasComponent<RelationshipComponent>())
		{
			entt::entity parentID = entity.GetComponent<RelationshipComponent>().Parent;
			if (parentID != entt::null)
			{
				Entity parent{ parentID, this };
				worldTransform = GetWorldTransform(parent) * worldTransform;
			}
		}

		const_cast<TransformComponent&>(transform).WorldTransform = worldTransform;
		const_cast<TransformComponent&>(transform).IsDirty = false;
		return worldTransform;
	}

	void Scene::OnRenderOverlay(const glm::mat4& viewProjection)
	{
		Renderer2D::BeginScene(viewProjection);
		
		auto viewbc = m_Registry.view<TransformComponent, BoxCollider2DComponent>();

		viewbc.each([&](auto entity, auto& tc, auto& bc2d)
			{
				glm::vec3 scale = tc.Scale * glm::vec3(bc2d.Size, 1.0f);

				glm::mat4 transform = glm::translate(glm::mat4(1.0f), tc.Translation)
					* glm::rotate(glm::mat4(1.0f), tc.Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f))
					* glm::translate(glm::mat4(1.0f), glm::vec3(bc2d.Offset, 0.001f))
					* glm::scale(glm::mat4(1.0f), scale);

				Renderer2D::DrawRect(transform, glm::vec4(0, 1, 0, 1));
			});
		
		auto viewcc = m_Registry.view<TransformComponent, CircleCollider2DComponent>();

		viewcc.each([&](auto entity, auto& tc, auto& cc2d)
			{
				float scale = std::max(tc.Scale.x, tc.Scale.y) * cc2d.Radius * 2.0f;

				glm::mat4 transform = glm::translate(glm::mat4(1.0f), tc.Translation)
					* glm::rotate(glm::mat4(1.0f), tc.Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f))
					* glm::translate(glm::mat4(1.0f), glm::vec3(cc2d.Offset, 0.001f))
					* glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, 1.0f));

				Renderer2D::DrawCircle(transform, glm::vec4(0, 1, 0, 1), 0.05f, 0.01f, (int)entity);
			});

		Renderer2D::EndScene();

		OnRenderDebugOverlay();
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

#ifndef R2D_DIST
		// Log every time the number of groups changes (including inline <-> threaded transitions)
		static uint32_t s_LastGroupCount = 0;
		if (stats.GroupsDispatched != s_LastGroupCount)
		{
			s_LastGroupCount = stats.GroupsDispatched;
			if (stats.GroupsDispatched == 0)
				R2D_CORE_INFO("[Anim] Mode: INLINE ({0} entities, groupSize={1})", count, groupSize);
			else
				R2D_CORE_INFO("[Anim] Mode: MULTITHREADED | groups={0} | entities={1} | groupSize={2}", stats.GroupsDispatched, count, groupSize);
		}
#endif
	}

	SceneStats Scene::GetStats() const
	{
		SceneStats stats;

		// Comptem totes les entitats vives
		stats.TotalEntities = (uint32_t)GetSizeOfAllEntities();

		// Comptem quants scripts natius s'estan actualitzant
		stats.ScriptUpdates = (uint32_t)m_Registry.view<NativeScriptComponent>().size();

		// Les partícules vives
		stats.ActiveParticles = (uint32_t)m_ParticleSystem.GetActiveParticleCount();

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

		std::string debugStr = "Renderer Stats:\n";
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
					InvalidateTransform(entity);

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


	void Scene::InvalidateTransform(Entity entity)
	{
		if (entity.HasComponent<TransformComponent>())
			entity.GetComponent<TransformComponent>().IsDirty = true;
		
		if (entity.HasComponent<RectTransformComponent>())
			entity.GetComponent<RectTransformComponent>().m_IsDirty = true;

		if (entity.HasComponent<RelationshipComponent>())
		{
			auto& rel = entity.GetComponent<RelationshipComponent>();
			entt::entity childID = rel.FirstChild;
			while (childID != entt::null)
			{
				InvalidateTransform({ childID, this });
				childID = m_Registry.get<RelationshipComponent>(childID).NextSibling;
			}
		}
	}
}