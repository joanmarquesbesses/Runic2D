#include "Score.h"

using namespace Runic2D;

void Score::OnCreate()
{
    auto& textComp = GetComponent<TextComponent>();
    textComp.TextString = "SCORE: 0";
}

void Score::OnDestroy()
{
}

void Score::OnUpdate(Timestep ts)
{

}

void Score::AddScore(int amount)
{
    m_Score += amount;

    if (HasComponent<TextComponent>())
    {
        GetComponent<TextComponent>().TextString = "SCORE: " + std::to_string(m_Score);
    }
}