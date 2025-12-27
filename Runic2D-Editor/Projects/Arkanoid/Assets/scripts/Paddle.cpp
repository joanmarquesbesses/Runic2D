#include "Paddle.h"

using namespace Runic2D;

void Paddle::OnCreate()
{
	auto texture = ResourceManager::Get<Texture2D>(Project::GetAssetFileSystemPath("textures/blocksTileset.png"));
	GetComponent<SpriteRendererComponent>().Texture = texture;
	GetComponent<SpriteRendererComponent>().SubTexture = SubTexture2D::CreateFromPixelCoords(
		texture,
		32.0f, 64.0f,
		48.0f, 14.0f
	);
}

void Paddle::OnDestroy()
{
}

void Paddle::OnUpdate(Timestep ts)
{
	auto& rb2d = GetComponent<Rigidbody2DComponent>();
	b2BodyId bodyId = (b2BodyId)rb2d.RuntimeBody;

	float move = 0.0f;
	if (Input::IsKeyPressed(KeyCode::A))
		move = -1.0f;
	else if (Input::IsKeyPressed(KeyCode::D))
		move = 1.0f;

	b2Vec2 velocity = { move * m_Speed, 0.0f };
	b2Body_SetLinearVelocity(bodyId, velocity);
}

void Paddle::OnCollision(Entity other)
{

}