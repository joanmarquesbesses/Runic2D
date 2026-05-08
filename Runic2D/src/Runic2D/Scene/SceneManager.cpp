#include "R2Dpch.h"
#include "SceneManager.h"
#include "SceneSerializer.h"
#include "Runic2D/Project/Project.h"
#include "Runic2D/Core/Application.h"
#include "Runic2D/Scripting/ScriptEngine.h"
#include "Component.h"

namespace Runic2D {

    Ref<Scene>                           SceneManager::s_ActiveScene = nullptr;
    SceneManager::SceneChangedCallbackFn SceneManager::s_OnSceneChanged = nullptr;
    std::filesystem::path                  SceneManager::s_QueuedScenePath = "";

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
        R2D_CORE_INFO("SceneManager: LoadSceneInternal called for '{0}' (startRuntime: {1})", absolutePath.string(), startRuntime);

        if (!std::filesystem::exists(absolutePath))
        {
            R2D_CORE_ERROR("SceneManager: Escena no trobada: '{0}'", absolutePath.string());
            return false;
        }

        StopActiveScene();

        auto newScene = CreateRef<Scene>();
        SceneSerializer serializer(newScene);

        if (!serializer.Deserialize(absolutePath.string()))
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
        s_QueuedScenePath = relativePath;
    }

    void SceneManager::ProcessDeferredLoad()
    {
        if (s_QueuedScenePath.empty()) return;

        std::filesystem::path path = s_QueuedScenePath;
        s_QueuedScenePath.clear(); // Clear before loading to avoid loops
        LoadScene(path);
    }
}
