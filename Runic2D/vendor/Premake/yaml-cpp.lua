project "yaml-cpp"
	location "%{wks.location}/Runic2D/vendor/yaml-cpp"
	kind "StaticLib"
	language "C++"
	staticruntime "on"

	targetdir ("%{prj.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{prj.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.location}/src/**.h",
		"%{prj.location}/src/**.cpp",
		"%{prj.location}/include/**.h"
	}

	includedirs
	{
		"%{prj.location}/include"
	}

	defines
	{
		"YAML_CPP_STATIC_DEFINE"
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