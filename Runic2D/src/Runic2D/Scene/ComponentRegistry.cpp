#include "R2Dpch.h"
#include "ComponentRegistry.h"
#include "Component.h"
#include <yaml-cpp/yaml.h>
#ifndef R2D_DIST
#include <imgui.h>
#endif


namespace Runic2D {

	// Inicialització de la variable estàtica
	std::vector<ComponentDescriptor> ComponentRegistry::s_Descriptors;

	void ComponentRegistry::Register(ComponentDescriptor&& descriptor)
	{
		auto it = std::find_if(s_Descriptors.begin(), s_Descriptors.end(),
			[&descriptor](const ComponentDescriptor& d) { return d.Name == descriptor.Name; });

		if (it != s_Descriptors.end())
		{
			R2D_CORE_WARN("ComponentRegistry: '{}' ja registrat, sobreescrivint.", descriptor.Name);
			*it = std::move(descriptor);
			return;
		}
		/*
		R2D_CORE_ASSERT(descriptor.Add, "ComponentDescriptor '{}': falta lambda 'Add'", descriptor.Name);
		R2D_CORE_ASSERT(descriptor.Has, "ComponentDescriptor '{}': falta lambda 'Has'", descriptor.Name);
		R2D_CORE_ASSERT(descriptor.Remove, "ComponentDescriptor '{}': falta lambda 'Remove'", descriptor.Name);
		R2D_CORE_ASSERT(descriptor.CopyTo, "ComponentDescriptor '{}': falta lambda 'CopyTo'", descriptor.Name);
		R2D_CORE_ASSERT(descriptor.Serialize, "ComponentDescriptor '{}': falta lambda 'Serialize'", descriptor.Name);
		R2D_CORE_ASSERT(descriptor.Deserialize, "ComponentDescriptor '{}': falta lambda 'Deserialize'", descriptor.Name);
		*/
		// Afegim el descriptor a la llista (fem servir std::move per eficiència)
		s_Descriptors.push_back(std::move(descriptor));
		R2D_CORE_TRACE("ComponentRegistry: Component registrat '{0}' [{1}]", s_Descriptors.back().Name, s_Descriptors.back().Category);
	}

	void ComponentRegistry::Clear()
	{
		s_Descriptors.erase(std::remove_if(s_Descriptors.begin(), s_Descriptors.end(),
			[](const ComponentDescriptor& desc) { return !desc.IsEngineComponent; }), s_Descriptors.end());
		R2D_CORE_TRACE("ComponentRegistry: S'han netejat els components registrats del joc.");
	}

	const std::vector<ComponentDescriptor>& ComponentRegistry::GetAll()
	{
		return s_Descriptors;
	}

	std::vector<const ComponentDescriptor*> ComponentRegistry::GetPresentOn(Entity entity)
	{
		std::vector<const ComponentDescriptor*> presentComponents;

		// Recorrem tots els components registrats
		for (const auto& desc : s_Descriptors)
		{
			// Si l'entitat té aquest component, guardem el punter al descriptor
			if (desc.HasOnEntity(entity))
			{
				presentComponents.push_back(&desc);
			}
		}

		return presentComponents;
	}

	void ComponentRegistry::InitEngineComponents()
	{
		Register({
			"RectTransform", "UI",
			[](Entity e) { if (!e.HasComponent<RectTransformComponent>()) e.AddComponent<RectTransformComponent>(); },
			[](Entity e) { return e.HasComponent<RectTransformComponent>(); },
#ifndef R2D_DIST
			[](Entity e) {
				auto& c = e.GetComponent<RectTransformComponent>();
				ImGui::DragFloat2("Position", &c.Position[0], 0.1f);
				ImGui::DragFloat2("Size", &c.Size[0], 0.1f);
				ImGui::DragFloat2("Anchor Min", &c.AnchorMin[0], 0.05f, 0.0f, 1.0f);
				ImGui::DragFloat2("Anchor Max", &c.AnchorMax[0], 0.05f, 0.0f, 1.0f);
				ImGui::DragFloat2("Pivot", &c.Pivot[0], 0.05f, 0.0f, 1.0f);
				ImGui::DragFloat("Rotation", &c.Rotation, 0.05f);
				ImGui::DragFloat2("Scale", &c.Scale[0], 0.05f);
				ImGui::DragInt("Z Index", &c.ZIndex, 1);
			},
#else
			nullptr,
#endif
			[](Entity e) { e.RemoveComponent<RectTransformComponent>(); },
			[](Entity src, Entity dst) {
				auto& srcData = src.GetComponent<RectTransformComponent>();
				auto& dstData = dst.AddComponent<RectTransformComponent>();
				dstData = srcData;
			},
			[](YAML::Emitter& out, Entity e) {
				auto& c = e.GetComponent<RectTransformComponent>();
				out << YAML::Key << "Position" << YAML::Value << YAML::Flow << YAML::BeginSeq << c.Position.x << c.Position.y << YAML::EndSeq;
				out << YAML::Key << "Size" << YAML::Value << YAML::Flow << YAML::BeginSeq << c.Size.x << c.Size.y << YAML::EndSeq;
				out << YAML::Key << "AnchorMin" << YAML::Value << YAML::Flow << YAML::BeginSeq << c.AnchorMin.x << c.AnchorMin.y << YAML::EndSeq;
				out << YAML::Key << "AnchorMax" << YAML::Value << YAML::Flow << YAML::BeginSeq << c.AnchorMax.x << c.AnchorMax.y << YAML::EndSeq;
				out << YAML::Key << "Pivot" << YAML::Value << YAML::Flow << YAML::BeginSeq << c.Pivot.x << c.Pivot.y << YAML::EndSeq;
				out << YAML::Key << "Rotation" << YAML::Value << c.Rotation;
				out << YAML::Key << "Scale" << YAML::Value << YAML::Flow << YAML::BeginSeq << c.Scale.x << c.Scale.y << YAML::EndSeq;
				out << YAML::Key << "ZIndex" << YAML::Value << c.ZIndex;
			},
			[](YAML::Node& node, Entity e) {
				auto& c = e.AddComponent<RectTransformComponent>();
				if (node["Position"]) { c.Position.x = node["Position"][0].as<float>(); c.Position.y = node["Position"][1].as<float>(); }
				if (node["Size"]) { c.Size.x = node["Size"][0].as<float>(); c.Size.y = node["Size"][1].as<float>(); }
				if (node["AnchorMin"]) { c.AnchorMin.x = node["AnchorMin"][0].as<float>(); c.AnchorMin.y = node["AnchorMin"][1].as<float>(); }
				if (node["AnchorMax"]) { c.AnchorMax.x = node["AnchorMax"][0].as<float>(); c.AnchorMax.y = node["AnchorMax"][1].as<float>(); }
				if (node["Pivot"]) { c.Pivot.x = node["Pivot"][0].as<float>(); c.Pivot.y = node["Pivot"][1].as<float>(); }
				if (node["Rotation"]) c.Rotation = node["Rotation"].as<float>();
				if (node["Scale"]) { c.Scale.x = node["Scale"][0].as<float>(); c.Scale.y = node["Scale"][1].as<float>(); }
				if (node["ZIndex"]) c.ZIndex = node["ZIndex"].as<int>();
			},
			true // IsEngineComponent
		});
	}
}
