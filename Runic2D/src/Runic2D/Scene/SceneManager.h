#pragma once

#include "Runic2D/Core/Core.h"

#include "Scene.h"
#include "Components/ComponentRegistry.h"

#include <filesystem>
#include <functional>

namespace Runic2D {

    class RUNIC_API SceneManager
    {
    public:
        using SceneChangedCallbackFn = std::function<void(Ref<Scene>)>;

        static bool LoadStartScene();

        static bool LoadScene(const std::filesystem::path& relativePath);

        static bool LoadSceneAbsolute(const std::filesystem::path& absolutePath);

        static bool TransitionTo(const std::filesystem::path& relativePath);
        static void TransitionToAsync(const std::filesystem::path& relativePath, const std::filesystem::path& loadingScenePath);

        static float GetLoadingProgress() { return s_LoadingProgress; }

		static void SetLoadingProgress(float progress) { s_LoadingProgress = progress; }

        static Ref<Scene> GetActiveScene() { return s_ActiveScene; }

        static void StopActiveScene();

        static void SetSceneChangedCallback(SceneChangedCallbackFn callback)
        {
            s_OnSceneChanged = callback;
        }

        static void Shutdown()
        {
            StopActiveScene();
            s_OnSceneChanged = nullptr;
            Runic2D::ComponentRegistry::Clear();
        }

        static void ProcessDeferredLoad();

        static void SetActiveScene(Ref<Scene> scene)
        {
            StopActiveScene();
            s_ActiveScene = scene;
            if (s_OnSceneChanged) s_OnSceneChanged(s_ActiveScene);
        }

    private:
        static bool LoadSceneInternal(const std::filesystem::path& absolutePath, bool startRuntime);
        static void RequestLoadScene(const std::filesystem::path& relativePath);

        struct SceneLoadData
        {
            std::filesystem::path TargetPath;
            std::filesystem::path LoadingPath;
            bool IsAsync = false;
            bool Pending = false;
        };

        static SceneLoadData            s_PendingLoad;
        static float                    s_LoadingProgress;
        static Ref<Scene>               s_ActiveScene;
        static SceneChangedCallbackFn   s_OnSceneChanged;
    };
}