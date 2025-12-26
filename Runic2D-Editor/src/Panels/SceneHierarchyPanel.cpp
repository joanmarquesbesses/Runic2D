#include "SceneHierarchyPanel.h"

#include <Imgui/imgui.h>
#include <Imgui/imgui_internal.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Runic2D/Scene/Component.h"

#include "Runic2D/Renderer/Renderer2D.h"
#include "Runic2D/Project/Project.h"
#include "Runic2D/Scripting/ScriptEngine.h"

#include <cstring>

/* The Microsoft C++ compiler is non-compliant with the C++ standard and needs
 * the following definition to disable a security warning on std::strncpy().
 */
#ifdef _MSVC_LANG
#define _CRT_SECURE_NO_WARNINGS
#endif

namespace Runic2D
{

	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction, bool canBeDeleted = true)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

		if (entity.HasComponent<T>())
		{
			auto& component = entity.GetComponent<T>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;

			// Dibuixar el Header (la fletxa i el nom)
			ImGui::Separator();
			ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopFont();
			ImGui::PopStyleVar();

			// Botó de opcions (els tres puntets o "+") a la dreta
			bool removeComponent = false;
			if (canBeDeleted) {
				ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
				if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
				{
					ImGui::OpenPopup("ComponentSettings");
				}

				if (ImGui::BeginPopup("ComponentSettings"))
				{
					if (ImGui::MenuItem("Remove component"))
						removeComponent = true;

					ImGui::EndPopup();
				}
			}

			if (open)
			{
				uiFunction(component);
				ImGui::TreePop();
			}

			if (removeComponent && canBeDeleted)
				entity.RemoveComponent<T>();
		}
	}

	static bool DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		bool changed = false;

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });

		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

		if (ImGui::Button("X", buttonSize)) {
			values.x = resetValue;
			changed = true;
		}
		ImGui::PopStyleColor(3);

		ImGui::PopFont();

		ImGui::SameLine();
		if (ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f")) changed = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });

		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

		if (ImGui::Button("Y", buttonSize)) {
			values.y = resetValue;
			changed = true;
		}
		ImGui::PopStyleColor(3);

		ImGui::PopFont();

		ImGui::SameLine();
		if (ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f")) changed = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });

		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

		if (ImGui::Button("Z", buttonSize)) {
			values.z = resetValue;
			changed = true;
		}
		ImGui::PopStyleColor(3);

		ImGui::PopFont();

		ImGui::SameLine();
		if (ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f")) changed = true;
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);
		ImGui::PopID();

		return changed; // Retornem si hi ha hagut canvis
	}

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
	{
		SetContext(context);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
		m_SelectionContext = {};
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;

		ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

		bool hasChildren = false;
		if (entity.HasComponent<RelationshipComponent>())
			hasChildren = entity.GetComponent<RelationshipComponent>().ChildrenCount > 0;

		if (!hasChildren)
			flags |= ImGuiTreeNodeFlags_Leaf;

		if (m_SelectionContext == entity)
			ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

		// 1. Dibuixem el Node
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());

		if (m_SelectionContext == entity)
			ImGui::PopFont();

		if (ImGui::IsItemClicked())
		{
			m_SelectionContext = entity;
		}

		// --- CORRECCIÓ 1: DRAG & DROP IMMEDIATAMENT DESPRÉS DEL TREENODE ---

		// Drag Source (Origen)
		if (ImGui::BeginDragDropSource())
		{
			entt::entity entityID = (entt::entity)entity;
			ImGui::SetDragDropPayload("SCENE_HIERARCHY_ENTITY", &entityID, sizeof(entt::entity));
			ImGui::Text("%s", tag.c_str());
			ImGui::EndDragDropSource();
		}

		// Drag Target (Destí)
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_ENTITY"))
			{
				entt::entity payloadEntityID = *(const entt::entity*)payload->Data;
				Entity payloadEntity{ payloadEntityID, m_Context.get() };

				bool isDescendant = false;

				if (entity.HasComponent<RelationshipComponent>())
				{
					entt::entity parentCheck = entity.GetComponent<RelationshipComponent>().Parent;
					while (parentCheck != entt::null)
					{
						if (parentCheck == payloadEntityID)
						{
							isDescendant = true;
							break;
						}
						// Pugem un nivell més
						Entity parentEnt{ parentCheck, m_Context.get() };
						if (parentEnt.HasComponent<RelationshipComponent>())
							parentCheck = parentEnt.GetComponent<RelationshipComponent>().Parent;
						else
							parentCheck = entt::null;
					}
				}

				if (payloadEntity != entity && !isDescendant)
				{
					m_Context->ParentEntity(payloadEntity, entity);
				}
				else if (isDescendant)
				{
					R2D_CORE_WARN("Cannot parent an entity to its own descendant!");
				}
			}
			ImGui::EndDragDropTarget();
		}

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			bool hasParent = false;
			if (entity.HasComponent<RelationshipComponent>())
				if (entity.GetComponent<RelationshipComponent>().Parent != entt::null)
					hasParent = true;

			if (hasParent)
			{
				if (ImGui::MenuItem("Unparent"))
				{
					m_DeferredUnparentEntity = entity;
					ImGui::CloseCurrentPopup();
				}
				ImGui::Separator();
			}

			if (ImGui::MenuItem("Delete Entity"))
			{
				m_DeferredDeleteEntity = entity;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		if (opened)
		{
			if (hasChildren)
			{
				auto& rc = entity.GetComponent<RelationshipComponent>();
				entt::entity currentChild = rc.FirstChild;

				while (currentChild != entt::null)
				{
					Entity childEntity{ currentChild, m_Context.get() };
					DrawEntityNode(childEntity);

					if (childEntity.HasComponent<RelationshipComponent>())
						currentChild = childEntity.GetComponent<RelationshipComponent>().NextSibling;
					else
						currentChild = entt::null;
				}
			}
			ImGui::TreePop();
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		DrawComponent<TagComponent>("Tag", entity, [](auto& component)
			{
				auto& tag = component.Tag;
				char buffer[256];
				memset(buffer, 0, sizeof(buffer));
				std::strncpy(buffer, tag.c_str(), sizeof(buffer));

				if (ImGui::InputText("Tag", buffer, sizeof(buffer)))
				{
					tag = std::string(buffer);
				}
			}, false);

		DrawComponent<TransformComponent>("Transform", entity, [](auto& component)
			{
				if (DrawVec3Control("Translation", component.Translation))
					component.IsDirty = true;

				glm::vec3 rotation = glm::degrees(component.Rotation);
				if (DrawVec3Control("Rotation", rotation))
				{
					component.Rotation = glm::radians(rotation);
					component.IsDirty = true;
				}

				if (DrawVec3Control("Scale", component.Scale, 1.0f))
					component.IsDirty = true;
			}, false);


		DrawComponent<CameraComponent>("Camera", entity, [](auto& component)
			{
				auto& camera = component.Camera;

				ImGui::Checkbox("Primary", &component.Primary);

				// Tipus de Projecció (Selector)
				const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
				const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];

				if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
				{
					for (int i = 0; i < 2; i++)
					{
						bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
						if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
						{
							currentProjectionTypeString = projectionTypeStrings[i];
							camera.SetProjectionType((SceneCamera::ProjectionType)i);
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				// Paràmetres segons el tipus
				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
				{
					float orthoSize = camera.GetOrthographicSize();
					if (ImGui::DragFloat("Size", &orthoSize))
						camera.SetOrthographicSize(orthoSize);

					float orthoNear = camera.GetOrthographicNearClip();
					if (ImGui::DragFloat("Near Clip", &orthoNear))
						camera.SetOrthographicNearClip(orthoNear);

					float orthoFar = camera.GetOrthographicFarClip();
					if (ImGui::DragFloat("Far Clip", &orthoFar))
						camera.SetOrthographicFarClip(orthoFar);

					ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);
				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
				{
					float perspectiveVerticalFov = glm::degrees(camera.GetPerspectiveVerticalFOV());
					if (ImGui::DragFloat("Vertical FOV", &perspectiveVerticalFov))
						camera.SetPerspectiveVerticalFOV(glm::radians(perspectiveVerticalFov));

					float perspectiveNear = camera.GetPerspectiveNearClip();
					if (ImGui::DragFloat("Near Clip", &perspectiveNear))
						camera.SetPerspectiveNearClip(perspectiveNear);

					float perspectiveFar = camera.GetPerspectiveFarClip();
					if (ImGui::DragFloat("Far Clip", &perspectiveFar))
						camera.SetPerspectiveFarClip(perspectiveFar);
				}
			});


		DrawComponent<NativeScriptComponent>("Script", entity, [&](auto& component)
			{
				std::vector<std::string> scriptNames = ScriptEngine::GetAvailableScripts();

				std::string currentScript = component.ClassName;
				if (currentScript.empty()) currentScript = "None";


				if (ImGui::BeginCombo("Class", currentScript.c_str()))
				{
					bool isNoneSelected = (currentScript == "None");
					if (ImGui::Selectable("None", isNoneSelected))
					{
						// Si tries None, podríem voler esborrar el component o deixar-lo buit
						// De moment no fem res o pots posar ClassName = "None"
					}

					for (const auto& name : scriptNames)
					{
						bool isSelected = (currentScript == name);

						if (ImGui::Selectable(name.c_str(), isSelected))
						{
							ScriptEngine::BindScript(name, entity);
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			});

		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component)
			{
				auto& color = component.Color;
				ImGui::ColorEdit4("Color", glm::value_ptr(color));

				ImGui::Separator();

				ImGui::Text("Texture");
				Ref<Texture2D> textureToShow = component.Texture ? component.Texture : Renderer2D::GetWhiteTexture();
				ImGui::ImageButton("TexturePreview", (ImTextureID)textureToShow->GetRendererID(), ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const char* path = (const char*)payload->Data;
						std::filesystem::path texturePath = Project::GetAssetFileSystemPath(path);
						component.Texture = Texture2D::Create(texturePath.string());
					}
					ImGui::EndDragDropTarget();
				}

				if (component.Texture)
				{
					ImGui::SameLine();
					if (ImGui::Button("X"))
						component.Texture = nullptr;
				}

				ImGui::DragFloat("Tiling Factor", &component.TilingFactor, 0.1f, 0.0f, 100.0f);
			});

		DrawComponent<CircleRendererComponent>("Circle Renderer", entity, [](auto& component)
			{
				auto& color = component.Color;
				ImGui::ColorEdit4("Color", glm::value_ptr(color));
				ImGui::DragFloat("Thickness", &component.Thickness, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Fade", &component.Fade, 0.01f, 0.0f, 1.0f);
			});

		DrawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity, [](auto& component)
			{
				const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
				const char* currentBodyTypeString = bodyTypeStrings[(int)component.Type];

				if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
				{
					for (int i = 0; i < 3; i++)
					{
						bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
						if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
						{
							currentBodyTypeString = bodyTypeStrings[i];
							component.Type = (Rigidbody2DComponent::BodyType)i;
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				ImGui::Checkbox("Fixed Rotation", &component.FixedRotation);

				ImGui::DragFloat("Gravity Scale", &component.GravityScale, 0.1f, 0.0f, 10.0f);
			});

		DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [](auto& component)
			{
				ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
				ImGui::DragFloat2("Size", glm::value_ptr(component.Size));
				ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
				ImGui::Spacing();
				ImGui::Checkbox("Is Sensor", &component.IsSensor);
			});

		DrawComponent<CircleCollider2DComponent>("Circle Collider 2D", entity, [](auto& component)
			{
				ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
				ImGui::DragFloat("Radius", &component.Radius);
				ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
				ImGui::Spacing();
				ImGui::Checkbox("Is Sensor", &component.IsSensor);
			});

		// BOTÓ D'AFEGIR COMPONENT
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		float buttonWidth = 150.0f;
		float buttonHeight = 25.0f;
		float availableWidth = ImGui::GetContentRegionAvail().x;

		ImGui::SetCursorPosX((availableWidth - buttonWidth) * 0.5f);

		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

		if (ImGui::Button("Add Component", ImVec2{ buttonWidth, buttonHeight }))
		{
			ImGui::OpenPopup("AddComponent");
		}

		ImGui::PopFont();

		if (ImGui::BeginPopup("AddComponent"))
		{
			if (!entity.HasComponent<CameraComponent>())
			{
				if (ImGui::MenuItem("Camera"))
				{
					entity.AddComponent<CameraComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!entity.HasComponent<SpriteRendererComponent>())
			{
				if (ImGui::MenuItem("Sprite Renderer"))
				{
					entity.AddComponent<SpriteRendererComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!entity.HasComponent<CircleRendererComponent>())
			{
				if (ImGui::MenuItem("Circle Renderer"))
				{
					entity.AddComponent<CircleRendererComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!entity.HasComponent<NativeScriptComponent>())
			{
				if (ImGui::MenuItem("Native Script"))
				{
					entity.AddComponent<NativeScriptComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!m_SelectionContext.HasComponent<Rigidbody2DComponent>())
			{
				if (ImGui::MenuItem("Rigidbody 2D"))
				{
					m_SelectionContext.AddComponent<Rigidbody2DComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!m_SelectionContext.HasComponent<BoxCollider2DComponent>())
			{
				if (ImGui::MenuItem("Box Collider 2D"))
				{
					m_SelectionContext.AddComponent<BoxCollider2DComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!m_SelectionContext.HasComponent<CircleCollider2DComponent>())
			{
				if (ImGui::MenuItem("Circle Collider 2D"))
				{
					m_SelectionContext.AddComponent<CircleCollider2DComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::EndPopup();
		}

	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(200.0f, 200.0f));

		ImGui::Begin("Scene Hierarchy");

		if (m_Context)
		{
			auto view = m_Context->m_Registry.view<TagComponent>();

			for (auto entityID : view)
			{
				Entity entity{ entityID, m_Context.get() };

				bool isRootEntity = true;

				if (entity.HasComponent<RelationshipComponent>())
				{
					if (entity.GetComponent<RelationshipComponent>().Parent != entt::null)
						isRootEntity = false;
				}

				if (isRootEntity)
				{
					DrawEntityNode(entity);
				}
			}
		}

		// Deseleccionar si cliquem al buit
		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			m_SelectionContext = {};

		//right click on a empty space
		if(ImGui::BeginPopupContextWindow(0, 1 | ImGuiPopupFlags_NoOpenOverItems))
		{
			if(ImGui::MenuItem("Create Empty Entity"))
			{
				Entity newEntity = m_Context->CreateEntity("Empty Entity");
			}
			ImGui::EndPopup();
		}

		ImGui::End();

		ImGui::PopStyleVar();

		if (m_DeferredUnparentEntity)
		{
			m_Context->UnparentEntity(m_DeferredUnparentEntity);
			m_DeferredUnparentEntity = {};
		}

		if (m_DeferredDeleteEntity)
		{
			if (m_SelectionContext == m_DeferredDeleteEntity)
				m_SelectionContext = {};

			m_Context->DestroyEntity(m_DeferredDeleteEntity);
			m_DeferredDeleteEntity = {};
		}

		if (m_SelectionContext)
		{
			if (!m_Context->m_Registry.valid((entt::entity)m_SelectionContext))
				m_SelectionContext = {};
		}

		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(300.0f, 200.0f));
		ImGui::Begin("Properties");
		if (m_SelectionContext)
			DrawComponents(m_SelectionContext);
		ImGui::End();
		ImGui::PopStyleVar();
	}

	void SceneHierarchyPanel::SetSelectedEntity(Entity entity)
	{
		m_SelectionContext = entity;
	}

}