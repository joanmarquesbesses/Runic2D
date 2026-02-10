#include "Player.h"

#include <Box2D/box2d.h>

#include "GameComponents.h"
#include "EntityFactory.h"
#include "OrbitalManager.h"

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

    if (HasComponent<BoxCollider2DComponent>())
    {
        auto& bc = GetComponent<BoxCollider2DComponent>();
        bc.Offset = { -0.150f, -0.150f };
        bc.CategoryBits = PhysicsLayers::Player;
		bc.MaskBits = PhysicsLayers::Default | PhysicsLayers::Enemy | PhysicsLayers::Projectile;
	}

    if (HasComponent<CircleCollider2DComponent>())
    {
        auto& cc = GetComponent<CircleCollider2DComponent>();
        cc.Offset = { -0.200f, -0.1f };
		cc.CategoryBits = PhysicsLayers::Player;
		cc.MaskBits = PhysicsLayers::Item;
        cc.IsSensor = true;
        cc.EnableSensorEvents = true;
		cc.EnableContactEvents = false;
        cc.Radius = cc.Radius * 2;
    }

	GetEntity().AddComponent<PlayerStatsComponent>().SyncToContext();

    if (!HasComponent<PlayerUpgradesComponent>()) {
        GetEntity().AddComponent<PlayerUpgradesComponent>();
    }

    m_OrbitalManager = GetScene()->CreateEntity("OrbitalManager");
    m_OrbitalManager.AddComponent<NativeScriptComponent>().Bind<OrbitalManager>();
}

void Player::OnDestroy()
{
    if (m_OrbitalManager) {
        GetScene()->SubmitForDestruction(m_OrbitalManager);
    }
}

void Player::OnUpdate(Timestep ts)
{
	HandleMovement(ts);
	HandleAnimation();
}

void Player::HandleMovement(Timestep ts)
{
    m_Rb = &GetComponent<Rigidbody2DComponent>();
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
	m_Transform = &GetComponent<TransformComponent>();
    if (m_Transform && CanChangeDirection())
    {
		auto& bc = GetComponent<BoxCollider2DComponent>();
		auto& cc = GetComponent<CircleCollider2DComponent>();

        bool changed = false;

        if (Input::IsKeyPressed(KeyCode::A))
        {
            if (m_Transform->Scale.x > 0)
            {
                m_Transform->Scale.x = -fabs(m_Transform->Scale.x);
                bc.Offset.x = fabs(bc.Offset.x);
                cc.Offset.x = fabs(cc.Offset.x);
                changed = true;
            }
        }
        else if (Input::IsKeyPressed(KeyCode::D))
        {
            if (m_Transform->Scale.x < 0)
            {
                m_Transform->Scale.x = fabs(m_Transform->Scale.x);
                bc.Offset.x = -fabs(bc.Offset.x);
                cc.Offset.x = -fabs(cc.Offset.x);
                changed = true;
            }
        }

        if (changed)
        {
            GetScene()->UpdateEntityColliders(GetEntity());
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

				Shoot(mousePos, spawnPos, direction);

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
	m_Anim = &GetComponent<AnimationComponent>();
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

	auto& bc = GetComponent<BoxCollider2DComponent>();
	auto& cc = GetComponent<CircleCollider2DComponent>();

	bool changed = false;

    if (mousePos.x < playerPos.x)
    {
        if (m_Transform->Scale.x > 0)
        {
            m_Transform->Scale.x = -fabs(m_Transform->Scale.x);
            bc.Offset.x = fabs(bc.Offset.x);
            cc.Offset.x = fabs(cc.Offset.x);
            changed = true;
        }
    }
    else
    {
        if (m_Transform->Scale.x < 0)
        {
            m_Transform->Scale.x = fabs(m_Transform->Scale.x);
            bc.Offset.x = -fabs(bc.Offset.x);
            cc.Offset.x = -fabs(cc.Offset.x);
            changed = true;
        }
    }

    if (changed)
    {
        GetScene()->UpdateEntityColliders(GetEntity());
    }
}

void Player::Shoot(glm::vec2 mousePos, glm::vec2 spawnPos, glm::vec2 direction)
{
    int multiShotLevel = 0;
    int piercingLevel = 0;

    if (HasComponent<PlayerUpgradesComponent>()) {
        auto& upgrades = GetComponent<PlayerUpgradesComponent>();
        multiShotLevel = upgrades.GetLevel(UpgradeType::MultiShot);
        piercingLevel = upgrades.GetLevel(UpgradeType::Piercing);
    }

    int projectileCount = 1 + (multiShotLevel * 2); 

    glm::vec2 perpDirection = { -direction.y, direction.x };
    float spacing = 0.6f; 

    for (int i = 0; i < projectileCount; i++)
    {
        float offsetIndex = i - (projectileCount / 2);

        glm::vec2 finalSpawnPos = spawnPos + (perpDirection * (offsetIndex * spacing));

        Entity bullet = EntityFactory::CreatePlayerProjectile(finalSpawnPos, direction);

        if (bullet.HasComponent<ProjectileComponent>()) {
            auto& pc = bullet.GetComponent<ProjectileComponent>();
            pc.Penetration = piercingLevel; 
        }
    }

    // So de dispar?
    // Audio::Play("Shoot");
}

void Player::ApplyUpgradeEffect(UpgradeType type, int newLevel) {

    if (type == UpgradeType::Orbitals) {
        if (m_OrbitalManager.HasComponent<NativeScriptComponent>()) {
            auto& nsc = m_OrbitalManager.GetComponent<NativeScriptComponent>();
            OrbitalManager* script = (OrbitalManager*)nsc.Instance;

            if (script) {
                script->SetLevel(newLevel);
            }
        }
    }

    // Aquí pots posar els altres efectes (MultiShot, Speed, etc.)

    // (Opcional) Aquí podries fer spawn de partícules al voltant del player
    // o reproduir un so de "Power Up".
}