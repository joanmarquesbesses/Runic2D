#include "ScriptRegistry.h"
#include "Runic2D/Scripting/ScriptEngine.h" 
#include "Runic2D/Core/Core.h"

extern "C" {
    __declspec(dllexport) void InitRuntimeModule(GameContext* sharedContext) {
        // Configurem el Singleton de la DLL amb el punter de l'Editor
        GameContext::Set(sharedContext);

        Runic2D::ScriptEngine::SetScriptBinder(ScriptRegistry::BindScript);
        Runic2D::ScriptEngine::SetScriptNamesGetter(ScriptRegistry::GetScriptNames);

        R2D_CORE_INFO("Survivor DLL: Context shared and scripts registered!");
    }
}