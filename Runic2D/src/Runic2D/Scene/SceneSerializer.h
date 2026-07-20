#pragma once

#include "Scene.h"

namespace Runic2D
{
	class RUNIC_API SceneSerializer
	{
	public:
		SceneSerializer(const Ref<Scene>& scene);

		void Serialize(const std::string& filepath);
		void SerializeBinary(const std::string& filepath);
		void SerializeRuntime(const std::string& filepath);

		bool Deserialize(const std::string& filepath);
		bool DeserializeBinary(const std::string& filepath);
		bool DeserializeRuntime(const std::string& filepath);

		static void SerializeEntityToPrefab(Entity entity, const std::filesystem::path& filepath);
		static Entity DeserializePrefabToScene(const std::filesystem::path& filepath, Scene* scene);
	private:
		Ref<Scene> m_Scene;
	};
} // namespace Runic2D



