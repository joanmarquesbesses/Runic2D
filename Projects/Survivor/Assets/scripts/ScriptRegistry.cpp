#include "ScriptRegistry.h"
#include "Runic2D/Scripting/ScriptEngine.h" 
#include "Runic2D/Core/Core.h"

extern "C" {
    __declspec(dllexport) void InitRuntimeModule() {
        UpgradeDatabase::Init();

        ScriptEngine::SetScriptBinder(ScriptRegistry::BindScript);
        ScriptEngine::SetScriptNamesGetter(ScriptRegistry::GetScriptNames);

        R2D_CORE_INFO("Survivor DLL: Inicialitzada i dades carregades.");
    }
}

extern "C" {
    __declspec(dllexport) void ShutdownRuntimeModule() {
        UpgradeDatabase::Shutdown();
        R2D_CORE_INFO("Survivor DLL: Shutdown correcte!");
    }
}