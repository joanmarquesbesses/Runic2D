#include "Ball.h"

using namespace Runic2D;

void Ball::Restart()
{
	auto& rb2d = GetComponent<Rigidbody2DComponent>();
	b2BodyId bodyId = (b2BodyId)rb2d.RuntimeBody;

	auto& transform = GetComponent<TransformComponent>();
	transform.SetTranslation({ 0.0f, 3.5f, 0.0f });

	float x = (float)(rand() % 100) / 100.0f * 2.0f - 1.0f;
	float y = 1.0f;

	b2Vec2 velocity = { x, y };
	b2Vec2 normalizedVel = b2Normalize(velocity);

	velocity = { normalizedVel.x * m_Speed, normalizedVel.y * m_Speed };
	b2Body_SetLinearVelocity(bodyId, velocity);
}

void Ball::OnCreate()
{
	Restart();
}

void Ball::OnUpdate(Timestep ts)
{
	auto& rb2d = GetComponent<Rigidbody2DComponent>();
	b2BodyId bodyId = (b2BodyId)rb2d.RuntimeBody;

	b2Vec2 velocity = b2Body_GetLinearVelocity(bodyId);
	float speed = b2Length(velocity);

	if (speed > 0.01f)
	{
		b2Vec2 normalized = b2Normalize(velocity);
		b2Vec2 newVel = { normalized.x * m_Speed, normalized.y * m_Speed };
		b2Body_SetLinearVelocity(bodyId, newVel);
	}
}

void Ball::OnCollision(Entity other)
{
	
}