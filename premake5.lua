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
IncludeDir["glm"] = "Runic2D/vendor/glm"
IncludeDir["stb_image"] = "Runic2D/vendor/stb_image"
IncludeDir["entt"] = "Runic2D/vendor/entt/include"
-- Include the vendor libraries

group "Dependencies"
	include "Runic2D/vendor/Premake/glfw.lua"
	include "Runic2D/vendor/Premake/Glad.lua"
	include "Runic2D/vendor/Premake/imgui.lua"
group ""

project "Runic2D"
	location "Runic2D"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "R2Dpch.h"
	pchsource "Runic2D/src/R2Dpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/stb_image/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.entt}"
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"RUNIC2D_PLATFORM_WINDOWS",
			"RUNIC2D_BUILD_DLL",
			"GLFW_INCLUDE_NONE" -- Prevent GLFW from including OpenGL headers
		}

	filter "configurations:Debug"
		defines "RUNIC2D_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "RUNIC2D_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "RUNIC2D_DIST"
		runtime "Release"
		optimize "on"

	filter { "system:windows", "configurations:Debug" }
		buildoptions "/utf-8"

	filter { "system:windows", "configurations:Release" }
		buildoptions "/utf-8"


project "SandBox"
	location "SandBox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

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
		"Runic2D/src",
		"Runic2D/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}"
	}

	links
	{
		"Runic2D"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"RUNIC2D_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "RUNIC2D_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "RUNIC2D_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "RUNIC2D_DIST"
		runtime "Release"
		optimize "on"

	filter { "system:windows", "configurations:Debug" }
		buildoptions "/utf-8"

	filter { "system:windows", "configurations:Release" }
		buildoptions "/utf-8"


project "Runic2D-Editor"
	location "Runic2D-Editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

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
		"Runic2D/src",
		"Runic2D/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}"
	}

	links
	{
		"Runic2D"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"RUNIC2D_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "RUNIC2D_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "RUNIC2D_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "RUNIC2D_DIST"
		runtime "Release"
		optimize "on"

	filter { "system:windows", "configurations:Debug" }
		buildoptions "/utf-8"

	filter { "system:windows", "configurations:Release" }
		buildoptions "/utf-8"