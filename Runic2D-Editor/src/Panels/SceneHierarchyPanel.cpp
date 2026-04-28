#include "SceneHierarchyPanel.h"

#include <Imgui/imgui.h>
#include <Imgui/imgui_internal.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Runic2D/Scene/Component.h"
#include "Runic2D/Scene/ComponentRegistry.h"

#include "Runic2D/Renderer/Renderer2D.h"
#include "Runic2D/Project/Project.h"
#include "Runic2D/Scripting/ScriptEngine.h"
#include "Runic2D/Assets/ResourceManager.h"

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
	static void DrawNativeComponent(const std::string& name, Entity entity, UIFunction uiFunction, bool canBeDeleted = true)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

		if (entity.HasComponent<T>())
		{
			auto& component = entity.GetComponent<T>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushID((void*)typeid(T).hash_code());

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

			ImGui::PopID();
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
		DrawNativeComponent<TagComponent>("Tag", entity, [](auto& component)
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

		DrawNativeComponent<TransformComponent>("Transform", entity, [](auto& component)
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


		for (const auto& desc : ComponentRegistry::GetAll())
			{
				if (desc.HasOnEntity(entity))
				{
					// Utilitzem un estil similar al template DrawComponent per coherència visual
					ImGui::PushID(desc.Name.c_str());

					ImGui::Separator();
					float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
					ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

					const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

					ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
					bool open = ImGui::TreeNodeEx((void*)desc.Name.c_str(), treeNodeFlags, desc.Name.c_str());
					ImGui::PopFont();

					// Botó per eliminar el component (el "+" o "X" a la dreta)
					ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
					if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
					{
						ImGui::OpenPopup("ComponentSettings");
					}

					bool removeComponent = false;
					if (ImGui::BeginPopup("ComponentSettings"))
					{
						if (ImGui::MenuItem("Remove component"))
							removeComponent = true;
						ImGui::EndPopup();
					}

					if (open)
					{
						if (desc.DrawImGui) 
							desc.DrawImGui(entity);
						ImGui::TreePop();
					}

					if (removeComponent)
						desc.RemoveFromEntity(entity);

					ImGui::PopID();
				}
			}

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
		
		ImVec2 buttonMin = ImGui::GetItemRectMin();
		ImVec2 buttonMax = ImGui::GetItemRectMax();
		ImGui::SetNextWindowPos(ImVec2((buttonMin.x + buttonMax.x) * 0.5f, buttonMax.y), ImGuiCond_Appearing, ImVec2(0.5f, 0.0f));

  		ImGui::PopFont();

		if (ImGui::BeginPopup("AddComponent"))
		{
			struct CategoryData {
				std::string Name;
				std::vector<const ComponentDescriptor*> Components;
			};
			std::vector<CategoryData> engineCategories;
			std::vector<CategoryData> gameCategories;

			for (const auto& desc : ComponentRegistry::GetAll()) {
				if (desc.HasOnEntity(entity)) continue;
				
				auto& targetList = desc.IsEngineComponent ? engineCategories : gameCategories;
				bool found = false;
				for (auto& cat : targetList) {
					if (cat.Name == desc.Category) {
						cat.Components.push_back(&desc);
						found = true;
						break;
					}
				}
				if (!found) {
					targetList.push_back({desc.Category, {&desc}});
				}
			}

			auto windowWidth = ImGui::GetWindowWidth();

			if (!engineCategories.empty()) {
				ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
				auto textWidth = ImGui::CalcTextSize("-- Engine Components --").x;
				float posX = (windowWidth - textWidth) * 0.5f;
				if (posX < 0) posX = 0;
				ImGui::SetCursorPosX(posX);
				ImGui::TextDisabled("-- Engine Components --");
				ImGui::PopFont();

				for (const auto& cat : engineCategories) {
					if (!cat.Name.empty()) {
						ImGui::Spacing();
						auto catWidth = ImGui::CalcTextSize(cat.Name.c_str()).x;
						posX = (windowWidth - catWidth) * 0.5f;
						if (posX < 0) posX = 0;
						ImGui::SetCursorPosX(posX);
						ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), cat.Name.c_str());
					}

					for (const auto* desc : cat.Components) {
						ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));
						if (ImGui::Selectable(desc->Name.c_str())) {
							desc->AddToEntity(entity);
							ImGui::CloseCurrentPopup();
						}
						ImGui::PopStyleVar();
					}
				}
			}

			if (!gameCategories.empty()) {
				if (!engineCategories.empty()) ImGui::Separator();
				ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
				auto textWidth = ImGui::CalcTextSize("-- Game Components --").x;
				float posX = (windowWidth - textWidth) * 0.5f;
				if (posX < 0) posX = 0;
				ImGui::SetCursorPosX(posX);
				ImGui::TextDisabled("-- Game Components --");
				ImGui::PopFont();

				for (const auto& cat : gameCategories) {
					if (!cat.Name.empty()) {
						ImGui::Spacing();
						auto catWidth = ImGui::CalcTextSize(cat.Name.c_str()).x;
						posX = (windowWidth - catWidth) * 0.5f;
						if (posX < 0) posX = 0;
						ImGui::SetCursorPosX(posX);
						ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), cat.Name.c_str());
					}

					for (const auto* desc : cat.Components) {
						ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));
						if (ImGui::Selectable(desc->Name.c_str())) {
							desc->AddToEntity(entity);
							ImGui::CloseCurrentPopup();
						}
						ImGui::PopStyleVar();
					}
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