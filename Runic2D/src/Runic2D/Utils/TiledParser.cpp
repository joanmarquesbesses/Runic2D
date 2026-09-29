#include "R2Dpch.h"
#include "TiledParser.h"

#include "Runic2D/Assets/AssetRegistry.h"

#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace Runic2D {

	bool TiledParser::LoadMap(const std::string& filepath, Ref<Scene> scene, TiledMapAsset* outComponent)
	{
		R2D_CORE_INFO("Loading Tiled map: {0}", filepath);
		std::ifstream stream(filepath);
		if (!stream.is_open()) {
			R2D_CORE_ERROR("TiledParser: Failed to open file {0}", filepath);
			return false;
		}
		nlohmann::json data;
		try { stream >> data; }
		catch (nlohmann::json::parse_error& e) {
			R2D_CORE_ERROR("TiledParser: JSON Parse Error: {0}", e.what());
			return false;
		}
		outComponent->MapWidth = data["width"];
		outComponent->MapHeight = data["height"];
		std::filesystem::path mapPath(filepath);
		std::vector<std::pair<int, std::filesystem::path>> tilesetsInfo;
		if (data.contains("tilesets"))
		{
			for (const auto& ts : data["tilesets"])
			{
				int firstgid = ts["firstgid"];
				std::filesystem::path tsjPath = (mapPath.parent_path() / (std::string)ts["source"]).lexically_normal();
				tilesetsInfo.push_back({ firstgid, tsjPath });
				std::ifstream tsjStream(tsjPath);
				if (tsjStream.is_open())
				{
					nlohmann::json tsjData;
					try {
						tsjStream >> tsjData;
						if (tsjData.contains("image"))
						{
							std::filesystem::path imagePath = (tsjPath.parent_path() / (std::string)tsjData["image"]).lexically_normal();
							UUID textureUUID = AssetRegistry::GetUUID(imagePath);
							if (textureUUID == 0) {
								textureUUID = UUID();
								AssetRegistry::RegisterAsset(textureUUID, imagePath);
							}
							outComponent->TilesetTextureHandle = textureUUID;
							outComponent->TilesetColumns = tsjData["columns"];
							outComponent->TileWidth = tsjData["tilewidth"];
							outComponent->TileHeight = tsjData["tileheight"];
							R2D_CORE_INFO("Textura del mapa registrada amb UUID {0}: {1}", (uint64_t)textureUUID, imagePath.string());
						}
					}
					catch (...) {}
				}
			}
		}
		if (data.contains("layers"))
		{
			for (const auto& layer : data["layers"])
			{
				std::string layerType = layer["type"];
				if (layerType == "tilelayer")
				{
					TileLayer newLayer;
					newLayer.Name = layer["name"];
					newLayer.Data = layer["data"].get<std::vector<int>>();
					outComponent->TileLayers.push_back(newLayer);
					R2D_CORE_INFO("Capa afegida: {0} ({1} rajoles)", newLayer.Name, newLayer.Data.size());
				}
				else if (layerType == "objectgroup")
				{
					std::string layerName = layer["name"];
					// --- NOVA PART: CAPA DE COLLIDERS ---
					if (layerName == "Colliders")
					{
						for (const auto& obj : layer["objects"])
						{
							TiledColliderData col;
							col.X = obj["x"];
							col.Y = obj["y"];
							col.Width = obj.contains("width") ? (float)obj["width"] : 32.0f;
							col.Height = obj.contains("height") ? (float)obj["height"] : 32.0f;
							outComponent->Colliders.push_back(col);
						}
						R2D_CORE_INFO("Capa afegida: {0} ({1} colliders)", layerName, outComponent->Colliders.size());
					}
					// --- PART ANTIGA: LA RESTA D'OBJECTES/PREFABS ---
					else
					{
						for (const auto& obj : layer["objects"])
						{
							std::string objClass = "";
							if (obj.contains("gid"))
							{
								int gid = obj["gid"];
								std::filesystem::path tsjPath;
								int firstGidOfTsj = 1;
								for (auto it = tilesetsInfo.rbegin(); it != tilesetsInfo.rend(); ++it) {
									if (gid >= it->first) {
										firstGidOfTsj = it->first;
										tsjPath = it->second;
										break;
									}
								}
								if (!tsjPath.empty())
								{
									std::ifstream tsjStream(tsjPath);
									if (tsjStream.is_open())
									{
										nlohmann::json tsjData;
										tsjStream >> tsjData;
										int localId = gid - firstGidOfTsj;
										if (tsjData.contains("tiles"))
										{
											for (const auto& tile : tsjData["tiles"])
											{
												if (tile["id"] == localId)
												{
													if (tile.contains("class") && tile["class"].is_string()) objClass = tile["class"];
													else if (tile.contains("type") && tile["type"].is_string()) objClass = tile["type"];
													break;
												}
											}
										}
									}
								}
							}
							else
							{
								if (obj.contains("class") && obj["class"].is_string()) objClass = obj["class"];
								else if (obj.contains("type") && obj["type"].is_string()) objClass = obj["type"];
							}
							float x = obj["x"];
							float y = obj["y"];
							R2D_CORE_INFO("Objecte detectat -> Classe: '{0}', Pos: X:{1} Y:{2}", objClass, x, y);
						}
					}
				}
			}
		}
		return true;
	}

}