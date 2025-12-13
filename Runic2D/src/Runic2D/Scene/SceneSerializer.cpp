#include "R2Dpch.h"
#include "SceneSerializer.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

#include "Entity.h"
#include "Component.h"

#define YAML_LOAD(node, key, target) if (node[key]) target = node[key].as<decltype(target)>();

namespace YAML {

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

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: m_Scene(scene)
	{
	}

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
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

		if(entity.HasComponent<SpriteRendererComponent>())
		{
			auto& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();

			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap;

			out << YAML::Key << "Color" << YAML::Value << spriteRenderer.Color;

			if (spriteRenderer.Texture)
			{
				out << YAML::Key << "TexturePath" << YAML::Value << spriteRenderer.Texture->GetPath();
			}

			out << YAML::Key << "TilingFactor" << YAML::Value << spriteRenderer.TilingFactor;

			out << YAML::EndMap;
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

			SerializeEntity(out, entity);
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
						std::string texturePath = spriteRendererComponent["TexturePath"].as<std::string>();
						src.Texture = Texture2D::Create(texturePath);
					}

					if (spriteRendererComponent["TilingFactor"])
						src.TilingFactor = spriteRendererComponent["TilingFactor"].as<float>();
				}
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