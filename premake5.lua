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

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "Runic2D/vendor/GLFW/include"
IncludeDir["Glad"] = "Runic2D/vendor/Glad/include"
IncludeDir["ImGui"] = "Runic2D/vendor/imgui"

include "Runic2D/vendor/Premake/glfw.lua"
include "Runic2D/vendor/Premake/Glad.lua"
include "Runic2D/vendor/Premake/imgui.lua"

project "Runic2D"
	location "Runic2D"
	kind "SharedLib"
	language "C++"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "R2Dpch.h"
	pchsource "Runic2D/src/R2Dpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}"
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib"
	}

	filter "system:windows"
		cppdialect "C++20"
		systemversion "latest"

		defines
		{
			"RUNIC2D_PLATFORM_WINDOWS",
			"RUNIC2D_BUILD_DLL",
			"GLFW_INCLUDE_NONE" -- Prevent GLFW from including OpenGL headers
		}

		postbuildcommands {
			("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/SandBox/\"")
		}

	filter "configurations:Debug"
		defines "RUNIC2D_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "RUNIC2D_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "RUNIC2D_DIST"
		runtime "Release"
		optimize "On"

	filter { "system:windows", "configurations:Debug" }
		buildoptions "/utf-8"

	filter { "system:windows", "configurations:Release" }
		buildoptions "/utf-8"


project "SandBox"
	location "SandBox"
	kind "ConsoleApp"
	language "C++"
	staticruntime "off"

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
		systemversion "latest"

		defines
		{
			"RUNIC2D_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "RUNIC2D_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "RUNIC2D_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "RUNIC2D_DIST"
		runtime "Release"
		optimize "On"

	filter { "system:windows", "configurations:Debug" }
		buildoptions "/utf-8"

	filter { "system:windows", "configurations:Release" }
		buildoptions "/utf-8"