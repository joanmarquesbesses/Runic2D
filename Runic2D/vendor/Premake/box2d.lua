project "Box2D"
    location "%{wks.location}/Runic2D/vendor/Box2D"
    kind "SharedLib"
    language "C" 
    cdialect "C17"  
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

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

    defines
	{
		"B2_USE_DEFAULT_ALLOCATOR",
        "_CRT_SECURE_NO_WARNINGS",
        "box2d_EXPORTS"
	}

    filter "system:windows"
        systemversion "latest"

        postbuildcommands {
            ('if not exist "..\\..\\..\\bin\\' .. outputdir .. '\\SandBox" mkdir "..\\..\\..\\bin\\' .. outputdir .. '\\SandBox"'),
            ('copy /Y "..\\..\\..\\bin\\' .. outputdir .. '\\Box2D\\Box2D.dll" "..\\..\\..\\bin\\' .. outputdir .. '\\SandBox\\"'),
            ('if not exist "..\\..\\..\\bin\\' .. outputdir .. '\\Runic2D-Editor" mkdir "..\\..\\..\\bin\\' .. outputdir .. '\\Runic2D-Editor"'),
            ('copy /Y "..\\..\\..\\bin\\' .. outputdir .. '\\Box2D\\Box2D.dll" "..\\..\\..\\bin\\' .. outputdir .. '\\Runic2D-Editor\\"'),
            ('if not exist "..\\..\\..\\bin\\' .. outputdir .. '\\' .. activeGame .. '" mkdir "..\\..\\..\\bin\\' .. outputdir .. '\\' .. activeGame .. '"'),
            ('copy /Y "..\\..\\..\\bin\\' .. outputdir .. '\\Box2D\\Box2D.dll" "..\\..\\..\\bin\\' .. outputdir .. '\\' .. activeGame .. '\\"')
        }

    filter "configurations:Debug"
        runtime "Debug" 
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"
        
    filter "configurations:Dist"
        runtime "Release"
        optimize "on"