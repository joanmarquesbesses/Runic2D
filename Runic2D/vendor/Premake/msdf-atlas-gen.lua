project "msdf-atlas-gen"
	location (EngineRoot .. "/Runic2D/vendor/msdf-atlas-gen")
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ((EngineRoot .. "/bin/") .. outputdir .. "/%{prj.name}")
	objdir ((EngineRoot .. "/bin-int/") .. outputdir .. "/%{prj.name}")

	files
    {
        (EngineRoot .. "/Runic2D/vendor/msdf-atlas-gen/msdf-atlas-gen/**.h"),
		(EngineRoot .. "/Runic2D/vendor/msdf-atlas-gen/msdf-atlas-gen/**.cpp"),
		(EngineRoot .. "/Runic2D/vendor/msdf-atlas-gen/msdfgen/**.h"),
		(EngineRoot .. "/Runic2D/vendor/msdf-atlas-gen/msdfgen/**.cpp"),

        (EngineRoot .. "/Runic2D/vendor/freetype/src/base/ftdebug.c"), 
        
		(EngineRoot .. "/Runic2D/vendor/freetype/src/autofit/autofit.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/base/ftbase.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/base/ftbbox.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/base/ftbdf.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/base/ftbitmap.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/base/ftcid.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/base/ftfstype.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/base/ftgasp.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/base/ftglyph.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/base/ftgxval.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/base/ftinit.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/base/ftmm.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/base/ftotval.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/base/ftpatent.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/base/ftpfr.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/base/ftstroke.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/base/ftsynth.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/base/ftsystem.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/base/fttype1.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/base/ftwinfnt.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/bdf/bdf.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/cache/ftcache.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/cff/cff.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/cid/type1cid.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/gzip/ftgzip.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/lzw/ftlzw.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/pcf/pcf.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/pfr/pfr.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/psaux/psaux.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/pshinter/pshinter.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/psnames/psnames.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/raster/raster.c"),
        
        (EngineRoot .. "/Runic2D/vendor/freetype/src/sdf/sdf.c"),
        (EngineRoot .. "/Runic2D/vendor/freetype/src/svg/svg.c"),

		(EngineRoot .. "/Runic2D/vendor/freetype/src/sfnt/sfnt.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/smooth/smooth.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/truetype/truetype.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/type1/type1.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/type42/type42.c"),
		(EngineRoot .. "/Runic2D/vendor/freetype/src/winfonts/winfnt.c")
    }

    includedirs
    {
        (EngineRoot .. "/Runic2D/vendor/msdf-atlas-gen/msdf-atlas-gen"),
        (EngineRoot .. "/Runic2D/vendor/msdf-atlas-gen/msdfgen"),
        (EngineRoot .. "/Runic2D/vendor/msdf-atlas-gen/artery-font-format"),
        
        (EngineRoot .. "/Runic2D/vendor/freetype/include")
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "MSDFGEN_USE_CPP11",
        "MSDFGEN_PUBLIC=",  
        "FT2_BUILD_LIBRARY"
    }

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		runtime "Release"
		optimize "on"
