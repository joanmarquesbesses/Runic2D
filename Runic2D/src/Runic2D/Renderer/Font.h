#pragma once

#include "Runic2D/Renderer/Texture.h"
#include <filesystem>

#undef INFINITE
#include "msdf-atlas-gen.h"

namespace Runic2D {

	struct MSDFData
	{
		std::vector<msdf_atlas::GlyphGeometry> Glyphs;
		msdf_atlas::FontGeometry FontGeometry;
	};

	class Font
	{
	public:
		Font(const std::filesystem::path& filepath);
		~Font();

		Ref<Texture2D> GetAtlasTexture() const { return m_AtlasTexture; }

		const MSDFData* GetMSDFData() const { return m_Data; }

		static Ref<Font> GetDefault();

		float GetStringWidth(const std::string& string, float kerning = 0.0f) const;

	private:
		std::filesystem::path m_FilePath;
		Ref<Texture2D> m_AtlasTexture;
		MSDFData* m_Data = nullptr;
	};

}