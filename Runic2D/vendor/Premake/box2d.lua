project "Box2D"
    location "%{wks.location}/Runic2D/vendor/Box2D"
    kind "StaticLib"
    language "C" 
    cdialect "C17"  
    staticruntime "on"

	targetdir ("%{prj.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{prj.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.location}/src/**.c",  
        "%{prj.location}/src/**.h",
        "%{prj.location}/include/**.h"
    }

    includedirs
    {
        "%{prj.location}/include",
        "%{prj.location}/src"
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