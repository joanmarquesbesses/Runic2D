#include "R2Dpch.h"
#include "SceneSerializer.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

#include "Entity.h"
#include "Component.h"
#include "Runic2D/Project/Project.h"
#include "Runic2D/Assets/ResourceManager.h"

#include <unordered_map>

#define YAML_LOAD(node, key, target) if (node[key]) target = node[key].as<decltype(target)>();

namespace YAML {

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3> {
		static Node encode(const glm::vec3& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs) {
			if (!node.IsSequence() || node.size() != 3)
				return false;
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4> {
		static Node encode(const glm::vec4& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs) {
			if (!node.IsSequence() || node.size() != 4)
				return false;
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};
} // namespace YAML

namespace Runic2D {

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& vec)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << vec.x << vec.y << vec.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& vec)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << vec.x << vec.y << vec.z << vec.w << YAML::EndSeq;
		return out;
	}

	static std::string RigidBody2DBodyTypeToString(Rigidbody2DComponent::BodyType bodyType)
	{
		switch (bodyType)
		{
		case Rigidbody2DComponent::BodyType::Static:    return "Static";
		case Rigidbody2DComponent::BodyType::Dynamic:   return "Dynamic";
		case Rigidbody2DComponent::BodyType::Kinematic: return "Kinematic";
		}

		R2D_CORE_ASSERT(false, "Unknown body type");
		return {};
	}

	static Rigidbody2DComponent::BodyType RigidBody2DBodyTypeFromString(const std::string& bodyTypeString)
	{
		if (bodyTypeString == "Static")    return Rigidbody2DComponent::BodyType::Static;
		if (bodyTypeString == "Dynamic")   return Rigidbody2DComponent::BodyType::Dynamic;
		if (bodyTypeString == "Kinematic") return Rigidbody2DComponent::BodyType::Kinematic;

		R2D_CORE_ASSERT(false, "Unknown body type");
		return Rigidbody2DComponent::BodyType::Static;
	}

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: m_Scene(scene)
	{
	}

	static void SerializeEntity(YAML::Emitter& out, Entity entity, Scene* scene)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "EntityID" << YAML::Value << entity.GetUUID();

		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;

			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "Tag" << YAML::Value << tag;
			out << YAML::EndMap;
		}

		if(entity.HasComponent<TransformComponent>())
		{
			auto& transform = entity.GetComponent<TransformComponent>();

			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "Translation" << YAML::Value << transform.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << transform.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << transform.Scale;
			out << YAML::EndMap;
		}

		if (entity.HasComponent<RelationshipComponent>())
		{
			out << YAML::Key << "RelationshipComponent";
			out << YAML::BeginMap;

			auto& rc = entity.GetComponent<RelationshipComponent>();

			UUID parentUUID = 0;
			if (rc.Parent != entt::null)
			{
				Entity parent = { rc.Parent, scene };
				parentUUID = parent.GetUUID();
			}
			out << YAML::Key << "Parent" << YAML::Value << parentUUID;
			out << YAML::EndMap;
		}

		if(entity.HasComponent<CameraComponent>())
		{
			auto& cameraComponent = entity.GetComponent<CameraComponent>();

			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; // CameraComponent

			auto& camera = cameraComponent.Camera;
			out << YAML::Key << "Camera";
			out << YAML::BeginMap; // Camera
			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
			out << YAML::EndMap; // Camera

			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;

			out << YAML::EndMap; // CameraComponent
		}

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			auto& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();

			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap;

			out << YAML::Key << "Color" << YAML::Value << spriteRenderer.Color;

			if (spriteRenderer.Texture)
			{
				std::filesystem::path texturePath = spriteRenderer.Texture->GetPath();
				std::string texturePathString = texturePath.string();

				std::replace(texturePathString.begin(), texturePathString.end(), '\\', '/');

				if (Project::GetActive())
				{
					std::filesystem::path assetPath = Project::GetAssetDirectory();
					std::string assetPathString = assetPath.string();

					std::replace(assetPathString.begin(), assetPathString.end(), '\\', '/');

					size_t pos = texturePathString.find(assetPathString);

					if (pos != std::string::npos)
					{
						texturePathString = texturePathString.substr(pos + assetPathString.length());

						if (texturePathString.size() > 0 && (texturePathString[0] == '/' || texturePathString[0] == '\\'))
						{
							texturePathString = texturePathString.substr(1);
						}
					}
					else
					{
						std::filesystem::path absTexture = std::filesystem::absolute(texturePath);
						std::filesystem::path absAssets = std::filesystem::absolute(assetPath);

						std::string absTexStr = absTexture.string();
						std::string absAssStr = absAssets.string();

						std::replace(absTexStr.begin(), absTexStr.end(), '\\', '/');
						std::replace(absAssStr.begin(), absAssStr.end(), '\\', '/');

						pos = absTexStr.find(absAssStr);
						if (pos != std::string::npos)
						{
							texturePathString = absTexStr.substr(pos + absAssStr.length());
							if (texturePathString.size() > 0 && texturePathString[0] == '/')
								texturePathString = texturePathString.substr(1);
						}
					}
				}

				out << YAML::Key << "TexturePath" << YAML::Value << texturePathString;
			}

			out << YAML::Key << "TilingFactor" << YAML::Value << spriteRenderer.TilingFactor;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<CircleRendererComponent>())
		{
			out << YAML::Key << "CircleRendererComponent";
			out << YAML::BeginMap;

			auto& circleRendererComponent = entity.GetComponent<CircleRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << circleRendererComponent.Color;
			out << YAML::Key << "Thickness" << YAML::Value << circleRendererComponent.Thickness;
			out << YAML::Key << "Fade" << YAML::Value << circleRendererComponent.Fade;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			out << YAML::Key << "Rigidbody2DComponent";
			out << YAML::BeginMap; // Rigidbody2DComponent

			auto& rb2dComponent = entity.GetComponent<Rigidbody2DComponent>();
			out << YAML::Key << "BodyType" << YAML::Value << RigidBody2DBodyTypeToString(rb2dComponent.Type);
			out << YAML::Key << "FixedRotation" << YAML::Value << rb2dComponent.FixedRotation;

			out << YAML::EndMap; // Rigidbody2DComponent
		}

		if (entity.HasComponent<BoxCollider2DComponent>())
		{
			out << YAML::Key << "BoxCollider2DComponent";
			out << YAML::BeginMap; // BoxCollider2DComponent

			auto& bc2dComponent = entity.GetComponent<BoxCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << bc2dComponent.Offset;
			out << YAML::Key << "Size" << YAML::Value << bc2dComponent.Size;
			out << YAML::Key << "Density" << YAML::Value << bc2dComponent.Density;
			out << YAML::Key << "Friction" << YAML::Value << bc2dComponent.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << bc2dComponent.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << bc2dComponent.RestitutionThreshold;

			out << YAML::EndMap; // BoxCollider2DComponent
		}

		if (entity.HasComponent<CircleCollider2DComponent>())
		{
			out << YAML::Key << "CircleCollider2DComponent";
			out << YAML::BeginMap; // CircleCollider2DComponent

			auto& cc2dComponent = entity.GetComponent<CircleCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << cc2dComponent.Offset;
			out << YAML::Key << "Radius" << YAML::Value << cc2dComponent.Radius;
			out << YAML::Key << "Density" << YAML::Value << cc2dComponent.Density;
			out << YAML::Key << "Friction" << YAML::Value << cc2dComponent.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << cc2dComponent.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << cc2dComponent.RestitutionThreshold;

			out << YAML::EndMap; // CircleCollider2DComponent
		}

		out << YAML::EndMap; // Entity
	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Scene Name";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		
		auto view = m_Scene->m_Registry.view<TagComponent>();

		for (auto it = view.rbegin(); it != view.rend(); ++it) // Iterate in reverse to serialize in creation order
		{
			Entity entity = { *it, m_Scene.get() };
			if (!entity) continue;

			SerializeEntity(out, entity, m_Scene.get());
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();

	}

	void SceneSerializer::SerializeRuntime(const std::string& filepath)
	{
		R2D_ASSERT(false, "Not Implemented!");
	}

	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		YAML::Node data = YAML::LoadFile(filepath);
		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();
		R2D_CORE_TRACE("Deserializing scene '{0}'", sceneName);

		auto entities = data["Entities"];

		std::unordered_map<UUID, UUID> parentMap;

		if(entities)
		{

			for (auto entityNode : entities)
			{
				uint64_t uuid = entityNode["EntityID"].as<uint64_t>();
				std::string name;
				auto tagComponent = entityNode["TagComponent"];
				if (tagComponent)
					name = tagComponent["Tag"].as<std::string>();
				R2D_CORE_TRACE("Deserialized entity with ID = {0}, name = {1}", uuid, name);

				Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuid, name);

				auto transformComponent = entityNode["TransformComponent"];
				if (transformComponent)
				{
					// entities always have transform component
					auto& tc = deserializedEntity.GetComponent<TransformComponent>();
					YAML_LOAD(transformComponent, "Translation", tc.Translation);
					YAML_LOAD(transformComponent, "Rotation", tc.Rotation);
					YAML_LOAD(transformComponent, "Scale", tc.Scale);
				}

				auto relationshipComponent = entityNode["RelationshipComponent"];
				if (relationshipComponent)
				{
					UUID parentUUID = relationshipComponent["Parent"].as<uint64_t>();

					if (parentUUID != 0)
					{
						parentMap[deserializedEntity.GetUUID()] = parentUUID;
					}
				}

				auto cameraComponent = entityNode["CameraComponent"];
				if (cameraComponent)
				{
					auto& cc = deserializedEntity.AddComponent<CameraComponent>();
					auto cameraNode = cameraComponent["Camera"];

					if (cameraNode["ProjectionType"])
						cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraNode["ProjectionType"].as<int>());

					if (cameraNode["PerspectiveFOV"])
						cc.Camera.SetPerspectiveVerticalFOV(cameraNode["PerspectiveFOV"].as<float>());
					if (cameraNode["PerspectiveNear"])
						cc.Camera.SetPerspectiveNearClip(cameraNode["PerspectiveNear"].as<float>());
					if (cameraNode["PerspectiveFar"])
						cc.Camera.SetPerspectiveFarClip(cameraNode["PerspectiveFar"].as<float>());

					if (cameraNode["OrthographicSize"])
						cc.Camera.SetOrthographicSize(cameraNode["OrthographicSize"].as<float>());
					if (cameraNode["OrthographicNear"])
					cc.Camera.SetOrthographicNearClip(cameraNode["OrthographicNear"].as<float>());
					if (cameraNode["OrthographicFar"])
					cc.Camera.SetOrthographicFarClip(cameraNode["OrthographicFar"].as<float>());

					YAML_LOAD(cameraComponent, "Primary", cc.Primary);
					YAML_LOAD(cameraComponent, "FixedAspectRatio", cc.FixedAspectRatio);
				}

				auto spriteRendererComponent = entityNode["SpriteRendererComponent"];
				if (spriteRendererComponent)
				{
					auto& src = deserializedEntity.AddComponent<SpriteRendererComponent>();
					YAML_LOAD(spriteRendererComponent, "Color", src.Color);

					if (spriteRendererComponent["TexturePath"])
					{
						std::string texturePathString = spriteRendererComponent["TexturePath"].as<std::string>();
						std::filesystem::path path = Project::GetAssetFileSystemPath(texturePathString);

						if (std::filesystem::exists(path))
						{
							src.Texture = ResourceManager::Get<Texture2D>(path);
						}
						else
						{
							R2D_CORE_WARN("Texture not found: {0}", path.string());
						}
					}

					if (spriteRendererComponent["TilingFactor"])
						src.TilingFactor = spriteRendererComponent["TilingFactor"].as<float>();
				}

				auto circleRendererComponent = entityNode["CircleRendererComponent"];
				if (circleRendererComponent)
				{
					auto& crc = deserializedEntity.AddComponent<CircleRendererComponent>();
					YAML_LOAD(circleRendererComponent, "Color", crc.Color);
					YAML_LOAD(circleRendererComponent, "Thickness", crc.Thickness);
					YAML_LOAD(circleRendererComponent, "Fade", crc.Fade);
				}

				auto rigidbody2DComponent = entityNode["Rigidbody2DComponent"];
				if (rigidbody2DComponent)
				{
					auto& rb2d = deserializedEntity.AddComponent<Rigidbody2DComponent>();
					rb2d.Type = RigidBody2DBodyTypeFromString(rigidbody2DComponent["BodyType"].as<std::string>());
					rb2d.FixedRotation = rigidbody2DComponent["FixedRotation"].as<bool>();
				}

				auto boxCollider2DComponent = entityNode["BoxCollider2DComponent"];
				if (boxCollider2DComponent)
				{
					auto& bc2d = deserializedEntity.AddComponent<BoxCollider2DComponent>();
					bc2d.Offset = boxCollider2DComponent["Offset"].as<glm::vec2>();
					bc2d.Size = boxCollider2DComponent["Size"].as<glm::vec2>();
					bc2d.Density = boxCollider2DComponent["Density"].as<float>();
					bc2d.Friction = boxCollider2DComponent["Friction"].as<float>();
					bc2d.Restitution = boxCollider2DComponent["Restitution"].as<float>();
					bc2d.RestitutionThreshold = boxCollider2DComponent["RestitutionThreshold"].as<float>();
				}

				auto circleCollider2DComponent = entityNode["CircleCollider2DComponent"];
				if (circleCollider2DComponent)
				{
					auto& cc2d = deserializedEntity.AddComponent<CircleCollider2DComponent>();
					cc2d.Offset = circleCollider2DComponent["Offset"].as<glm::vec2>();
					cc2d.Radius = circleCollider2DComponent["Radius"].as<float>();
					cc2d.Density = circleCollider2DComponent["Density"].as<float>();
					cc2d.Friction = circleCollider2DComponent["Friction"].as<float>();
					cc2d.Restitution = circleCollider2DComponent["Restitution"].as<float>();
					cc2d.RestitutionThreshold = circleCollider2DComponent["RestitutionThreshold"].as<float>();
				}
			}
		}

		for (auto const& [childUUID, parentUUID] : parentMap)
		{
			Entity child = m_Scene->GetEntityByUUID(childUUID);
			Entity parent = m_Scene->GetEntityByUUID(parentUUID);

			if (child && parent)
			{
				auto& tc = child.GetComponent<TransformComponent>();
				glm::vec3 oldTranslation = tc.Translation;
				glm::vec3 oldRotation = tc.Rotation;
				glm::vec3 oldScale = tc.Scale;

				m_Scene->ParentEntity(child, parent);

				tc.Translation = oldTranslation;
				tc.Rotation = oldRotation;
				tc.Scale = oldScale;
				tc.IsDirty = true;
			}
		}

		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
	{
		R2D_ASSERT(false, "Not Implemented!");
		return false;
	}
} // namespace Runic2D