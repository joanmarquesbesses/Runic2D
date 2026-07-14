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

	activeGame = "Survivor" 
    -- local activeGame = "Arkanoid"
    
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
IncludeDir["msdf_atlas_gen"] = "%{wks.location}/Runic2D/vendor/msdf-atlas-gen/msdf-atlas-gen"
IncludeDir["msdfgen"] = "%{wks.location}/Runic2D/vendor/msdf-atlas-gen/msdfgen"
IncludeDir["miniaudio"] = "%{wks.location}/Runic2D/vendor/miniaudio"
IncludeDir["lz4"] = "%{wks.location}/Runic2D/vendor/lz4"
-- Include the vendor libraries

group "Dependencies"
	include "Runic2D/vendor/Premake/glfw.lua"
	include "Runic2D/vendor/Premake/Glad.lua"
	include "Runic2D/vendor/Premake/imgui.lua"
	include "Runic2D/vendor/Premake/yaml-cpp.lua"
	include "Runic2D/vendor/Premake/box2d.lua"
	include "Runic2D/vendor/Premake/msdf-atlas-gen.lua"
group ""

project "Runic2D"
	location "Runic2D"
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

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
		"%{prj.name}/vendor/glm/glm/**.inl",
		"%{prj.name}/vendor/lz4/**.h",
		"%{prj.name}/vendor/lz4/**.c"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"YAML_CPP_STATIC_DEFINE",
		"MSDFGEN_PUBLIC=",
		"B2_USE_DEFAULT_ALLOCATOR",
		"BOX2D_DLL"
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
		"%{IncludeDir.Box2D}",
		"%{IncludeDir.msdf_atlas_gen}",
		"%{IncludeDir.msdfgen}",
		"%{wks.location}/Runic2D/vendor/msdf-atlas-gen",
		"%{IncludeDir.miniaudio}",
		"%{IncludeDir.lz4}"
	}

	links
	{
		"Box2D",
		"GLFW",
		"Glad",
		"ImGui",
		"yaml-cpp",
		"opengl32.lib",
		"msdf-atlas-gen"
	}

	libdirs { 
        "bin/" .. outputdir .. "/Box2D" 
    }

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"R2D_BUILD_DLL",
			"GLFW_INCLUDE_NONE" -- Prevent GLFW from including OpenGL headers
		}
		links {
			"opengl32.lib"      
		}

	postbuildcommands
    {
        ("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Runic2D-Editor"),
        ("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/SandBox")
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

	filter { "system:windows", "configurations:Dist" }
        buildoptions "/utf-8"

	filter "files:**.c"
		flags { "NoPCH" }
	filter {}


project "SandBox"
	location "SandBox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	debugdir "%{wks.location}"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	defines
	{
		"MSDFGEN_PUBLIC="
	}

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
		"%{IncludeDir.Box2D}",
		"%{IncludeDir.msdf_atlas_gen}",
		"Runic2D/vendor/msdf-atlas-gen/msdfgen"
	}

	links
	{
		"Runic2D"
	}

	filter "system:windows"
		systemversion "latest"

		links {
            "opengl32.lib" 
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
		kind "WindowedApp"
		entrypoint "mainCRTStartup"
		runtime "Release"
		optimize "on"

	filter { "system:windows", "configurations:Debug" }
		buildoptions "/utf-8"

	filter { "system:windows", "configurations:Release" }
		buildoptions "/utf-8"

	filter { "configurations:Dist", "system:windows" }
		buildoptions "/utf-8"
        entrypoint "mainCRTStartup"


project "Runic2D-Editor"
	location "Runic2D-Editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	debugdir "%{wks.location}"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	defines
	{
		"MSDFGEN_PUBLIC=" 
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
		"%{IncludeDir.msdf_atlas_gen}",
		"%{IncludeDir.msdfgen}"
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
		kind "WindowedApp"           
		entrypoint "mainCRTStartup"
		runtime "Release"
		optimize "on"

	filter { "system:windows", "configurations:Debug" }
		buildoptions "/utf-8"

	filter { "system:windows", "configurations:Release" }
		buildoptions "/utf-8"

	filter { "configurations:Dist", "system:windows" }
		buildoptions "/utf-8"
        entrypoint "mainCRTStartup"


project (activeGame)
    location ("Projects/" .. activeGame)
    kind "SharedLib" 
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "Projects/" .. activeGame .. "/Assets/scripts/**.h",
        "Projects/" .. activeGame .. "/Assets/scripts/**.cpp"
    }

	pchheader (activeGame .. "PCH.h")
	pchsource ("Projects/" .. activeGame .. "/Assets/scripts/" .. activeGame .. "PCH.cpp")

	defines
    {
        "MSDFGEN_PUBLIC=",
        "YAML_CPP_STATIC_DEFINE",
		"B2_USE_DEFAULT_ALLOCATOR",
		"BOX2D_DLL"
    }

    includedirs {
        "Runic2D/src",
        "Runic2D/vendor",
		"Runic2D/vendor/spdlog/include",
        "%{IncludeDir.glm}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.Box2D}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.msdf_atlas_gen}",
		"%{IncludeDir.msdfgen}",
        "%{wks.location}/Runic2D/vendor/msdf-atlas-gen/msdfgen/include",
        "%{wks.location}/Runic2D/vendor/msdf-atlas-gen/msdfgen/core",
        "Projects/" .. activeGame .. "/Assets/scripts"
    }

    links { "Runic2D", "Box2D", "yaml-cpp" }

    filter "system:windows"
        systemversion "latest"
		buildoptions "/utf-8"

	filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
        defines { "R2D_DEBUG", "B2_USE_DEFAULT_ALLOCATOR" }

    filter "configurations:Release"
        runtime "Release"
        optimize "on"
        defines { "R2D_RELEASE", "B2_USE_DEFAULT_ALLOCATOR" }

    filter "configurations:Dist"
        runtime "Release"
        optimize "on"
        defines { "R2D_DIST", "B2_USE_DEFAULT_ALLOCATOR" }

	postbuildcommands
    {
        ("{COPY} ../../bin/" .. outputdir .. "/Runic2D/Runic2D.dll %{cfg.buildtarget.directory}")
    }