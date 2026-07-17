#include "R2Dpch.h"

#include "Runic2D/Project/Project.h"
#include "Runic2D/Assets/ResourceManager.h"
#include "Runic2D/Renderer/Renderer2D.h"

#include "ComponentRegistry.h"
#include "MotionComponents.h"
#include "PhysicsComponents.h"
#include "RenderComponents.h"
#include "ScriptingComponents.h"
#include "UIComponents.h"
#include "AudioComponents.h"

#include <yaml-cpp/yaml.h>
#include <glm/gtc/type_ptr.hpp>

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
			"CameraComponent", "Core",
			[](Entity e) { if (!e.HasComponent<CameraComponent>()) e.AddComponent<CameraComponent>(); },
			[](Entity e) { return e.HasComponent<CameraComponent>(); },
#ifndef R2D_DIST
			[](Entity e) {
				auto& component = e.GetComponent<CameraComponent>();
				auto& camera = component.Camera;
				ImGui::Checkbox("Primary", &component.Primary);
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
						if (isSelected) ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
				{
					float orthoSize = camera.GetOrthographicSize();
					if (ImGui::DragFloat("Size", &orthoSize)) camera.SetOrthographicSize(orthoSize);
					float orthoNear = camera.GetOrthographicNearClip();
					if (ImGui::DragFloat("Near Clip", &orthoNear)) camera.SetOrthographicNearClip(orthoNear);
					float orthoFar = camera.GetOrthographicFarClip();
					if (ImGui::DragFloat("Far Clip", &orthoFar)) camera.SetOrthographicFarClip(orthoFar);
					ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);
				}
				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
				{
					float verticalFov = glm::degrees(camera.GetPerspectiveVerticalFOV());
					if (ImGui::DragFloat("Vertical FOV", &verticalFov)) camera.SetPerspectiveVerticalFOV(glm::radians(verticalFov));
					float perspNear = camera.GetPerspectiveNearClip();
					if (ImGui::DragFloat("Near Clip", &perspNear)) camera.SetPerspectiveNearClip(perspNear);
					float perspFar = camera.GetPerspectiveFarClip();
					if (ImGui::DragFloat("Far Clip", &perspFar)) camera.SetPerspectiveFarClip(perspFar);
				}
			},
#else
			nullptr,
#endif
			[](Entity e) { e.RemoveComponent<CameraComponent>(); },
			[](Entity src, Entity dst) { dst.AddOrReplaceComponent<CameraComponent>(src.GetComponent<CameraComponent>()); },
			[](YAML::Emitter& out, Entity e) {
				auto& cameraComponent = e.GetComponent<CameraComponent>();
				auto& camera = cameraComponent.Camera;
				out << YAML::Key << "Camera";
				out << YAML::BeginMap;
				out << YAML::Key << "ProjectionType" << YAML::Value << static_cast<int>(camera.GetProjectionType());
				out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
				out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
				out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
				out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
				out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
				out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
				out << YAML::EndMap;
				out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
				out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;
			},
			[](YAML::Node& node, Entity e) {
				auto& cc = e.AddComponent<CameraComponent>();
				auto cameraProps = node["Camera"];
				cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());
				cc.Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
				cc.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
				cc.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());
				cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
				cc.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
				cc.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());
				cc.Primary = node["Primary"].as<bool>();
				cc.FixedAspectRatio = node["FixedAspectRatio"].as<bool>();
			},
			[](BufferStreamWriter& out, Entity e) {
				auto& cc = e.GetComponent<CameraComponent>();
				out.WriteRaw(cc.Primary);
				out.WriteRaw(cc.FixedAspectRatio);
				out.WriteRaw(cc.Camera.GetProjectionType());
				out.WriteRaw(cc.Camera.GetPerspectiveVerticalFOV());
				out.WriteRaw(cc.Camera.GetPerspectiveNearClip());
				out.WriteRaw(cc.Camera.GetPerspectiveFarClip());
				out.WriteRaw(cc.Camera.GetOrthographicSize());
				out.WriteRaw(cc.Camera.GetOrthographicNearClip());
				out.WriteRaw(cc.Camera.GetOrthographicFarClip());
			},
			[](BufferStreamReader& in, Entity e) {
				CameraComponent cc;
				in.ReadRaw(cc.Primary);
				in.ReadRaw(cc.FixedAspectRatio);

				SceneCamera::ProjectionType projType;
				float pFov, pNear, pFar;
				float oSize, oNear, oFar;
				in.ReadRaw(projType);
				in.ReadRaw(pFov);
				in.ReadRaw(pNear);
				in.ReadRaw(pFar);
				in.ReadRaw(oSize);
				in.ReadRaw(oNear);
				in.ReadRaw(oFar);

				cc.Camera.SetProjectionType(projType);
				cc.Camera.SetPerspective(pFov, pNear, pFar);
				cc.Camera.SetOrthographic(oSize, oNear, oFar);
				e.AddOrReplaceComponent<CameraComponent>(cc);
			},
			true
		});

		Register({
			"SpriteRendererComponent", "Renderer",
			[](Entity e) { if (!e.HasComponent<SpriteRendererComponent>()) e.AddComponent<SpriteRendererComponent>(); },
			[](Entity e) { return e.HasComponent<SpriteRendererComponent>(); },
#ifndef R2D_DIST
			[](Entity e) {
				auto& component = e.GetComponent<SpriteRendererComponent>();
				ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
				ImGui::Text("Texture");
				ImGui::SameLine();
				Ref<Texture2D> textureToShow = component.Texture ? component.Texture : Renderer2D::GetWhiteTexture();
				ImGui::ImageButton("TexturePreview", (ImTextureID)textureToShow->GetRendererID(), ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const char* path = (const char*)payload->Data;
						std::filesystem::path texturePath = Project::GetAssetFileSystemPath(path);
						component.Texture = ResourceManager::Get<Texture2D>(texturePath.string());
					}
					ImGui::EndDragDropTarget();
				}
				if (ImGui::Button("Clear Texture")) component.Texture = nullptr;
				ImGui::DragFloat("Tiling Factor", &component.TilingFactor, 0.1f, 0.0f, 100.0f);
			},
#else
			nullptr,
#endif
			[](Entity e) { e.RemoveComponent<SpriteRendererComponent>(); },
			[](Entity src, Entity dst) { dst.AddOrReplaceComponent<SpriteRendererComponent>(src.GetComponent<SpriteRendererComponent>()); },
			[](YAML::Emitter& out, Entity e) {
				auto& spriteRenderer = e.GetComponent<SpriteRendererComponent>();
				out << YAML::Key << "Color" << YAML::Value << YAML::Flow << YAML::BeginSeq << spriteRenderer.Color.r << spriteRenderer.Color.g << spriteRenderer.Color.b << spriteRenderer.Color.a << YAML::EndSeq;
				if (spriteRenderer.Texture)
				{
					out << YAML::Key << "TextureUUID" << YAML::Value << (uint64_t)spriteRenderer.Texture->Handle;
				}
				out << YAML::Key << "TilingFactor" << YAML::Value << spriteRenderer.TilingFactor;
			},
			[](YAML::Node& node, Entity e) {
				auto& src = e.AddComponent<SpriteRendererComponent>();
				if (node["Color"]) { src.Color.r = node["Color"][0].as<float>(); src.Color.g = node["Color"][1].as<float>(); src.Color.b = node["Color"][2].as<float>(); src.Color.a = node["Color"][3].as<float>(); }
				if (node["TextureUUID"]) {
					src.Texture = ResourceManager::Get<Texture2D>(node["TextureUUID"].as<uint64_t>());
				}
				else if (node["TexturePath"])
				{
					std::string texturePathString = node["TexturePath"].as<std::string>();
					std::filesystem::path path = Project::GetAssetFileSystemPath(texturePathString);
					if (!std::filesystem::exists(path) && std::filesystem::exists(texturePathString)) path = texturePathString;

					if (std::filesystem::exists(path)) src.Texture = ResourceManager::Get<Texture2D>(path); // Aquest Get generarà l'UUID sol!
					else R2D_CORE_WARN("Texture not found: {0}", path.string());
				}
				if (node["TilingFactor"]) src.TilingFactor = node["TilingFactor"].as<float>();
			},
			[](BufferStreamWriter& out, Entity e) {
				auto& src = e.GetComponent<SpriteRendererComponent>();
				out.WriteRaw(src.Color);
				out.WriteRaw(src.TextureUUID); // Guardem l'UUID, no el punter!
				out.WriteRaw(src.TilingFactor);
			},
			[](BufferStreamReader& in, Entity e) {
				SpriteRendererComponent src;
				in.ReadRaw(src.Color);
				in.ReadRaw(src.TextureUUID);
				if (src.TextureUUID != 0)
				{
					src.Texture = ResourceManager::Get<Texture2D>(src.TextureUUID);
				}
				in.ReadRaw(src.TilingFactor);
				// El punter Ref<Texture> el deixem null. Quan el joc arrenqui, el Scene ja buscarà la textura usant l'UUID!
				e.AddOrReplaceComponent<SpriteRendererComponent>(src);
			},
			true
		});

		Register({
			"CircleRendererComponent", "Renderer",
			[](Entity e) { if (!e.HasComponent<CircleRendererComponent>()) e.AddComponent<CircleRendererComponent>(); },
			[](Entity e) { return e.HasComponent<CircleRendererComponent>(); },
#ifndef R2D_DIST
			[](Entity e) {
				auto& component = e.GetComponent<CircleRendererComponent>();
				ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
				ImGui::DragFloat("Thickness", &component.Thickness, 0.025f, 0.0f, 1.0f);
				ImGui::DragFloat("Fade", &component.Fade, 0.00025f, 0.0f, 1.0f);
			},
#else
			nullptr,
#endif
			[](Entity e) { e.RemoveComponent<CircleRendererComponent>(); },
			[](Entity src, Entity dst) { dst.AddOrReplaceComponent<CircleRendererComponent>(src.GetComponent<CircleRendererComponent>()); },
			[](YAML::Emitter& out, Entity e) {
				auto& circleRendererComponent = e.GetComponent<CircleRendererComponent>();
				out << YAML::Key << "Color" << YAML::Value << YAML::Flow << YAML::BeginSeq << circleRendererComponent.Color.r << circleRendererComponent.Color.g << circleRendererComponent.Color.b << circleRendererComponent.Color.a << YAML::EndSeq;
				out << YAML::Key << "Thickness" << YAML::Value << circleRendererComponent.Thickness;
				out << YAML::Key << "Fade" << YAML::Value << circleRendererComponent.Fade;
			},
			[](YAML::Node& node, Entity e) {
				auto& crc = e.AddComponent<CircleRendererComponent>();
				if (node["Color"]) { crc.Color.r = node["Color"][0].as<float>(); crc.Color.g = node["Color"][1].as<float>(); crc.Color.b = node["Color"][2].as<float>(); crc.Color.a = node["Color"][3].as<float>(); }
				if (node["Thickness"]) crc.Thickness = node["Thickness"].as<float>();
				if (node["Fade"]) crc.Fade = node["Fade"].as<float>();
			},
			[](BufferStreamWriter& out, Entity e) {
				auto& src = e.GetComponent<CircleRendererComponent>();
				out.WriteRaw(src.Color);
				out.WriteRaw(src.Thickness); // Guardem l'UUID, no el punter!
				out.WriteRaw(src.Fade);
			},
			[](BufferStreamReader& in, Entity e) {
				CircleRendererComponent src;
				in.ReadRaw(src.Color);
				in.ReadRaw(src.Thickness);
				in.ReadRaw(src.Fade);
				// El punter Ref<Texture> el deixem null. Quan el joc arrenqui, el Scene ja buscarà la textura usant l'UUID!
				e.AddOrReplaceComponent<CircleRendererComponent>(src);
			},
			true
		});

		Register({
			"NativeScriptComponent", "Core",
			[](Entity e) { if (!e.HasComponent<NativeScriptComponent>()) e.AddComponent<NativeScriptComponent>(); },
			[](Entity e) { return e.HasComponent<NativeScriptComponent>(); },
#ifndef R2D_DIST
			[](Entity e) {
				auto& component = e.GetComponent<NativeScriptComponent>();
				char buffer[256];
				memset(buffer, 0, sizeof(buffer));
				std::strncpy(buffer, component.ClassName.c_str(), sizeof(buffer));
				if (ImGui::InputText("Class Name", buffer, sizeof(buffer)))
				{
					component.ClassName = std::string(buffer);
				}
			},
#else
			nullptr,
#endif
			[](Entity e) { e.RemoveComponent<NativeScriptComponent>(); },
			[](Entity src, Entity dst) { dst.AddOrReplaceComponent<NativeScriptComponent>(src.GetComponent<NativeScriptComponent>()); },
			[](YAML::Emitter& out, Entity e) {
				auto& nsc = e.GetComponent<NativeScriptComponent>();
				out << YAML::Key << "ClassName" << YAML::Value << nsc.ClassName;
			},
			[](YAML::Node& node, Entity e) {
				auto& nsc = e.AddComponent<NativeScriptComponent>();
				if (node["ClassName"]) nsc.ClassName = node["ClassName"].as<std::string>();
			},
			[](BufferStreamWriter& out, Entity e) {
				out.WriteString(e.GetComponent<NativeScriptComponent>().ClassName);
			},
			[](BufferStreamReader& in, Entity e) {
				NativeScriptComponent nsc;
				in.ReadString(nsc.ClassName);
				e.AddOrReplaceComponent<NativeScriptComponent>(nsc);
			},
			true
		});

		Register({
			"Rigidbody2DComponent", "Physics",
			[](Entity e) { if (!e.HasComponent<Rigidbody2DComponent>()) e.AddComponent<Rigidbody2DComponent>(); },
			[](Entity e) { return e.HasComponent<Rigidbody2DComponent>(); },
#ifndef R2D_DIST
			[](Entity e) {
				auto& component = e.GetComponent<Rigidbody2DComponent>();
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
						if (isSelected) ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				ImGui::Checkbox("Fixed Rotation", &component.FixedRotation);
				ImGui::DragFloat("Gravity Scale", &component.GravityScale, 0.01f);
			},
#else
			nullptr,
#endif
			[](Entity e) { e.RemoveComponent<Rigidbody2DComponent>(); },
			[](Entity src, Entity dst) { dst.AddOrReplaceComponent<Rigidbody2DComponent>(src.GetComponent<Rigidbody2DComponent>()); },
			[](YAML::Emitter& out, Entity e) {
				auto& rb2dComponent = e.GetComponent<Rigidbody2DComponent>();
				auto typeToString = [](Rigidbody2DComponent::BodyType type) {
					switch (type) { case Rigidbody2DComponent::BodyType::Static: return "Static"; case Rigidbody2DComponent::BodyType::Dynamic: return "Dynamic"; case Rigidbody2DComponent::BodyType::Kinematic: return "Kinematic"; } return "Static";
				};
				out << YAML::Key << "BodyType" << YAML::Value << typeToString(rb2dComponent.Type);
				out << YAML::Key << "FixedRotation" << YAML::Value << rb2dComponent.FixedRotation;
				out << YAML::Key << "GravityScale" << YAML::Value << rb2dComponent.GravityScale;
			},
			[](YAML::Node& node, Entity e) {
				auto& rb2d = e.AddComponent<Rigidbody2DComponent>();
				auto stringToType = [](const std::string& type) {
					if (type == "Static") return Rigidbody2DComponent::BodyType::Static;
					if (type == "Dynamic") return Rigidbody2DComponent::BodyType::Dynamic;
					if (type == "Kinematic") return Rigidbody2DComponent::BodyType::Kinematic;
					return Rigidbody2DComponent::BodyType::Static;
				};
				if (node["BodyType"]) rb2d.Type = stringToType(node["BodyType"].as<std::string>());
				if (node["FixedRotation"]) rb2d.FixedRotation = node["FixedRotation"].as<bool>();
				if (node["GravityScale"]) rb2d.GravityScale = node["GravityScale"].as<float>();
			},
			[](BufferStreamWriter& out, Entity e) {
				auto& rbc = e.GetComponent<Rigidbody2DComponent>();
				out.WriteRaw(rbc.Type);
				out.WriteRaw(rbc.FixedRotation); 
				out.WriteRaw(rbc.GravityScale);
			},
			[](BufferStreamReader& in, Entity e) {
				Rigidbody2DComponent rbc;
				in.ReadRaw(rbc.Type);
				in.ReadRaw(rbc.FixedRotation);
				in.ReadRaw(rbc.GravityScale);
				e.AddOrReplaceComponent<Rigidbody2DComponent>(rbc);
			},
			true
		});

		Register({
			"BoxCollider2DComponent", "Physics",
			[](Entity e) { if (!e.HasComponent<BoxCollider2DComponent>()) e.AddComponent<BoxCollider2DComponent>(); },
			[](Entity e) { return e.HasComponent<BoxCollider2DComponent>(); },
#ifndef R2D_DIST
			[](Entity e) {
				auto& component = e.GetComponent<BoxCollider2DComponent>();
				ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
				ImGui::DragFloat2("Size", glm::value_ptr(component.Size));
				ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
				ImGui::Checkbox("Is Sensor", &component.IsSensor);
				ImGui::Checkbox("Enable Contact Events", &component.EnableContactEvents);
				ImGui::Checkbox("Enable Sensor Events", &component.EnableSensorEvents);
			},
#else
			nullptr,
#endif
			[](Entity e) { e.RemoveComponent<BoxCollider2DComponent>(); },
			[](Entity src, Entity dst) { dst.AddOrReplaceComponent<BoxCollider2DComponent>(src.GetComponent<BoxCollider2DComponent>()); },
			[](YAML::Emitter& out, Entity e) {
				auto& bc2dComponent = e.GetComponent<BoxCollider2DComponent>();
				out << YAML::Key << "Offset" << YAML::Value << YAML::Flow << YAML::BeginSeq << bc2dComponent.Offset.x << bc2dComponent.Offset.y << YAML::EndSeq;
				out << YAML::Key << "Size" << YAML::Value << YAML::Flow << YAML::BeginSeq << bc2dComponent.Size.x << bc2dComponent.Size.y << YAML::EndSeq;
				out << YAML::Key << "Density" << YAML::Value << bc2dComponent.Density;
				out << YAML::Key << "Friction" << YAML::Value << bc2dComponent.Friction;
				out << YAML::Key << "Restitution" << YAML::Value << bc2dComponent.Restitution;
				out << YAML::Key << "RestitutionThreshold" << YAML::Value << bc2dComponent.RestitutionThreshold;
				out << YAML::Key << "IsSensor" << YAML::Value << bc2dComponent.IsSensor;
				out << YAML::Key << "EnableContactEvents" << YAML::Value << bc2dComponent.EnableContactEvents;
				out << YAML::Key << "EnableSensorEvents" << YAML::Value << bc2dComponent.EnableSensorEvents;
			},
			[](YAML::Node& node, Entity e) {
				auto& bc2d = e.AddComponent<BoxCollider2DComponent>();
				if (node["Offset"]) { bc2d.Offset.x = node["Offset"][0].as<float>(); bc2d.Offset.y = node["Offset"][1].as<float>(); }
				if (node["Size"]) { bc2d.Size.x = node["Size"][0].as<float>(); bc2d.Size.y = node["Size"][1].as<float>(); }
				if (node["Density"]) bc2d.Density = node["Density"].as<float>();
				if (node["Friction"]) bc2d.Friction = node["Friction"].as<float>();
				if (node["Restitution"]) bc2d.Restitution = node["Restitution"].as<float>();
				if (node["RestitutionThreshold"]) bc2d.RestitutionThreshold = node["RestitutionThreshold"].as<float>();
				if (node["IsSensor"]) bc2d.IsSensor = node["IsSensor"].as<bool>();
				if (node["EnableContactEvents"]) bc2d.EnableContactEvents = node["EnableContactEvents"].as<bool>();
				if (node["EnableSensorEvents"]) bc2d.EnableSensorEvents = node["EnableSensorEvents"].as<bool>();
			},
			[](BufferStreamWriter& out, Entity e) {
				auto& bcc = e.GetComponent<BoxCollider2DComponent>();
				out.WriteRaw(bcc.Offset);
				out.WriteRaw(bcc.Size);
				out.WriteRaw(bcc.Density);
				out.WriteRaw(bcc.Friction);
				out.WriteRaw(bcc.Restitution);
				out.WriteRaw(bcc.RestitutionThreshold);
				out.WriteRaw(bcc.CategoryBits);
				out.WriteRaw(bcc.MaskBits);
				out.WriteRaw(bcc.GroupIndex);
				out.WriteRaw(bcc.IsSensor);
				out.WriteRaw(bcc.EnableContactEvents);
				out.WriteRaw(bcc.EnableSensorEvents);
			},
			[](BufferStreamReader& in, Entity e) {
				BoxCollider2DComponent bcc;
				in.ReadRaw(bcc.Offset);
				in.ReadRaw(bcc.Size);
				in.ReadRaw(bcc.Density);
				in.ReadRaw(bcc.Friction);
				in.ReadRaw(bcc.Restitution);
				in.ReadRaw(bcc.RestitutionThreshold);
				in.ReadRaw(bcc.CategoryBits);
				in.ReadRaw(bcc.MaskBits);
				in.ReadRaw(bcc.GroupIndex);
				in.ReadRaw(bcc.IsSensor);
				in.ReadRaw(bcc.EnableContactEvents);
				in.ReadRaw(bcc.EnableSensorEvents);
				e.AddOrReplaceComponent<BoxCollider2DComponent>(bcc);
			},
			true
		});

		Register({
			"CircleCollider2DComponent", "Physics",
			[](Entity e) { if (!e.HasComponent<CircleCollider2DComponent>()) e.AddComponent<CircleCollider2DComponent>(); },
			[](Entity e) { return e.HasComponent<CircleCollider2DComponent>(); },
#ifndef R2D_DIST
			[](Entity e) {
				auto& component = e.GetComponent<CircleCollider2DComponent>();
				ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
				ImGui::DragFloat("Radius", &component.Radius);
				ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
				ImGui::Checkbox("Is Sensor", &component.IsSensor);
				ImGui::Checkbox("Enable Contact Events", &component.EnableContactEvents);
				ImGui::Checkbox("Enable Sensor Events", &component.EnableSensorEvents);
			},
#else
			nullptr,
#endif
			[](Entity e) { e.RemoveComponent<CircleCollider2DComponent>(); },
			[](Entity src, Entity dst) { dst.AddOrReplaceComponent<CircleCollider2DComponent>(src.GetComponent<CircleCollider2DComponent>()); },
			[](YAML::Emitter& out, Entity e) {
				auto& cc2dComponent = e.GetComponent<CircleCollider2DComponent>();
				out << YAML::Key << "Offset" << YAML::Value << YAML::Flow << YAML::BeginSeq << cc2dComponent.Offset.x << cc2dComponent.Offset.y << YAML::EndSeq;
				out << YAML::Key << "Radius" << YAML::Value << cc2dComponent.Radius;
				out << YAML::Key << "Density" << YAML::Value << cc2dComponent.Density;
				out << YAML::Key << "Friction" << YAML::Value << cc2dComponent.Friction;
				out << YAML::Key << "Restitution" << YAML::Value << cc2dComponent.Restitution;
				out << YAML::Key << "RestitutionThreshold" << YAML::Value << cc2dComponent.RestitutionThreshold;
				out << YAML::Key << "IsSensor" << YAML::Value << cc2dComponent.IsSensor;
				out << YAML::Key << "EnableContactEvents" << YAML::Value << cc2dComponent.EnableContactEvents;
				out << YAML::Key << "EnableSensorEvents" << YAML::Value << cc2dComponent.EnableSensorEvents;
			},
			[](YAML::Node& node, Entity e) {
				auto& cc2d = e.AddComponent<CircleCollider2DComponent>();
				if (node["Offset"]) { cc2d.Offset.x = node["Offset"][0].as<float>(); cc2d.Offset.y = node["Offset"][1].as<float>(); }
				if (node["Radius"]) cc2d.Radius = node["Radius"].as<float>();
				if (node["Density"]) cc2d.Density = node["Density"].as<float>();
				if (node["Friction"]) cc2d.Friction = node["Friction"].as<float>();
				if (node["Restitution"]) cc2d.Restitution = node["Restitution"].as<float>();
				if (node["RestitutionThreshold"]) cc2d.RestitutionThreshold = node["RestitutionThreshold"].as<float>();
				if (node["IsSensor"]) cc2d.IsSensor = node["IsSensor"].as<bool>();
				if (node["EnableContactEvents"]) cc2d.EnableContactEvents = node["EnableContactEvents"].as<bool>();
				if (node["EnableSensorEvents"]) cc2d.EnableSensorEvents = node["EnableSensorEvents"].as<bool>();
			},
			[](BufferStreamWriter& out, Entity e) {
				auto& ccc = e.GetComponent<CircleCollider2DComponent>();
				out.WriteRaw(ccc.Offset);
				out.WriteRaw(ccc.Radius);
				out.WriteRaw(ccc.Density);
				out.WriteRaw(ccc.Friction);
				out.WriteRaw(ccc.Restitution);
				out.WriteRaw(ccc.RestitutionThreshold);
				out.WriteRaw(ccc.CategoryBits);
				out.WriteRaw(ccc.MaskBits);
				out.WriteRaw(ccc.GroupIndex);
				out.WriteRaw(ccc.IsSensor);
				out.WriteRaw(ccc.EnableContactEvents);
				out.WriteRaw(ccc.EnableSensorEvents);
			},
			[](BufferStreamReader& in, Entity e) {
				CircleCollider2DComponent ccc;
				in.ReadRaw(ccc.Offset);
				in.ReadRaw(ccc.Radius);
				in.ReadRaw(ccc.Density);
				in.ReadRaw(ccc.Friction);
				in.ReadRaw(ccc.Restitution);
				in.ReadRaw(ccc.RestitutionThreshold);
				in.ReadRaw(ccc.CategoryBits);
				in.ReadRaw(ccc.MaskBits);
				in.ReadRaw(ccc.GroupIndex);
				in.ReadRaw(ccc.IsSensor);
				in.ReadRaw(ccc.EnableContactEvents);
				in.ReadRaw(ccc.EnableSensorEvents);
				e.AddOrReplaceComponent<CircleCollider2DComponent>(ccc);
			},
			true
		});

		Register({
			"TextComponent", "Renderer",
			[](Entity e) { if (!e.HasComponent<TextComponent>()) e.AddComponent<TextComponent>(); },
			[](Entity e) { return e.HasComponent<TextComponent>(); },
#ifndef R2D_DIST
			[](Entity e) {
				auto& component = e.GetComponent<TextComponent>();
				char buffer[256];
				memset(buffer, 0, sizeof(buffer));
				std::strncpy(buffer, component.GetText().c_str(), sizeof(buffer));
				if (ImGui::InputTextMultiline("Text String", buffer, sizeof(buffer)))
				{
					component.SetText(std::string(buffer));
				}
				ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
				if (ImGui::DragFloat("Kerning", &component.Kerning, 0.025f))
					component.MarkDirty();
				ImGui::DragFloat("Line Spacing", &component.LineSpacing, 0.025f);

				const char* alignmentStrings[] = { "Left", "Center", "Right" };
				const char* currentAlignmentString = alignmentStrings[(int)component.TextAlignment];

				if (ImGui::BeginCombo("Alignment", currentAlignmentString))
				{
					for (int i = 0; i < 3; i++)
					{
						bool isSelected = (currentAlignmentString == alignmentStrings[i]);
						if (ImGui::Selectable(alignmentStrings[i], isSelected))
						{
							currentAlignmentString = alignmentStrings[i];
							component.TextAlignment = (TextComponent::Alignment)i;
						}
						if (isSelected) ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			},
#else
			nullptr,
#endif
			[](Entity e) { e.RemoveComponent<TextComponent>(); },
			[](Entity src, Entity dst) { dst.AddOrReplaceComponent<TextComponent>(src.GetComponent<TextComponent>()); },
			[](YAML::Emitter& out, Entity e) {
				auto& tc = e.GetComponent<TextComponent>();
				out << YAML::Key << "TextString" << YAML::Value << tc.GetText();
				out << YAML::Key << "Color" << YAML::Value << YAML::Flow << YAML::BeginSeq << tc.Color.r << tc.Color.g << tc.Color.b << tc.Color.a << YAML::EndSeq;
				out << YAML::Key << "Kerning" << YAML::Value << tc.Kerning;
				out << YAML::Key << "LineSpacing" << YAML::Value << tc.LineSpacing;
				out << YAML::Key << "Alignment" << YAML::Value << (int)tc.TextAlignment;
			},
			[](YAML::Node& node, Entity e) {
				auto& tc = e.AddComponent<TextComponent>();
				if (node["TextString"]) tc.SetText(node["TextString"].as<std::string>());
				if (node["Color"]) { tc.Color.r = node["Color"][0].as<float>(); tc.Color.g = node["Color"][1].as<float>(); tc.Color.b = node["Color"][2].as<float>(); tc.Color.a = node["Color"][3].as<float>(); }
				if (node["Kerning"]) tc.Kerning = node["Kerning"].as<float>();
				if (node["LineSpacing"]) tc.LineSpacing = node["LineSpacing"].as<float>();
				if (node["Alignment"]) tc.TextAlignment = (TextComponent::Alignment)node["Alignment"].as<int>();
			},
			[](BufferStreamWriter& out, Entity e) {
				auto& tc = e.GetComponent<TextComponent>();
				out.WriteRaw(tc.Color);
				out.WriteRaw(tc.FontUUID);
				out.WriteRaw(tc.Kerning);
				out.WriteRaw(tc.LineSpacing);
				out.WriteRaw(tc.Visible);
				out.WriteRaw(tc.TextAlignment);
				out.WriteString(tc.GetText());
			},
			[](BufferStreamReader& in, Entity e) {
				TextComponent tc;
				in.ReadRaw(tc.Color);
				in.ReadRaw(tc.FontUUID);
				in.ReadRaw(tc.Kerning);
				in.ReadRaw(tc.LineSpacing);
				in.ReadRaw(tc.Visible);
				in.ReadRaw(tc.TextAlignment);
				std::string text;
				in.ReadString(text);
				tc.SetText(text);
				e.AddOrReplaceComponent<TextComponent>(tc);

				if (tc.FontUUID != 0) {
					tc.FontAsset = ResourceManager::Get<Font>(tc.FontUUID);
				}
				if (!tc.FontAsset) {
					tc.FontAsset = Font::GetDefault();
				}
			},
			true
		});

		Register({
			"AnimationComponent", "Renderer",
			[](Entity e) { if (!e.HasComponent<AnimationComponent>()) e.AddComponent<AnimationComponent>(); },
			[](Entity e) { return e.HasComponent<AnimationComponent>(); },
#ifndef R2D_DIST
			[](Entity e) {
				auto& component = e.GetComponent<AnimationComponent>();
				ImGui::Checkbox("Playing", &component.Playing);
				ImGui::Checkbox("Loop All", &component.Loop);
				
				if (ImGui::Button("Add Animation Profile"))
				{
					AnimationProfile newProfile;
					newProfile.Name = "New Animation";
					component.Profiles.push_back(newProfile);
				}

				if (ImGui::BeginCombo("Current State", component.CurrentStateName.c_str()))
				{
					for (auto& profile : component.Profiles)
					{
						bool isSelected = (component.CurrentStateName == profile.Name);
						if (ImGui::Selectable(profile.Name.c_str(), isSelected))
						{
							component.CurrentStateName = profile.Name;
							component.TimeAccumulator = 0.0f;
							component.CurrentFrameIndex = 0;
							if (component.Animations.find(profile.Name) != component.Animations.end()) {
								component.CurrentAnimation = component.Animations[profile.Name];
							}
						}
						if (isSelected) ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				for (size_t i = 0; i < component.Profiles.size(); i++)
				{
					auto& profile = component.Profiles[i];
					ImGui::PushID((int)i);
					if (ImGui::TreeNodeEx((void*)(intptr_t)i, ImGuiTreeNodeFlags_Framed, "%s", profile.Name.c_str()))
					{
						char buffer[256];
						memset(buffer, 0, sizeof(buffer));
						std::strncpy(buffer, profile.Name.c_str(), sizeof(buffer));
						if (ImGui::InputText("Name", buffer, sizeof(buffer))) profile.Name = std::string(buffer);
						
						ImGui::Text("Sprite Sheet");
						Ref<Texture2D> textureToShow = profile.AtlasTexture ? profile.AtlasTexture : Renderer2D::GetWhiteTexture();
						ImGui::ImageButton("TexturePreview", (ImTextureID)textureToShow->GetRendererID(), ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));

						if (ImGui::BeginDragDropTarget())
						{
							if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
							{
								const char* path = (const char*)payload->Data;
								std::filesystem::path texturePath = Project::GetAssetFileSystemPath(path);
								profile.AtlasTexture = ResourceManager::Get<Texture2D>(texturePath.string());
								profile.TexturePath = texturePath.string();
								if (profile.TileSize.x > 0)
									profile.FramesPerRow = (int)(profile.AtlasTexture->GetWidth() / profile.TileSize.x);
							}
							ImGui::EndDragDropTarget();
						}

						bool dirty = false;
						dirty |= ImGui::DragFloat2("Tile Size", glm::value_ptr(profile.TileSize));
						dirty |= ImGui::DragInt("Start Frame", &profile.StartFrame);
						dirty |= ImGui::DragInt("Frame Count", &profile.FrameCount);
						dirty |= ImGui::DragInt("Frames Per Row", &profile.FramesPerRow, 0.1f, 1, 100);
						if (ImGui::IsItemHovered()) ImGui::SetTooltip("0 = Auto-detect (Single Row)");
						dirty |= ImGui::DragFloat("Speed", &profile.FrameTime, 0.01f, 0.01f, 10.0f);
						if (dirty) {
							component.Animations.clear();
						}

						if (profile.AtlasTexture && ImGui::Button("Auto-Calc Size from Rows"))
						{
							if (profile.FramesPerRow > 0)
							{
								float width = (float)profile.AtlasTexture->GetWidth();
								float height = (float)profile.AtlasTexture->GetHeight();
								profile.TileSize.x = width / (float)profile.FramesPerRow;
								int numRows = (profile.FrameCount / profile.FramesPerRow) + (profile.FrameCount % profile.FramesPerRow > 0 ? 1 : 0);
								if (numRows < 1) numRows = 1;
								profile.TileSize.y = height / (float)numRows;
								component.Animations.clear();
							}
						}

						if (ImGui::Checkbox("Loop", &profile.Loop))
						{
							if (component.CurrentStateName == profile.Name) component.Loop = profile.Loop;
						}
						ImGui::TreePop();
					}
					ImGui::PopID();
				}
			},
#else
			nullptr,
#endif
			[](Entity e) { e.RemoveComponent<AnimationComponent>(); },
			[](Entity src, Entity dst) { dst.AddOrReplaceComponent<AnimationComponent>(src.GetComponent<AnimationComponent>()); },
			[](YAML::Emitter& out, Entity e) {
				auto& ac = e.GetComponent<AnimationComponent>();
				out << YAML::Key << "Playing" << YAML::Value << ac.Playing;
				out << YAML::Key << "Loop" << YAML::Value << ac.Loop;
				out << YAML::Key << "Profiles" << YAML::Value << YAML::BeginSeq;
				for (auto& profile : ac.Profiles)
				{
					out << YAML::BeginMap;
					out << YAML::Key << "Name" << YAML::Value << profile.Name;
					out << YAML::Key << "TexturePath" << YAML::Value << profile.TexturePath;
					out << YAML::Key << "TileSize" << YAML::Value << YAML::Flow << YAML::BeginSeq << profile.TileSize.x << profile.TileSize.y << YAML::EndSeq;
					out << YAML::Key << "StartFrame" << YAML::Value << profile.StartFrame;
					out << YAML::Key << "FrameCount" << YAML::Value << profile.FrameCount;
					out << YAML::Key << "FramesPerRow" << YAML::Value << profile.FramesPerRow;
					out << YAML::Key << "FrameTime" << YAML::Value << profile.FrameTime;
					out << YAML::Key << "Loop" << YAML::Value << profile.Loop;
					out << YAML::EndMap;
				}
				out << YAML::EndSeq;
			},
			[](YAML::Node& node, Entity e) {
				auto& ac = e.AddComponent<AnimationComponent>();
				if (node["Playing"]) ac.Playing = node["Playing"].as<bool>();
				if (node["Loop"]) ac.Loop = node["Loop"].as<bool>();
				
				auto profilesNode = node["Profiles"];
				if (profilesNode)
				{
					for (auto profileNode : profilesNode)
					{
						AnimationProfile profile;
						profile.Name = profileNode["Name"].as<std::string>();
						if (profileNode["TexturePath"])
						{
							profile.TexturePath = profileNode["TexturePath"].as<std::string>();
							std::filesystem::path path = Project::GetAssetFileSystemPath(profile.TexturePath);
							if (!std::filesystem::exists(path) && std::filesystem::exists(profile.TexturePath)) path = profile.TexturePath;

							if (std::filesystem::exists(path)) {
								profile.AtlasTexture = ResourceManager::Get<Texture2D>(profile.TexturePath);
							} else {
								R2D_CORE_WARN("Animation Texture not found: {0}", path.string());
							}
						}
						if (profileNode["TileSize"]) { profile.TileSize.x = profileNode["TileSize"][0].as<float>(); profile.TileSize.y = profileNode["TileSize"][1].as<float>(); }
						if (profileNode["StartFrame"]) profile.StartFrame = profileNode["StartFrame"].as<int>();
						if (profileNode["FrameCount"]) profile.FrameCount = profileNode["FrameCount"].as<int>();
						if (profileNode["FramesPerRow"]) profile.FramesPerRow = profileNode["FramesPerRow"].as<int>();
						if (profileNode["FrameTime"]) profile.FrameTime = profileNode["FrameTime"].as<float>();
						if (profileNode["Loop"]) profile.Loop = profileNode["Loop"].as<bool>();
						ac.Profiles.push_back(profile);
					}
					
					if (!ac.Profiles.empty())
					{
						auto& profile = ac.Profiles[0];
						if (ac.CurrentStateName.empty()) ac.CurrentStateName = profile.Name;
						
						R2D_CORE_INFO("Checking to create SubTexture for profile {0}. Atlas: {1}, HasSpriteRenderer: {2}", profile.Name, (profile.AtlasTexture ? "Valid" : "Null"), e.HasComponent<SpriteRendererComponent>());
						if (profile.AtlasTexture && e.HasComponent<SpriteRendererComponent>())
						{
							int numCols = (int)(profile.AtlasTexture->GetWidth() / profile.TileSize.x);
							int frameIndex = profile.StartFrame;
							int col = frameIndex % numCols;
							int row = frameIndex / numCols;

							auto subtex = SubTexture2D::CreateFromPixelCoords(
								profile.AtlasTexture,
								col * profile.TileSize.x, row * profile.TileSize.y,
								profile.TileSize.x, profile.TileSize.y
							);

							e.GetComponent<SpriteRendererComponent>().SubTexture = subtex;
							e.GetComponent<SpriteRendererComponent>().Color = glm::vec4(1.0f);
						}
					}
				}
			},
			[](BufferStreamWriter& out, Entity e) {
				auto& ac = e.GetComponent<AnimationComponent>();

				out.WriteString(ac.CurrentStateName);
				out.WriteRaw(ac.Playing);
				out.WriteRaw(ac.Loop);

				uint32_t profileCount = (uint32_t)ac.Profiles.size();
				out.WriteRaw(profileCount);

				for (auto& profile : ac.Profiles) {
					out.WriteString(profile.Name);
					if (profile.AtlasTexture) profile.AtlasTextureUUID = profile.AtlasTexture->Handle;
					out.WriteRaw(profile.AtlasTextureUUID);
					out.WriteRaw(profile.TileSize);
					out.WriteRaw(profile.StartFrame);
					out.WriteRaw(profile.FrameCount);
					out.WriteRaw(profile.FramesPerRow);
					out.WriteRaw(profile.FrameTime);
					out.WriteRaw(profile.Loop);
				}
			},
			[](BufferStreamReader& in, Entity e) {
				AnimationComponent ac;

				in.ReadString(ac.CurrentStateName);
				in.ReadRaw(ac.Playing);
				in.ReadRaw(ac.Loop);

				uint32_t profileCount;
				in.ReadRaw(profileCount);

				for (uint32_t i = 0; i < profileCount; i++) {
					AnimationProfile profile;
					in.ReadString(profile.Name);

					in.ReadRaw(profile.AtlasTextureUUID);
					if (profile.AtlasTextureUUID != 0) {
						profile.AtlasTexture = ResourceManager::Get<Texture2D>(profile.AtlasTextureUUID);
					}

					in.ReadRaw(profile.TileSize);
					in.ReadRaw(profile.StartFrame);
					in.ReadRaw(profile.FrameCount);
					in.ReadRaw(profile.FramesPerRow);
					in.ReadRaw(profile.FrameTime);
					in.ReadRaw(profile.Loop);

					ac.Profiles.push_back(profile);
				}
				e.AddOrReplaceComponent<AnimationComponent>(ac);

				if (!ac.Profiles.empty())
				{
					auto& profile = ac.Profiles[0];
					if (ac.CurrentStateName.empty()) ac.CurrentStateName = profile.Name;

					R2D_CORE_INFO("Checking to create SubTexture for profile {0}. Atlas: {1}, HasSpriteRenderer: {2}", profile.Name, (profile.AtlasTexture ? "Valid" : "Null"), e.HasComponent<SpriteRendererComponent>());
					if (profile.AtlasTexture && e.HasComponent<SpriteRendererComponent>())
					{
						int numCols = (int)(profile.AtlasTexture->GetWidth() / profile.TileSize.x);
						int frameIndex = profile.StartFrame;
						int col = frameIndex % numCols;
						int row = frameIndex / numCols;

						auto subtex = SubTexture2D::CreateFromPixelCoords(
							profile.AtlasTexture,
							col * profile.TileSize.x, row * profile.TileSize.y,
							profile.TileSize.x, profile.TileSize.y
						);

						e.GetComponent<SpriteRendererComponent>().SubTexture = subtex;
						e.GetComponent<SpriteRendererComponent>().Color = glm::vec4(1.0f);
					}
				}
			},
			true
		});

		Register({
			"RectTransform", "UI",
			[](Entity e) { if (!e.HasComponent<RectTransformComponent>()) e.AddComponent<RectTransformComponent>(); },
			[](Entity e) { return e.HasComponent<RectTransformComponent>(); },
#ifndef R2D_DIST
			[](Entity e) {
				auto& c = e.GetComponent<RectTransformComponent>();
				glm::vec2 pos = c.GetPosition();
				if (ImGui::DragFloat2("Position", &pos[0], 0.1f)) {
					c.SetPosition(pos); e.InvalidateTransform();
				}
				glm::vec2 size = c.GetSize();
				if (ImGui::DragFloat2("Size", &size[0], 0.1f)) {
					c.SetSize(size); e.InvalidateTransform();
				}
				glm::vec2 anchorMin = c.GetAnchorMin();
				if(ImGui::DragFloat2("Anchor Min", &anchorMin[0], 0.05f, 0.0f, 1.0f)) {
					c.SetAnchorMin(anchorMin); e.InvalidateTransform();
				}
				glm::vec2 anchorMax = c.GetAnchorMax();
				if(ImGui::DragFloat2("Anchor Max", &anchorMax[0], 0.05f, 0.0f, 1.0f)) {
					c.SetAnchorMax(anchorMax); e.InvalidateTransform();
				}
				glm::vec2 pivot = c.GetPivot();
				if(ImGui::DragFloat2("Pivot", &pivot[0], 0.05f, 0.0f, 1.0f)) {
					c.SetPivot(pivot); e.InvalidateTransform();
				}
				float rotation = c.GetRotation();
				if(ImGui::DragFloat("Rotation", &rotation, 0.05f)) {
					c.SetRotation(rotation); e.InvalidateTransform();
				}
				glm::vec2 scale = c.GetScale();
				if(ImGui::DragFloat2("Scale", &scale[0], 0.05f)) {
					c.SetScale(scale); e.InvalidateTransform();
				}
				if(ImGui::DragInt("Z Index", &c.ZIndex)) {
					c.ZIndex = c.ZIndex;
				}
			},
#else
			nullptr,
#endif
			[](Entity e) { e.RemoveComponent<RectTransformComponent>(); },
			[](Entity src, Entity dst) { dst.AddOrReplaceComponent<RectTransformComponent>(src.GetComponent<RectTransformComponent>()); },
			[](YAML::Emitter& out, Entity e) {
				auto& c = e.GetComponent<RectTransformComponent>();
				out << YAML::Key << "Position" << YAML::Value << YAML::Flow << YAML::BeginSeq << c.GetPosition().x << c.GetPosition().y << YAML::EndSeq;
				out << YAML::Key << "Size" << YAML::Value << YAML::Flow << YAML::BeginSeq << c.GetSize().x << c.GetSize().y << YAML::EndSeq;
				out << YAML::Key << "AnchorMin" << YAML::Value << YAML::Flow << YAML::BeginSeq << c.GetAnchorMin().x << c.GetAnchorMin().y << YAML::EndSeq;
				out << YAML::Key << "AnchorMax" << YAML::Value << YAML::Flow << YAML::BeginSeq << c.GetAnchorMax().x << c.GetAnchorMax().y << YAML::EndSeq;
				out << YAML::Key << "Pivot" << YAML::Value << YAML::Flow << YAML::BeginSeq << c.GetPivot().x << c.GetPivot().y << YAML::EndSeq;
				out << YAML::Key << "Rotation" << YAML::Value << c.GetRotation();
				out << YAML::Key << "Scale" << YAML::Value << YAML::Flow << YAML::BeginSeq << c.GetScale().x << c.GetScale().y << YAML::EndSeq;
				out << YAML::Key << "ZIndex" << YAML::Value << c.ZIndex;
			},
			[](YAML::Node& node, Entity e) {
				auto& c = e.AddComponent<RectTransformComponent>();
				if (node["Position"]) { c.SetPosition({node["Position"][0].as<float>(), node["Position"][1].as<float>()}); }
				if (node["Size"]) { c.SetSize({node["Size"][0].as<float>(), node["Size"][1].as<float>()}); }
				if (node["AnchorMin"]) { c.SetAnchorMin({node["AnchorMin"][0].as<float>(), node["AnchorMin"][1].as<float>()}); }
				if (node["AnchorMax"]) { c.SetAnchorMax({node["AnchorMax"][0].as<float>(), node["AnchorMax"][1].as<float>()}); }
				if (node["Pivot"]) { c.SetPivot({node["Pivot"][0].as<float>(), node["Pivot"][1].as<float>()}); }
				if (node["Rotation"]) c.SetRotation(node["Rotation"].as<float>());
				if (node["Scale"]) { c.SetScale({node["Scale"][0].as<float>(), node["Scale"][1].as<float>()}); }
				if (node["ZIndex"]) c.ZIndex = node["ZIndex"].as<int>();
			},
			[](BufferStreamWriter& out, Entity e) {
				auto& rtc = e.GetComponent<RectTransformComponent>();
				out.WriteRaw(rtc.GetPosition());
				out.WriteRaw(rtc.GetSize());
				out.WriteRaw(rtc.GetScale());
				out.WriteRaw(rtc.GetRotation());
				out.WriteRaw(rtc.GetAnchorMin());
				out.WriteRaw(rtc.GetAnchorMax());
				out.WriteRaw(rtc.GetPivot());
				out.WriteRaw(rtc.ZIndex);
			},
			[](BufferStreamReader& in, Entity e) {
				RectTransformComponent rtc;

				glm::vec2 Position;
				glm::vec2 Size;
				glm::vec2 Scale;
				float Rotation;
				glm::vec2 AnchorMin;
				glm::vec2 AnchorMax;
				glm::vec2 Pivot;

				in.ReadRaw(Position);
				in.ReadRaw(Size);
				in.ReadRaw(Scale);
				in.ReadRaw(Rotation);
				in.ReadRaw(AnchorMin);
				in.ReadRaw(AnchorMax);
				in.ReadRaw(Pivot);
				in.ReadRaw(rtc.ZIndex);

				rtc.SetPosition(Position);
				rtc.SetSize(Size);
				rtc.SetScale(Scale);
				rtc.SetRotation(Rotation);
				rtc.SetAnchorMin(AnchorMin);
				rtc.SetAnchorMax(AnchorMax);
				rtc.SetPivot(Pivot);

				e.AddOrReplaceComponent<RectTransformComponent>(rtc);
			},
			true
		});

		Register({
			"Button", "UI",
			[](Entity e) { if (!e.HasComponent<ButtonComponent>()) e.AddComponent<ButtonComponent>(); },
			[](Entity e) { return e.HasComponent<ButtonComponent>(); },
#ifndef R2D_DIST
			[](Entity e) {
				auto& c = e.GetComponent<ButtonComponent>();
				// Utilitzem &c.Color[0] per consistència i evitar problemes d'includes de GLM
				ImGui::ColorEdit4("Normal Color", &c.NormalColor[0]);
				ImGui::ColorEdit4("Hovered Color", &c.HoveredColor[0]);
				ImGui::ColorEdit4("Pressed Color", &c.PressedColor[0]);
			},
#else
			nullptr,
#endif
			[](Entity e) { e.RemoveComponent<ButtonComponent>(); },
			[](Entity src, Entity dst) { dst.AddOrReplaceComponent<ButtonComponent>(src.GetComponent<ButtonComponent>()); },
			[](YAML::Emitter& out, Entity e) {
				auto& c = e.GetComponent<ButtonComponent>();
				out << YAML::Key << "NormalColor" << YAML::Value << YAML::Flow << YAML::BeginSeq << c.NormalColor.r << c.NormalColor.g << c.NormalColor.b << c.NormalColor.a << YAML::EndSeq;
				out << YAML::Key << "HoveredColor" << YAML::Value << YAML::Flow << YAML::BeginSeq << c.HoveredColor.r << c.HoveredColor.g << c.HoveredColor.b << c.HoveredColor.a << YAML::EndSeq;
				out << YAML::Key << "PressedColor" << YAML::Value << YAML::Flow << YAML::BeginSeq << c.PressedColor.r << c.PressedColor.g << c.PressedColor.b << c.PressedColor.a << YAML::EndSeq;
			},
			[](YAML::Node& node, Entity e) {
				auto& c = e.AddComponent<ButtonComponent>();
				if (node["NormalColor"]) { c.NormalColor.r = node["NormalColor"][0].as<float>(); c.NormalColor.g = node["NormalColor"][1].as<float>(); c.NormalColor.b = node["NormalColor"][2].as<float>(); c.NormalColor.a = node["NormalColor"][3].as<float>(); }
				if (node["HoveredColor"]) { c.HoveredColor.r = node["HoveredColor"][0].as<float>(); c.HoveredColor.g = node["HoveredColor"][1].as<float>(); c.HoveredColor.b = node["HoveredColor"][2].as<float>(); c.HoveredColor.a = node["HoveredColor"][3].as<float>(); }
				if (node["PressedColor"]) { c.PressedColor.r = node["PressedColor"][0].as<float>(); c.PressedColor.g = node["PressedColor"][1].as<float>(); c.PressedColor.b = node["PressedColor"][2].as<float>(); c.PressedColor.a = node["PressedColor"][3].as<float>(); }
			},
			[](BufferStreamWriter& out, Entity e) {
				auto& bc = e.GetComponent<ButtonComponent>();
				out.WriteRaw(bc.NormalColor);
				out.WriteRaw(bc.HoveredColor);
				out.WriteRaw(bc.PressedColor);
			},
			[](BufferStreamReader& in, Entity e) {
				ButtonComponent bc;
				in.ReadRaw(bc.NormalColor);
				in.ReadRaw(bc.HoveredColor);
				in.ReadRaw(bc.PressedColor);
				e.AddOrReplaceComponent<ButtonComponent>(bc);
			},
			true
		});

		Register({
			"AudioSourceComponent", "Audio",
			[](Entity e) { if (!e.HasComponent<AudioSourceComponent>()) e.AddComponent<AudioSourceComponent>(); },
			[](Entity e) { return e.HasComponent<AudioSourceComponent>(); },
#ifndef R2D_DIST
			[](Entity e) {
				auto& component = e.GetComponent<AudioSourceComponent>();
				
				ImGui::DragFloat("Volume", &component.Volume, 0.05f, 0.0f, 10.0f);
				ImGui::DragFloat("Pitch", &component.Pitch, 0.05f, 0.1f, 5.0f);
				ImGui::Checkbox("Looping", &component.Looping);
				ImGui::Checkbox("PlayOnAwake", &component.PlayOnAwake);
				
				ImGui::Text("Audio Clip UUID: %llu", component.AudioClipUUID);
				
				// Drag & Drop per Asset d'Audio
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const char* path = (const char*)payload->Data;
						std::filesystem::path filepath = path;

						if (filepath.extension() == ".wav" || filepath.extension() == ".mp3")
						{
							Ref<AudioClip> clip = ResourceManager::Get<AudioClip>(filepath);
							if (clip)
							{
								component.AudioClipUUID = clip->Handle;
								component.Clip = clip;
							}
						}
					}
					ImGui::EndDragDropTarget();
				}
			},
#else
			nullptr,
#endif
			[](Entity e) { e.RemoveComponent<AudioSourceComponent>(); },
			[](Entity src, Entity dst) { dst.AddOrReplaceComponent<AudioSourceComponent>(src.GetComponent<AudioSourceComponent>()); },
			[](YAML::Emitter& out, Entity e) {
				auto& ac = e.GetComponent<AudioSourceComponent>();
				out << YAML::Key << "AudioClipUUID" << YAML::Value << (uint64_t)ac.AudioClipUUID;
				out << YAML::Key << "Volume" << YAML::Value << ac.Volume;
				out << YAML::Key << "Pitch" << YAML::Value << ac.Pitch;
				out << YAML::Key << "Looping" << YAML::Value << ac.Looping;
				out << YAML::Key << "PlayOnAwake" << YAML::Value << ac.PlayOnAwake;
			},
			[](YAML::Node& node, Entity e) {
				auto& ac = e.AddComponent<AudioSourceComponent>();
				if (node["AudioClipUUID"]) ac.AudioClipUUID = node["AudioClipUUID"].as<uint64_t>();
				if (node["Volume"]) ac.Volume = node["Volume"].as<float>();
				if (node["Pitch"]) ac.Pitch = node["Pitch"].as<float>();
				if (node["Looping"]) ac.Looping = node["Looping"].as<bool>();
				if (node["PlayOnAwake"]) ac.PlayOnAwake = node["PlayOnAwake"].as<bool>();

				if (ac.AudioClipUUID != 0) {
					ac.Clip = ResourceManager::Get<AudioClip>(ac.AudioClipUUID);
				}
			},
			[](BufferStreamWriter& out, Entity e) {
				auto& ac = e.GetComponent<AudioSourceComponent>();
				out.WriteRaw(ac.AudioClipUUID);
				out.WriteRaw(ac.Volume);
				out.WriteRaw(ac.Pitch);
				out.WriteRaw(ac.Looping);
				out.WriteRaw(ac.PlayOnAwake);
			},
			[](BufferStreamReader& in, Entity e) {
				AudioSourceComponent ac;
				in.ReadRaw(ac.AudioClipUUID);
				in.ReadRaw(ac.Volume);
				in.ReadRaw(ac.Pitch);
				in.ReadRaw(ac.Looping);
				in.ReadRaw(ac.PlayOnAwake);

				if (ac.AudioClipUUID != 0) {
					ac.Clip = ResourceManager::Get<AudioClip>(ac.AudioClipUUID);
				}
				e.AddOrReplaceComponent<AudioSourceComponent>(ac);
			},
			true
		});
	}
}

