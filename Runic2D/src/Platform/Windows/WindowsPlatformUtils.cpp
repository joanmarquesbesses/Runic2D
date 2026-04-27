#include "R2Dpch.h"
#include "Runic2D/Utils/PlatformUtils.h"

#include <commdlg.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <filesystem>

#include "Runic2D/Core/Application.h"

namespace Runic2D {

    std::string FileDialogs::OpenFile(const char* filter, const char* initialDir)
    {
        OPENFILENAMEA ofn;
        char szFile[260] = { 0 };

        std::string fallbackPath = std::filesystem::current_path().string();

        ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
        ofn.lStructSize = sizeof(OPENFILENAMEA);
        ofn.hwndOwner = glfwGetWin32Window(
            (GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.lpstrInitialDir = (initialDir != nullptr) ? initialDir : fallbackPath.c_str();
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileNameA(&ofn) == TRUE)
            return std::string(ofn.lpstrFile);

        return std::string();
    }

    std::string FileDialogs::SaveFile(const char* filter, const char* initialDir)
    {
        OPENFILENAMEA ofn;
        char szFile[260] = { 0 };

        std::string fallbackPath = std::filesystem::current_path().string();

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = glfwGetWin32Window(
            (GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.lpstrInitialDir = (initialDir != nullptr) ? initialDir : fallbackPath.c_str();
        ofn.lpstrDefExt = strchr(filter, '\0') + 1;
        ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

        if (GetSaveFileNameA(&ofn) == TRUE)
            return std::string(ofn.lpstrFile);

        return std::string();
    }
} // namespace Runic2D