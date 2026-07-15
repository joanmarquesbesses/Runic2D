#include "R2Dpch.h"
#include "SceneManager.h"

#include "SceneSerializer.h"
#include "Runic2D/Project/Project.h"

#include "Runic2D/Core/Application.h"
#include "Runic2D/Core/JobSystem.h"
#include "Runic2D/Core/BackgroundTaskSystem.h"

#include "Runic2D/Scripting/ScriptEngine.h"
#include "Components/ScriptingComponents.h"

namespace Runic2D {

    Ref<Scene>                           SceneManager::s_ActiveScene = nullptr;
    SceneManager::SceneChangedCallbackFn SceneManager::s_OnSceneChanged = nullptr;
    SceneManager::SceneLoadData            SceneManager::s_PendingLoad;
    float                                  SceneManager::s_LoadingProgress = 0.0f;

    bool SceneManager::LoadStartScene()
    {
        R2D_CORE_ASSERT(Project::GetActive(), "No hi ha projecte actiu!");

        const std::string& startScene = Project::GetConfig().StartScene;
        if (startScene.empty())
        {
            R2D_CORE_WARN("SceneManager: StartScene no definida al projecte.");
            s_ActiveScene = CreateRef<Scene>();
            if (s_OnSceneChanged) s_OnSceneChanged(s_ActiveScene);
            return false;
        }

        return LoadScene(startScene);
    }

    bool SceneManager::LoadScene(const std::filesystem::path& relativePath)
    {
        auto absolutePath = Project::GetAssetFileSystemPath(relativePath);
        return LoadSceneInternal(absolutePath, true);
    }

    bool SceneManager::LoadSceneAbsolute(const std::filesystem::path& absolutePath)
    {
        return LoadSceneInternal(absolutePath, true);
    }

    bool SceneManager::TransitionTo(const std::filesystem::path& relativePath)
    {
        RequestLoadScene(relativePath);
        return true;
    }

    void SceneManager::StopActiveScene()
    {
        if (!s_ActiveScene) return;
        s_ActiveScene->OnRuntimeStop();
        s_ActiveScene = nullptr;
        R2D_CORE_INFO("SceneManager: Escena activa parada.");
    }

    bool SceneManager::LoadSceneInternal(const std::filesystem::path& absolutePath,
        bool startRuntime)
    {

        if (!std::filesystem::exists(absolutePath))
        {
            R2D_CORE_ERROR("SceneManager: Escena no trobada: '{0}'", absolutePath.string());
            return false;
        }

        StopActiveScene();

        auto newScene = CreateRef<Scene>();
        SceneSerializer serializer(newScene);

        if (!serializer.DeserializeBinary(absolutePath.string() + "_bin"))
        {
            R2D_CORE_ERROR("SceneManager: Error deserialitzant '{0}'", absolutePath.string());
            return false;
        }

        s_ActiveScene = newScene;

        auto view = s_ActiveScene->GetAllEntitiesWith<NativeScriptComponent>();
        for (auto e : view)
        {
            Entity entity = { e, s_ActiveScene.get() };
            auto& nsc = entity.GetComponent<NativeScriptComponent>();
            if (!nsc.ClassName.empty())
                ScriptEngine::BindScript(nsc.ClassName, entity);
        }

        if (startRuntime)
        {
            s_ActiveScene->OnRuntimeStart();
        }

        auto& window = Application::Get().GetWindow();
        s_ActiveScene->OnViewportResize(window.GetWidth(), window.GetHeight());

        R2D_CORE_INFO("SceneManager: Escena carregada: '{0}'", absolutePath.string());

        if (s_OnSceneChanged) s_OnSceneChanged(s_ActiveScene);

        return true;
    }

    void SceneManager::RequestLoadScene(const std::filesystem::path& relativePath)
    {
        s_PendingLoad.TargetPath = relativePath;
        s_PendingLoad.IsAsync = false;
        s_PendingLoad.Pending = true;
    }

    void SceneManager::ProcessDeferredLoad()
    {
        if (!s_PendingLoad.Pending) return;

        // Mark as processed immediately to avoid re-entry
        s_PendingLoad.Pending = false;

        if (s_PendingLoad.IsAsync)
        {
            // ASYNC FLOW
            // 1. Load the loading scene synchronously (it's end of frame, so it's safe)
            LoadScene(s_PendingLoad.LoadingPath);
            s_LoadingProgress = 0.0f;

            // 2. Offload the heavy work to a background thread
            auto targetPath = s_PendingLoad.TargetPath;
            auto absolutePath = Project::GetAssetFileSystemPath(targetPath);

            BackgroundTaskSystem::Execute([absolutePath, targetPath]() {
                Ref<Scene> newScene = CreateRef<Scene>();
                SceneSerializer serializer(newScene);

                if (serializer.Deserialize(absolutePath.string()))
                {
                    Application::Get().SubmitToMainThread([newScene, absolutePath]() {
                        StopActiveScene();
                        s_ActiveScene = newScene;

                        // Activate scripts
                        auto view = s_ActiveScene->GetAllEntitiesWith<NativeScriptComponent>();
                        for (auto e : view)
                        {
                            Entity entity = { e, s_ActiveScene.get() };
                            auto& nsc = entity.GetComponent<NativeScriptComponent>();
                            if (!nsc.ClassName.empty())
                                ScriptEngine::BindScript(nsc.ClassName, entity);
                        }

                        s_ActiveScene->OnRuntimeStart();
                        auto& window = Application::Get().GetWindow();
                        s_ActiveScene->OnViewportResize(window.GetWidth(), window.GetHeight());

                        if (s_OnSceneChanged) s_OnSceneChanged(s_ActiveScene);
                        R2D_CORE_INFO("SceneManager: Async load finished: '{0}'", absolutePath.string());
                    });
                }
            });
        }
        else
        {
            // SYNC FLOW
            LoadScene(s_PendingLoad.TargetPath);
        }
    }

    void SceneManager::TransitionToAsync(const std::filesystem::path& relativePath, const std::filesystem::path& loadingScenePath)
    {
        s_PendingLoad.TargetPath = relativePath;
        s_PendingLoad.LoadingPath = loadingScenePath;
        s_PendingLoad.IsAsync = true;
        s_PendingLoad.Pending = true;
        s_LoadingProgress = 0.0f;
    }
}
