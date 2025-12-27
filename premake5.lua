workspace "Runic2D"
	architecture "x64"
	startproject "Runic2D-Editor"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	flags
	{
		"MultiProcessorCompile"
	}

	local activeGame = "Arkanoid" 
    -- local activeGame = "Mario"
    
    local gameSourcePath = "Projects/" .. activeGame .. "/Assets/scripts"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/Runic2D/vendor/GLFW/include"
IncludeDir["Glad"] = "%{wks.location}/Runic2D/vendor/Glad/include"
IncludeDir["ImGui"] = "%{wks.location}/Runic2D/vendor/imgui"
IncludeDir["glm"] = "%{wks.location}/Runic2D/vendor/glm"
IncludeDir["stb_image"] = "%{wks.location}/Runic2D/vendor/stb_image"
IncludeDir["entt"] = "%{wks.location}/Runic2D/vendor/entt/include"
IncludeDir["yaml_cpp"] = "%{wks.location}/Runic2D/vendor/yaml-cpp/include"
IncludeDir["ImGuizmo"] = "%{wks.location}/Runic2D/vendor/ImGuizmo"
IncludeDir["Box2D"] = "%{wks.location}/Runic2D/vendor/Box2D/include"
-- Include the vendor libraries

group "Dependencies"
	include "Runic2D/vendor/Premake/glfw.lua"
	include "Runic2D/vendor/Premake/Glad.lua"
	include "Runic2D/vendor/Premake/imgui.lua"
	include "Runic2D/vendor/Premake/yaml-cpp.lua"
	include "Runic2D/vendor/Premake/box2d.lua"
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
		"_CRT_SECURE_NO_WARNINGS",
		"YAML_CPP_STATIC_DEFINE"
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
		"%{IncludeDir.entt}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.Box2D}"
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"yaml-cpp",
		"opengl32.lib",
		"Box2D"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"R2D_BUILD_DLL",
			"GLFW_INCLUDE_NONE" -- Prevent GLFW from including OpenGL headers
		}

	filter "configurations:Debug"
		defines "R2D_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "R2D_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "R2D_DIST"
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
		"%{IncludeDir.entt}",
		"%{IncludeDir.Box2D}"
	}

	links
	{
		"Runic2D"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "R2D_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "R2D_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "R2D_DIST"
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
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/" .. gameSourcePath .. "/**.h",
		"%{prj.name}/" .. gameSourcePath .. "/**.cpp"
	}

	includedirs
	{
		"Runic2D/vendor/spdlog/include",
		"Runic2D/src",
		"Runic2D/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.Box2D}",
		"%{prj.name}/" .. gameSourcePath
	}

	links
	{
		"Runic2D"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "R2D_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "R2D_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "R2D_DIST"
		runtime "Release"
		optimize "on"

	filter { "system:windows", "configurations:Debug" }
		buildoptions "/utf-8"

	filter { "system:windows", "configurations:Release" }
		buildoptions "/utf-8"