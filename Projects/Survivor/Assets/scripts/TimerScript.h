#include "Runic2D.h"

using namespace Runic2D;

class TimerScript : public ScriptableEntity {
public:
    void OnUpdate(Timestep ts) override {
        float time = GameContext::Get().TimeAlive;

        int minutes = (int)(time / 60.0f);
        int seconds = (int)(time) % 60;

        std::stringstream ss;
        ss << minutes << ":" << std::setfill('0') << std::setw(2) << seconds;

        GetComponent<TextComponent>().TextString = ss.str();
    }
};