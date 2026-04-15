#include "R2Dpch.h"
#include "CameraController.h"

void CameraController::OnUpdate(Timestep ts)
{
    if (!m_Player)
    {
        m_Player = GetScene()->FindEntityByName("Player");
        if (!m_Player) return;
    }

    auto& playerTrans = m_Player.GetComponent<TransformComponent>().Translation;
    auto& cameraTransComp = GetComponent<TransformComponent>();

    float t = m_SmoothSpeed * ts;

    cameraTransComp.Translation.x += (playerTrans.x - cameraTransComp.Translation.x) * t;
    cameraTransComp.Translation.y += (playerTrans.y - cameraTransComp.Translation.y) * t;
	cameraTransComp.IsDirty = true;
}