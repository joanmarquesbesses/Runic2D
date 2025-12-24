#include "R2Dpch.h"
#include "ScriptEngine.h"

namespace Runic2D {
    ScriptEngine::ScriptBinderFn ScriptEngine::s_Binder = nullptr;
    ScriptEngine::ScriptNamesFn ScriptEngine::s_NamesGetter = nullptr;
}