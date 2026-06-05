#include <thread>
#include <chrono>
#include "R2Dpch.h"
#include "SceneSerializer.h"
#include "SceneManager.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

#include "Entity.h"
#include "Component.h"
#include "ComponentRegistry.h"
#include "Runic2D/Project/Project.h"
#include "Runic2D/Assets/ResourceManager.h"
#include "Runic2D/Scripting/ScriptEngine.h"

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
			out << YAML::Key << "Translation" << YAML::Value << transform.GetTranslation();
			out << YAML::Key << "Rotation" << YAML::Value << transform.GetRotation();
			out << YAML::Key << "Scale" << YAML::Value << transform.GetScale();
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

		for (const auto& desc : ComponentRegistry::GetAll())
		{
			if (desc.HasOnEntity(entity) && desc.Serialize)
			{
				out << YAML::Key << desc.Name;
				out << YAML::BeginMap;
				desc.Serialize(out, entity); // La DLL sap quines variables escriure!
				out << YAML::EndMap;
			}
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
		if (!std::filesystem::exists(filepath))
		{
			R2D_CORE_ERROR("No s'ha pogut carregar l'escena: El fitxer no existeix ({0})", filepath);
			return false;
		}

		YAML::Node data = YAML::LoadFile(filepath);
		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();
		R2D_CORE_TRACE("Deserializing scene '{0}'", sceneName);

		auto entities = data["Entities"];

		std::unordered_map<UUID, UUID> parentMap;

		if(entities)
		{
			size_t entityCount = entities.size();
			size_t currentEntity = 0;

			for (auto entityNode : entities)
			{
				SceneManager::SetLoadingProgress((float)currentEntity / (float)entityCount);
				currentEntity++;
				

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
					auto& tc = deserializedEntity.GetComponent<TransformComponent>();
					glm::vec3 tempTranslation;
					glm::vec3 tempRotation;
					glm::vec3 tempScale;
					
					YAML_LOAD(transformComponent, "Translation", tempTranslation);
					tc.SetTranslation(tempTranslation);
					YAML_LOAD(transformComponent, "Rotation", tempRotation);
					tc.SetRotation(tempRotation);
					YAML_LOAD(transformComponent, "Scale", tempScale);
					tc.SetScale(tempScale);
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

				for (const auto& desc : ComponentRegistry::GetAll())
				{
					auto componentNode = entityNode[desc.Name];
					if (componentNode && desc.Deserialize)
					{
						desc.Deserialize(componentNode, deserializedEntity);
					}
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
				glm::vec3 oldTranslation = tc.GetTranslation();
				glm::vec3 oldRotation = tc.GetRotation();
				glm::vec3 oldScale = tc.GetScale();

				child.SetParent(parent);

				tc.SetTranslation(oldTranslation);
				tc.SetRotation(oldRotation);
				tc.SetScale(oldScale);
			}
		}

		SceneManager::SetLoadingProgress(1.0f);
		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
	{
		R2D_ASSERT(false, "Not Implemented!");
		return false;
	}
} // namespace Runic2D