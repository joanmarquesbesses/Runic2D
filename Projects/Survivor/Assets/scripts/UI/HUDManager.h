// Projects/Survivor/Assets/scripts/UI/HUDManager.h
#pragma once
#include "Runic2D.h"
#include "Core/GameComponents.h"
#include <sstream>
#include <iomanip>

using namespace Runic2D;

namespace Survivor {

    class HUDManager : public ScriptableEntity {
    public:
        void OnCreate() override {
            auto& window = Application::Get().GetWindow();
            float aspect = (float)window.GetWidth() / (float)window.GetHeight();

            // --- Càmera de UI ---
            m_UICamera = GetScene()->CreateEntity("UICamera");
            m_UICamera.AddComponent<UIComponent>();
            auto& cc = m_UICamera.AddComponent<CameraComponent>();
            cc.Camera.SetOrthographic(m_OrthoSize, -1.0f, 1.0f);
            cc.Primary = false;

            float halfH = m_OrthoSize * 0.5f;
            float halfW = halfH * aspect;

            // --- Timer (centre superior) ---
            m_TimerText = CreateUIText("HUD_Timer", { 0.0f, halfH - 0.6f, 0.0f }, "0:00");

            // --- Health (esquerra inferior) ---
            m_HealthText = CreateUIText("HUD_Health",
                { -halfW + 0.5f, -halfH + 0.3f, 0.0f }, "HP: 100");
            m_HealthText.GetComponent<TextComponent>().Color = { 0.2f, 1.0f, 0.2f, 1.0f };

            // --- XP / Nivell (dreta inferior) ---
            m_XPText = CreateUIText("HUD_XP",
                { halfW - 1.5f, -halfH + 0.3f, 0.0f }, "Lv.1");
            m_XPText.GetComponent<TextComponent>().Color = { 0.4f, 0.8f, 1.0f, 1.0f };

            // --- FPS (cantonada superior dreta, debug) ---
            m_FPSText = CreateUIText("HUD_FPS",
                { -halfW, halfH - 0.4f, 0.0f }, "FPS: 0");
            m_FPSText.GetComponent<TextComponent>().Color = { 0.5f, 0.5f, 0.5f, 1.0f };
            GetComponent<TransformComponent>().SetScale({ 0.6f, 0.6f, 1.0f });
        }

        void OnDestroy() override {
            // Les entitats de UI es destruiran quan l'escena pari
            // No cal fer res manualment — l'escena gestiona el cicle de vida
        }

        void OnUpdate(Timestep ts) override {
            UpdateTimer();
            UpdateHealth();
            UpdateXP();
            UpdateFPS();
        }

    private:
        // --- Helpers de creació ---
        Entity CreateUIText(const std::string& name, const glm::vec3& pos,
            const std::string& initial)
        {
            Entity e = GetScene()->CreateEntity(name);
            e.AddComponent<UIComponent>();

            auto& txt = e.AddComponent<TextComponent>();
            txt.TextString = initial;
			float width = txt.FontAsset->GetStringWidth(initial, txt.Kerning);

			glm::vec3 offset = { pos.x - width/2.0f, pos.y, pos.z };
            e.GetComponent<TransformComponent>().SetTranslation(offset);

            return e;
        }

        // --- Actualitzacions ---
        void UpdateTimer() {
            if (!m_TimerText) return;

            Entity statsEntity = GetScene()->GetEntityWithComponent<GameStatsComponent>();
            if (!statsEntity) return;

            float time = statsEntity.GetComponent<GameStatsComponent>().TimeAlive;

            int minutes = (int)(time / 60.0f);
            int seconds = (int)(time) % 60;

			if (seconds == m_LastSeconds) return;

            std::stringstream ss;
            ss << minutes << ":" << std::setfill('0') << std::setw(2) << seconds;

            auto& txt = m_TimerText.GetComponent<TextComponent>();
            txt.TextString = ss.str();

            float width = txt.FontAsset->GetStringWidth(txt.TextString, txt.Kerning);

            glm::vec3 offset = { -width / 2.0f, 0.0f, 0.0f };
            m_TimerText.GetComponent<TransformComponent>().SetTranslation(offset);
        }

        void UpdateHealth() {
            if (!m_HealthText) return;

            Entity playerEntity = GetScene()->GetEntityWithComponent<PlayerStatsComponent>();
            if (!playerEntity) return;

            auto& stats = playerEntity.GetComponent<PlayerStatsComponent>();
            int hp = (int)stats.Health;
            int maxHp = (int)stats.MaxHealth;

            m_HealthText.GetComponent<TextComponent>().TextString =
                "HP: " + std::to_string(hp) + "/" + std::to_string(maxHp);

            // Color verd -> groc -> vermell segons HP
            float ratio = stats.Health / stats.MaxHealth;
            m_HealthText.GetComponent<TextComponent>().Color = {
                1.0f - ratio, ratio, 0.0f, 1.0f
            };
        }

        void UpdateXP() {
            if (!m_XPText) return;

            Entity statsEntity = GetScene()->GetEntityWithComponent<GameStatsComponent>();
            if (!statsEntity) return;

            auto& stats = statsEntity.GetComponent<GameStatsComponent>();
            m_XPText.GetComponent<TextComponent>().TextString =
                "Lv." + std::to_string(stats.CurrentLevel) +
                "  XP: " + std::to_string((int)stats.CurrentXP) +
                "/" + std::to_string((int)stats.MaxXP);
        }

        void UpdateFPS() {
            if (!m_FPSText) return;
            m_FPSText.GetComponent<TextComponent>().TextString =
                "FPS: " + std::to_string((int)Application::Get().GetAverageFPS());
        }

    private:
        float m_OrthoSize = 10.0f;
		int m_LastSeconds = -1;

        Entity m_UICamera;
        Entity m_TimerText;
        Entity m_HealthText;
        Entity m_XPText;
        Entity m_FPSText;
    };
}