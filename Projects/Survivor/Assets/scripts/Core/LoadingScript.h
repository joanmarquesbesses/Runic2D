#pragma once

#include "Runic2D/Scene/ScriptableEntity.h"
#include "Runic2D/Scene/SceneManager.h"
#include "Runic2D/Scene/Components/RenderComponents.h"

using namespace Runic2D;

namespace Survivor
{
    class LoadingScript : public ScriptableEntity {
    public:
		Entity Text;
        void OnCreate() override {
			Text = GetScene()->GetEntityByUUID((UUID)17664064999875066267);
        }

        virtual void OnUpdate(Timestep ts) override {
			Text.GetComponent<TextComponent>().SetText("Loading... " + std::to_string((int)(SceneManager::GetLoadingProgress() * 100.0f)) + "%");
        }

        void OnDestroy() override {

        }

    private:

    };

}