project "ImGui"
	location "%{wks.location}/Runic2D/vendor/imgui"
	kind "StaticLib"
	language "C++"
	staticruntime "off"

	defines { "IMGUI_API=__declspec(dllexport)", "IMGUI_IMPL_OPENGL_LOADER_GLAD" }

	targetdir ("%{prj.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{prj.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.location}/imconfig.h",
		"%{prj.location}/imgui.h",
		"%{prj.location}/imgui.cpp",
		"%{prj.location}/imgui_demo.cpp",
		"%{prj.location}/imgui_draw.cpp",
		"%{prj.location}/imgui_internal.h",
		"%{prj.location}/imgui_tables.cpp",
		"%{prj.location}/imgui_widgets.cpp",
		"%{prj.location}/imstb_rectpack.h",
		"%{prj.location}/imstb_textedit.h",
		"%{prj.location}/imstb_truetype.h",
		"%{prj.location}/../ImGuizmo/ImGuizmo.h",
		"%{prj.location}/../ImGuizmo/ImGuizmo.cpp",
		"%{prj.location}/backends/imgui_impl_glfw.h",
        "%{prj.location}/backends/imgui_impl_glfw.cpp",
        "%{prj.location}/backends/imgui_impl_opengl3.h",
        "%{prj.location}/backends/imgui_impl_opengl3.cpp"
	}

	includedirs
    {
        "%{prj.location}",             
        "%{prj.location}/../ImGuizmo",
		"%{wks.location}/Runic2D/vendor/GLFW/include",
		"%{wks.location}/Runic2D/vendor/Glad/include"
    }

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"