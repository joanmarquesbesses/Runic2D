workspace "Runic2D"
	architecture "x64"
	startproject "SandBox"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Runic2D"
	location "Runic2D"
	kind "SharedLib"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include"
	}

	filter "system:windows"
		cppdialect "C++20"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"RUNIC2D_PLATFORM_WINDOWS",
			"RUNIC2D_BUILD_DLL"
		}

		postbuildcommands {
			("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/SandBox/\"")
		}

	filter "configurations:Debug"
		defines "RUNIC2D_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "RUNIC2D_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "RUNIC2D_DIST"
		optimize "On"

	filter { "system:windows", "configurations:Debug" }
		buildoptions "/utf-8"


project "SandBox"
	location "SandBox"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Runic2D/vendor/spdlog/include",
		"Runic2D/src"
	}

	links
	{
		"Runic2D"
	}

	filter "system:windows"
		cppdialect "C++20"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"RUNIC2D_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "RUNIC2D_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "RUNIC2D_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "RUNIC2D_DIST"
		optimize "On"

	filter { "system:windows", "configurations:Debug" }
		buildoptions "/utf-8"