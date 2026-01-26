#include "GameManager.h"
#include "Runic2D/Utils/Random.h" 

void GameManager::OnUpdate(Timestep ts) {
    m_SpawnTimer -= ts;

    // Input per testejar el spawn
    if (Input::IsKeyPressed(KeyCode::B) && m_SpawnTimer <= 0.0f) {
        SpawnEnemy();
        m_SpawnTimer = m_SpawnCooldown;
    }
}

void GameManager::SpawnEnemy() {
    glm::vec2 spawnPos = GetRandomOffScreenPosition();

    EntityFactory::CreateBat(spawnPos);
}

glm::vec2 GameManager::GetRandomOffScreenPosition() {
    auto scene = GetScene();
    Entity cameraEntity = scene->GetPrimaryCameraEntity();

    if (!cameraEntity) return { 0.0f, 0.0f };

    auto& camComp = cameraEntity.GetComponent<CameraComponent>();
    auto& camTrans = cameraEntity.GetComponent<TransformComponent>();

    float orthoSize = camComp.Camera.GetOrthographicSize();
	float aspectRatio = camComp.Camera.GetAspectRatio();

    float camHeight = orthoSize * 2.0f;
    float camWidth = camHeight * aspectRatio;

    float topEdge = camTrans.Translation.y + (camHeight / 2.0f);
    float bottomEdge = camTrans.Translation.y - (camHeight / 2.0f);
    float rightEdge = camTrans.Translation.x + (camWidth / 2.0f);
    float leftEdge = camTrans.Translation.x - (camWidth / 2.0f);

    float buffer = 2.0f;

    int side = (int)Random::Range(0.0f, 4.0f);

    glm::vec2 pos = { 0.0f, 0.0f };

    switch (side) {
    case 0: // TOP
        pos.x = Random::Range(leftEdge, rightEdge);
        pos.y = topEdge + buffer;
        break;
    case 1: // RIGHT
        pos.x = rightEdge + buffer;
        pos.y = Random::Range(bottomEdge, topEdge);
        break;
    case 2: // BOTTOM
        pos.x = Random::Range(leftEdge, rightEdge);
        pos.y = bottomEdge - buffer;
        break;
    case 3: // LEFT
        pos.x = leftEdge - buffer;
        pos.y = Random::Range(bottomEdge, topEdge);
        break;
    }

    return pos;
}