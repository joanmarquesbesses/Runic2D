#pragma once
#include "Runic2D/Scene/Entity.h"
#include <functional>
#include <string>
#include <vector>

namespace Runic2D {

	class ScriptEngine
	{
	public:
		using ScriptBinderFn = std::function<void(std::string, Entity)>;

		using ScriptNamesFn = std::function<std::vector<std::string>()>;

		static void SetScriptBinder(ScriptBinderFn binder) { s_Binder = binder; }

		static void SetScriptNamesGetter(ScriptNamesFn getter) { s_NamesGetter = getter; }

		static void BindScript(std::string name, Entity entity)
		{
			if (s_Binder) s_Binder(name, entity);
		}

		static std::vector<std::string> GetAvailableScripts()
		{
			if (s_NamesGetter) return s_NamesGetter();
			return {};
		}

	private:
		static ScriptBinderFn s_Binder;
		static ScriptNamesFn s_NamesGetter;
	};
}