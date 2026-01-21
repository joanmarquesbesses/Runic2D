#include "Player.h"

#include <Box2D/box2d.h>

#include "EntityFactory.h"

using namespace Runic2D;

void Player::OnCreate()
{
	if (HasComponent<Rigidbody2DComponent>())
		m_Rb = &GetComponent<Rigidbody2DComponent>();

    if (HasComponent<AnimationComponent>()) {
        m_Anim = &GetComponent<AnimationComponent>();
        m_Anim->CurrentStateName = "";
    }

	if (HasComponent<TransformComponent>())
		m_Transform = &GetComponent<TransformComponent>();
}

void Player::OnDestroy()
{
}

void Player::OnUpdate(Timestep ts)
{
	HandleMovement(ts);
	HandleAnimation();
}

void Player::HandleMovement(Timestep ts)
{
	if (!m_Rb) return;

    if (m_State == State::Attack || m_State == State::Death)
    {
        b2BodyId bodyId = m_Rb->RuntimeBody;
        b2Body_SetLinearVelocity(bodyId, { 0.0f, 0.0f });
        return;
    }

	glm::vec2 velocity = { 0.0f, 0.0f };

	if (Input::IsKeyPressed(KeyCode::W)) velocity.y += 1.0f;
	if (Input::IsKeyPressed(KeyCode::S)) velocity.y -= 1.0f;
	if (Input::IsKeyPressed(KeyCode::A)) velocity.x -= 1.0f;
	if (Input::IsKeyPressed(KeyCode::D)) velocity.x += 1.0f;

	if (glm::length(velocity) > 0.0f)
		velocity = glm::normalize(velocity) * m_MoveSpeed;

	b2BodyId bodyId = m_Rb->RuntimeBody;
	b2Vec2 newVel = { velocity.x, velocity.y };
	b2Body_SetLinearVelocity(bodyId, newVel);
}

void Player::HandleAnimation()
{
    if (m_Transform && CanChangeDirection())
    {
        if (Input::IsKeyPressed(KeyCode::A))
        {
            if (m_Transform->Scale.x > 0) m_Transform->Scale.x = -fabs(m_Transform->Scale.x);
        }
        else if (Input::IsKeyPressed(KeyCode::D))
        {
            if (m_Transform->Scale.x < 0) m_Transform->Scale.x = fabs(m_Transform->Scale.x);
        }
    }

    switch (m_State)
    {
        case State::Idle:
        {
            PlayAnimation("Idle");

            if (Input::IsMouseButtonPressed(MouseButton::Left))
            {
                TryAttack();
            }
            else if (Input::IsKeyPressed(KeyCode::T)) {
                m_State = State::Death;
                PlayAnimation("Death");
            }
            else if (IsMoving())
            {
                m_State = State::Run;
				PlayAnimation("Run");
            }
            break;
        }
        case State::Run:
        {
            PlayAnimation("Run");

            if (Input::IsMouseButtonPressed(MouseButton::Left))
            {
                TryAttack();
            }
            else if (!IsMoving())
            {
                m_State = State::Idle;
				PlayAnimation("Idle");
            }
            break;
        }
        case State::Attack:
        {
            glm::vec2 mousePos = Runic2D::Utils::SceneUtils::GetMouseWorldPosition(GetScene());
            glm::vec2 playerPos = { m_Transform->Translation.x, m_Transform->Translation.y };

            if (mousePos.x < playerPos.x) {
                if (m_Transform->Scale.x > 0) m_Transform->Scale.x = -fabs(m_Transform->Scale.x);
            }
            else {
                if (m_Transform->Scale.x < 0) m_Transform->Scale.x = fabs(m_Transform->Scale.x);
            }

            if (m_Anim->CurrentFrameIndex >= 5 && !m_HasFired)
            {
                float facingDirection = (m_Transform->Scale.x > 0.0f) ? 1.0f : -1.0f;
                float handOffsetX = 1.3f;
                float handOffsetY = 0.5f;

                glm::vec2 spawnPos = playerPos;
                spawnPos.x += handOffsetX * facingDirection;
                spawnPos.y += handOffsetY;

                glm::vec2 direction = mousePos - spawnPos;
                if (glm::length(direction) > 0.0f)
                    direction = glm::normalize(direction);
                else
                    direction = { facingDirection, 0.0f };

                EntityFactory::CreatePlayerProjectile(spawnPos, direction);
                m_HasFired = true;
            }

            if (m_Anim->IsFinished())
            {
                if (IsMoving()) {
                    m_State = State::Run;
                    PlayAnimation("Run");
                }
                else {
                    m_State = State::Idle;
                    PlayAnimation("Idle");
                }
            }
            break;
        }
        case State::Death:
        {
            if (!m_Anim->IsFinished())
            {
                // Encara s'està morint... esperem.
            }
            else
            {
                if (IsMoving())
                {
                    m_State = State::Run;
                    PlayAnimation("Run");
                }
            }
            break;
        }
    }
}

void Player::OnCollision(Entity other)
{

}

void Player::PlayAnimation(const std::string& name)
{
    if (m_Anim->CurrentStateName == name) return;

    auto it = m_Anim->Animations.find(name);
    if (it != m_Anim->Animations.end())
    {
        m_Anim->CurrentAnimation = it->second;
        m_Anim->CurrentStateName = name;

        m_Anim->CurrentFrameIndex = 0;
        m_Anim->TimeAccumulator = 0.0f;

        m_Anim->Playing = true;

        for (auto& profile : m_Anim->Profiles)
        {
            if (profile.Name == name)
            {
                m_Anim->Loop = profile.Loop;
                break;
            }
        }

        if (m_Rb) 
        {
            auto& src = GetComponent<SpriteRendererComponent>();
            src.SubTexture = m_Anim->CurrentAnimation->GetFrame(0);
        }
    }
}

bool Player::IsMoving() const
{
    if (!m_Anim || !m_Rb) return false;

    b2BodyId bodyId = (b2BodyId)m_Rb->RuntimeBody;
    const b2Vec2& vel = b2Body_GetLinearVelocity(bodyId);
    glm::vec2 velocity = { vel.x, vel.y };
    float speed = glm::length(velocity);

    bool hasInput = Input::IsKeyPressed(KeyCode::W) || Input::IsKeyPressed(KeyCode::S) ||
		Input::IsKeyPressed(KeyCode::A) || Input::IsKeyPressed(KeyCode::D);

    return speed > 0.1f || hasInput;
}

bool Player::CanChangeDirection() const
{
    if (m_State == State::Attack || m_State == State::Death)
		return false;
    else
		return true;
}

void Player::TryAttack()
{
    m_State = State::Attack;
    PlayAnimation("Attack");
    m_HasFired = false;

    glm::vec2 mousePos = Runic2D::Utils::SceneUtils::GetMouseWorldPosition(GetScene());
    glm::vec2 playerPos = { m_Transform->Translation.x, m_Transform->Translation.y };

    if (mousePos.x < playerPos.x)
    {
        if (m_Transform->Scale.x > 0) m_Transform->Scale.x = -fabs(m_Transform->Scale.x);
    }
    else
    {
        if (m_Transform->Scale.x < 0) m_Transform->Scale.x = fabs(m_Transform->Scale.x);
    }
}
