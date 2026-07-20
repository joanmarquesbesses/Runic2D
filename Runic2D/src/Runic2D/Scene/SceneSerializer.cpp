#include <thread>
#include <chrono>
#include "R2Dpch.h"
#include "SceneSerializer.h"

#include "Entity.h"
#include "Components/CoreComponents.h"
#include "Components/PhysicsComponents.h"
#include "Components/ComponentRegistry.h"

#include "SceneManager.h"
#include "Runic2D/Project/Project.h"

#include "Runic2D/Assets/ResourceManager.h"
#include "Runic2D/Scripting/ScriptEngine.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

#include <unordered_map>

#include "lz4.h"

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

	static void SerializeEntityBinary(BufferStreamWriter& out, Entity entity, Scene* scene)
	{
		// 1. Guardem el UUID (sempre hi és)
		out.WriteRaw(entity.GetUUID());
		// 2. TagComponent
		bool hasTag = entity.HasComponent<TagComponent>();
		out.WriteRaw(hasTag);
		if (hasTag) {
			out.WriteString(entity.GetComponent<TagComponent>().Tag);
		}
		// 3. TransformComponent
		bool hasTransform = entity.HasComponent<TransformComponent>();
		out.WriteRaw(hasTransform);
		if (hasTransform) {
			auto& transform = entity.GetComponent<TransformComponent>();
			out.WriteRaw(transform.GetTranslation());
			out.WriteRaw(transform.GetRotation());
			out.WriteRaw(transform.GetScale());
		}
		// 4. RelationshipComponent
		bool hasRelationship = entity.HasComponent<RelationshipComponent>();
		out.WriteRaw(hasRelationship);
		if (hasRelationship) {
			auto& rc = entity.GetComponent<RelationshipComponent>();
			UUID parentUUID = 0;
			if (rc.Parent != entt::null) {
				Entity parent = { rc.Parent, scene };
				parentUUID = parent.GetUUID();
			}
			out.WriteRaw(parentUUID);
		}
		// 5. La resta de components via ComponentRegistry
		for (const auto& desc : ComponentRegistry::GetAll())
		{
			if (desc.HasOnEntity(entity) && desc.SerializeBinary)
			{
				// Hashegem el nom per tenir un ID numèric únic i super ràpid
				uint32_t componentHash = (uint32_t)std::hash<std::string>{}(desc.Name);
				out.WriteRaw(componentHash);

				// Cridem al callback binari del registre!
				desc.SerializeBinary(out, entity);
			}
		}
		// Posem un '0' al final per dir-li a la lectura: "S'han acabat els components d'aquesta entitat!"
		out.WriteRaw((uint32_t)0);
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

	void SceneSerializer::SerializeBinary(const std::string& filepath)
	{
		BufferStreamWriter out;

		// 1. Escrivim capçalera 
		out.WriteString("R2DB");
		// 2. Escrivim nombre total d'entitats
		auto view = m_Scene->m_Registry.view<entt::entity>();
		uint32_t entityCount = (uint32_t)view.size();
		out.WriteRaw(entityCount);
		// 3. Serialitzem entitat per entitat en memòria cau
		for (auto it = view.rbegin(); it != view.rend(); ++it)
		{
			Entity entity = { *it, m_Scene.get() };
			if (!entity) continue;
			SerializeEntityBinary(out, entity, m_Scene.get());
		}
		// 4. Tenim tota l'escena crua a la RAM, ara comprimim amb LZ4
		Buffer rawBuffer = out.GetBuffer();

		// Calculem l'espai màxim que pot ocupar un cop comprimit
		int maxCompressedSize = LZ4_compressBound((int)rawBuffer.Size);
		Buffer compressedBuffer(maxCompressedSize);
		// Comprimim (la màgia de la velocitat AAA)
		int compressedSize = LZ4_compress_default(
			(const char*)rawBuffer.Data,
			(char*)compressedBuffer.Data,
			(int)rawBuffer.Size,
			maxCompressedSize
		);
		// 5. Ho bolquem al disc dur final!
		std::ofstream fout(filepath, std::ios::binary);
		// Vital: Guardem la mida original que tenia la RAM per saber com de gran ha de ser en descomprimir-ho
		fout.write((char*)&rawBuffer.Size, sizeof(uint32_t));
		// Guardem l'escena comprimida
		fout.write((char*)compressedBuffer.Data, compressedSize);
		fout.close();
		// Neteja
		R2D_CORE_INFO("Escena guardada en Binari! Mida original: {0} bytes -> Comprimida: {1} bytes", rawBuffer.Size, compressedSize);
		rawBuffer.Release();
		compressedBuffer.Release();
	}

	void SceneSerializer::SerializeRuntime(const std::string& filepath)
	{
		R2D_ASSERT(false, "Not Implemented!");
	}

	static void SerializeEntityRecursively(YAML::Emitter& out, Entity entity, Scene* scene)
	{
		if (!entity) return;

		SerializeEntity(out, entity, scene);

		if (entity.HasComponent<RelationshipComponent>())
		{
			auto& rc = entity.GetComponent<RelationshipComponent>();
			entt::entity currentChildHandle = rc.FirstChild;
			while (currentChildHandle != entt::null)
			{
				Entity child = { currentChildHandle, scene };
				SerializeEntityRecursively(out, child, scene);
				currentChildHandle = child.GetComponent<RelationshipComponent>().NextSibling;
			}
		}
	}

	void SceneSerializer::SerializeEntityToPrefab(Entity entity, const std::filesystem::path& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Prefab" << YAML::Value << "Runic2D Prefab";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		SerializeEntityRecursively(out, entity, entity.GetScene());

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();

		R2D_CORE_INFO("Guardat Prefab a: {0}", filepath.string());
	}

	Entity SceneSerializer::DeserializePrefabToScene(const std::filesystem::path& filepath, Scene* scene)
	{
		YAML::Node data;
		try {
			data = YAML::LoadFile(filepath.string());
		}
		catch (YAML::ParserException& e) {
			R2D_CORE_ERROR("Error llegint el Prefab '{0}': {1}", filepath.string(), e.what());
			return {};
		}

		if (!data["Prefab"]) return {};

		auto entities = data["Entities"];
		if (!entities) return {};

		std::unordered_map<UUID, Entity> newEntityMap;
		std::unordered_map<UUID, UUID> oldToNewUUIDs;
		Entity rootEntity;

		std::unordered_map<UUID, UUID> parentMap;

		for (auto entityNode : entities)
		{
			uint64_t oldUUID = entityNode["EntityID"].as<uint64_t>();

			std::string name;
			auto tagComponent = entityNode["TagComponent"];
			if (tagComponent) name = tagComponent["Tag"].as<std::string>();

			// Generem un NOU UUID
			UUID newUUID = UUID();
			Entity deserializedEntity = scene->CreateEntityWithUUID(newUUID, name);

			oldToNewUUIDs[oldUUID] = newUUID;
			newEntityMap[oldUUID] = deserializedEntity;

			if (!rootEntity)
			{
				rootEntity = deserializedEntity; // La primera entitat a la llista sempre es el Root del Prefab
			}

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
				UUID oldParentUUID = relationshipComponent["Parent"].as<uint64_t>();

				if (oldParentUUID != 0)
				{
					parentMap[deserializedEntity.GetUUID()] = oldParentUUID;
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

		// Re-enllaçar pare-fill fent servir el diccionari antic->nou
		for (auto const& [childNewUUID, oldParentUUID] : parentMap)
		{
			if (oldToNewUUIDs.find(oldParentUUID) != oldToNewUUIDs.end())
			{
				UUID newParentUUID = oldToNewUUIDs[oldParentUUID];
				Entity child = scene->GetEntityByUUID(childNewUUID);
				Entity parent = scene->GetEntityByUUID(newParentUUID);

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
		}

		R2D_CORE_INFO("Instanciat Prefab '{0}' a l'Escena!", filepath.filename().string());

		return rootEntity;
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

	bool SceneSerializer::DeserializeBinary(const std::string& filepath)
	{
		std::ifstream stream(filepath, std::ios::binary);
		if (!stream)
		{
			R2D_CORE_ERROR("No s'ha pogut obrir l'arxiu binari: {0}", filepath);
			return false;
		}

		// 1. Llegim la mida original de RAM que ocupava!
		uint32_t uncompressedSize;
		stream.read((char*)&uncompressedSize, sizeof(uint32_t));

		// 2. Llegim les dades LZ4 comprimides
		stream.seekg(0, std::ios::end);
		std::streamsize compressedSize = stream.tellg() - (std::streamsize)sizeof(uint32_t);
		stream.seekg(sizeof(uint32_t), std::ios::beg);

		Buffer compressedBuffer((uint32_t)compressedSize);
		stream.read((char*)compressedBuffer.Data, compressedSize);
		stream.close();

		// 3. Descomprimim (velocitat de la llum)
		Buffer rawBuffer(uncompressedSize);
		int decompressedSize = LZ4_decompress_safe(
			(const char*)compressedBuffer.Data,
			(char*)rawBuffer.Data,
			(int)compressedSize,
			(int)uncompressedSize
		);

		if (decompressedSize < 0)
		{
			R2D_CORE_ERROR("Error descomprimint l'escena binària!");
			compressedBuffer.Release();
			rawBuffer.Release();
			return false;
		}

		BufferStreamReader in(rawBuffer);

		// 4. Verifiquem Header (Magic Number)
		std::string header;
		in.ReadString(header);
		if (header != "R2DB")
		{
			R2D_CORE_ERROR("Arxiu invàlid o corrupte!");
			return false;
		}

		// 5. Creem les entitats
		uint32_t entityCount;
		in.ReadRaw(entityCount);

		std::unordered_map<UUID, UUID> parentMap;

		for (uint32_t i = 0; i < entityCount; i++)
		{
			UUID uuid;
			in.ReadRaw(uuid);

			std::string name = "Entity";
			bool hasTag;
			in.ReadRaw(hasTag);
			if (hasTag) {
				in.ReadString(name);
			}

			// CREEM L'ENTITAT VUIDA AMB EL SEU ID
			Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuid, name);

			bool hasTransform;
			in.ReadRaw(hasTransform);
			if (hasTransform) {
				auto& tc = deserializedEntity.GetComponent<TransformComponent>();
				glm::vec3 translation, rotation, scale;
				in.ReadRaw(translation);
				in.ReadRaw(rotation);
				in.ReadRaw(scale);
				tc.SetTranslation(translation);
				tc.SetRotation(rotation);
				tc.SetScale(scale);
			}

			bool hasRelationship;
			in.ReadRaw(hasRelationship);
			if (hasRelationship) {
				UUID parentUUID;
				in.ReadRaw(parentUUID);
				if (parentUUID != 0) {
					parentMap[uuid] = parentUUID;
				}
			}

			// AQUI LLEGIM LA RESTA DE COMPONENTS QUE VAS CONFIGURAR!
			while (true)
			{
				uint32_t componentHash;
				in.ReadRaw(componentHash);

				if (componentHash == 0) // El nostre 0 indicava final d'entitat!
					break;

				bool found = false;
				for (const auto& desc : ComponentRegistry::GetAll())
				{
					if ((uint32_t)std::hash<std::string>{}(desc.Name) == componentHash)
					{
						if (desc.DeserializeBinary)
							desc.DeserializeBinary(in, deserializedEntity);
						found = true;
						break;
					}
				}

				R2D_CORE_ASSERT(found, "Corrupció: Component desconegut a l'arxiu binari!");
			}
		}

		// 6. Restaurem l'arbre de Relacions (Pares i Fills)
		for (auto const& [childUUID, parentUUID] : parentMap)
		{
			Entity child = m_Scene->GetEntityByUUID(childUUID);
			Entity parent = m_Scene->GetEntityByUUID(parentUUID);

			if (child && parent)
			{
				auto& tc = child.GetComponent<TransformComponent>();
				glm::vec3 oldTrans = tc.GetTranslation();
				glm::vec3 oldRot = tc.GetRotation();
				glm::vec3 oldScale = tc.GetScale();

				child.SetParent(parent);

				tc.SetTranslation(oldTrans);
				tc.SetRotation(oldRot);
				tc.SetScale(oldScale);
			}
		}

		// Neteja final
		compressedBuffer.Release();
		rawBuffer.Release();

		R2D_CORE_INFO("Escena BINÀRIA deserialitzada correctament! ({0} entitats)", entityCount);
		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
	{
		R2D_ASSERT(false, "Not Implemented!");
		return false;
	}
} // namespace Runic2D
