#include "SurvivorPCH.h"
#include <R2Dpch.h>
#include "CameraController.h"

void CameraController::OnUpdate(Timestep ts)
{
    if (!m_Player)
    {
        m_Player = GetScene()->FindEntityByName("Player");
        if (!m_Player) return;
    }

    auto playerTrans = m_Player.GetComponent<TransformComponent>().GetTranslation();
    auto& cameraTransComp = GetComponent<TransformComponent>();

    float t = m_SmoothSpeed * ts;

    cameraTransComp.SetTranslation({ cameraTransComp.GetTranslation().x + (playerTrans.x - cameraTransComp.GetTranslation().x) * t,
                                     cameraTransComp.GetTranslation().y + (playerTrans.y - cameraTransComp.GetTranslation().y) * t,
                                     cameraTransComp.GetTranslation().z });
}
