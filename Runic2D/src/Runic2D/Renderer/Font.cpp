#include "R2Dpch.h"
#include "Font.h"

#include "FontGeometry.h"
#include "GlyphGeometry.h"

namespace Runic2D {

	template<typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
	static Ref<Texture2D> CreateAndCacheAtlas(const std::string& fontName, float fontSize, const std::vector<msdf_atlas::GlyphGeometry>& glyphs, const msdf_atlas::FontGeometry& fontGeometry, uint32_t width, uint32_t height)
	{
		msdf_atlas::GeneratorAttributes attributes;
		attributes.config.overlapSupport = true;
		attributes.scanlinePass = true;

		msdf_atlas::ImmediateAtlasGenerator<S, N, GenFunc, msdf_atlas::BitmapAtlasStorage<T, N>> generator(width, height);
		generator.setAttributes(attributes);
		generator.setThreadCount(8); 
		generator.generate(glyphs.data(), (int)glyphs.size());

		const auto& storage = generator.atlasStorage();

		msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>)storage;

		TextureSpecification spec;
		spec.Width = width;
		spec.Height = height;
		spec.Format = ImageFormat::RGBA8;
		spec.GenerateMips = false;

		Ref<Texture2D> texture = Texture2D::Create(spec);

		texture->SetData((void*)bitmap.pixels, width * height * N);

		return texture;
	}

	Font::Font(const std::filesystem::path& filepath)
		: m_Data(new MSDFData())
	{
		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		if (!ft)
		{
			R2D_CORE_ERROR("Could not initialize FreeType library.");
			return;
		}

		// 2. Carregar la font
		std::string fileString = filepath.string();
		msdfgen::FontHandle* font = msdfgen::loadFont(ft, fileString.c_str());
		if (!font)
		{
			R2D_CORE_ERROR("Could not load font: {0}", fileString);
			msdfgen::deinitializeFreetype(ft);
			return;
		}

		struct CharsetRange { uint32_t Begin, End; };
		static const CharsetRange charsetRanges[] = { { 32, 126 } };

		msdf_atlas::Charset charset;
		for (CharsetRange range : charsetRanges)
		{
			for (uint32_t c = range.Begin; c <= range.End; c++)
				charset.add(c);
		}

		double fontScale = 1.0;
		m_Data->FontGeometry = msdf_atlas::FontGeometry(&m_Data->Glyphs);

		int glyphsLoaded = m_Data->FontGeometry.loadCharset(font, fontScale, charset);
		R2D_CORE_INFO("Loaded {0} glyphs from font {1}", glyphsLoaded, fileString);

		double emSize = 40.0; 
		msdf_atlas::TightAtlasPacker packer;

		packer.setDimensions(512, 512);
		packer.setOuterPixelPadding(0);
		packer.setScale(emSize);
		packer.setPixelRange(2.0); 
		packer.setMiterLimit(1.0);

		int remaining = packer.pack(m_Data->Glyphs.data(), (int)m_Data->Glyphs.size());
		if (remaining > 0)
		{
			R2D_CORE_ERROR("Font Atlas is too small! Could not pack {0} glyphs.", remaining);
		}

		int width, height;
		packer.getDimensions(width, height);

		m_AtlasTexture = CreateAndCacheAtlas<uint8_t, float, 4, msdf_atlas::mtsdfGenerator>(
			"TestFont",
			(float)emSize,
			m_Data->Glyphs,
			m_Data->FontGeometry,
			width, height
		);

		// 6. Neteja
		msdfgen::destroyFont(font);
		msdfgen::deinitializeFreetype(ft);
	}

	Font::~Font()
	{
		delete m_Data;
	}

	Ref<Font> Font::GetDefault()
	{
		static Ref<Font> DefaultFont;
		if (!DefaultFont)
			DefaultFont = CreateRef<Font>("Resources/Fonts/Inter/Inter_18pt-Regular.ttf");

		return DefaultFont;
	}
}