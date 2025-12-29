project "msdf-atlas-gen"
	location "%{wks.location}/Runic2D/vendor/msdf-atlas-gen"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
    {
        "%{wks.location}/Runic2D/vendor/msdf-atlas-gen/msdf-atlas-gen/**.h",
		"%{wks.location}/Runic2D/vendor/msdf-atlas-gen/msdf-atlas-gen/**.cpp",
		"%{wks.location}/Runic2D/vendor/msdf-atlas-gen/msdfgen/**.h",
		"%{wks.location}/Runic2D/vendor/msdf-atlas-gen/msdfgen/**.cpp",

        "%{wks.location}/Runic2D/vendor/freetype/src/base/ftdebug.c", 
        
		"%{wks.location}/Runic2D/vendor/freetype/src/autofit/autofit.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/base/ftbase.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/base/ftbbox.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/base/ftbdf.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/base/ftbitmap.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/base/ftcid.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/base/ftfstype.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/base/ftgasp.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/base/ftglyph.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/base/ftgxval.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/base/ftinit.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/base/ftmm.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/base/ftotval.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/base/ftpatent.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/base/ftpfr.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/base/ftstroke.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/base/ftsynth.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/base/ftsystem.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/base/fttype1.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/base/ftwinfnt.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/bdf/bdf.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/cache/ftcache.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/cff/cff.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/cid/type1cid.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/gzip/ftgzip.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/lzw/ftlzw.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/pcf/pcf.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/pfr/pfr.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/psaux/psaux.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/pshinter/pshinter.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/psnames/psnames.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/raster/raster.c",
        
        "%{wks.location}/Runic2D/vendor/freetype/src/sdf/sdf.c",
        "%{wks.location}/Runic2D/vendor/freetype/src/svg/svg.c",

		"%{wks.location}/Runic2D/vendor/freetype/src/sfnt/sfnt.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/smooth/smooth.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/truetype/truetype.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/type1/type1.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/type42/type42.c",
		"%{wks.location}/Runic2D/vendor/freetype/src/winfonts/winfnt.c"
    }

    includedirs
    {
        "%{wks.location}/Runic2D/vendor/msdf-atlas-gen/msdf-atlas-gen",
        "%{wks.location}/Runic2D/vendor/msdf-atlas-gen/msdfgen",
        "%{wks.location}/Runic2D/vendor/msdf-atlas-gen/artery-font-format",
        
        "%{wks.location}/Runic2D/vendor/freetype/include"
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