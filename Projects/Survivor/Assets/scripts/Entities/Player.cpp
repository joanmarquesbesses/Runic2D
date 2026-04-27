#include "Player.h"

#include <Box2D/box2d.h>

#include "Core/GameComponents.h"
#include "EntityFactory.h"
#include "Systems/OrbitalManager.h"

using namespace Runic2D;

namespace Survivor {

    void Player::OnCreate()
    {
        if (HasComponent<AnimationComponent>()) {
            GetEntity().GetComponent<AnimationComponent>().CurrentStateName = "";
        }

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
        if (!HasComponent<Rigidbody2DComponent>()) return;
        auto& rb = GetComponent<Rigidbody2DComponent>();
        b2BodyId bodyId = rb.RuntimeBody;

        if (!b2Body_IsValid(bodyId)) return;

        if (m_State == State::Attack || m_State == State::Death)
        {
            b2Body_SetLinearVelocity(rb.RuntimeBody, { 0.0f, 0.0f });
            return;
        }

        glm::vec2 velocity = { 0.0f, 0.0f };
        if (Input::IsKeyPressed(KeyCode::W)) velocity.y += 1.0f;
        if (Input::IsKeyPressed(KeyCode::S)) velocity.y -= 1.0f;
        if (Input::IsKeyPressed(KeyCode::A)) velocity.x -= 1.0f;
        if (Input::IsKeyPressed(KeyCode::D)) velocity.x += 1.0f;

        if(HasComponent<PlayerStatsComponent>())
        {
			m_MoveSpeed = GetComponent<PlayerStatsComponent>().Speed;
		}

        if (glm::length(velocity) > 0.0f)
            velocity = glm::normalize(velocity) * m_MoveSpeed;

        // 2. Aquí ja és segur cridar-ho
        b2Vec2 newVel = { velocity.x, velocity.y };
        b2Body_SetLinearVelocity(rb.RuntimeBody, newVel);
        b2Body_SetAwake(bodyId, true);
    }

    void Player::HandleAnimation()
    {
        auto tranform = &GetComponent<TransformComponent>();
        if (tranform && CanChangeDirection())
        {
            auto& bc = GetComponent<BoxCollider2DComponent>();
            auto& cc = GetComponent<CircleCollider2DComponent>();

            bool changed = false;

            if (Input::IsKeyPressed(KeyCode::A))
            {
                if (tranform->Scale.x > 0)
                {
                    tranform->Scale.x = -fabs(tranform->Scale.x);
                    bc.Offset.x = fabs(bc.Offset.x);
                    cc.Offset.x = fabs(cc.Offset.x);
                    changed = true;
                }
            }
            else if (Input::IsKeyPressed(KeyCode::D))
            {
                if (tranform->Scale.x < 0)
                {
                    tranform->Scale.x = fabs(tranform->Scale.x);
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

        auto anim = &GetComponent<AnimationComponent>();

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
            glm::vec2 playerPos = { tranform->Translation.x, tranform->Translation.y };

            if (mousePos.x < playerPos.x) {
                if (tranform->Scale.x > 0) tranform->Scale.x = -fabs(tranform->Scale.x);
            }
            else {
                if (tranform->Scale.x < 0) tranform->Scale.x = fabs(tranform->Scale.x);
            }

            if (anim->CurrentFrameIndex >= 5 && !m_HasFired)
            {
                float facingDirection = (tranform->Scale.x > 0.0f) ? 1.0f : -1.0f;
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

            if (anim->IsFinished())
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
            if (!anim->IsFinished())
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
        auto anim = &GetComponent<AnimationComponent>();
        if (anim->CurrentStateName == name) return;

        auto it = anim->Animations.find(name);
        if (it != anim->Animations.end())
        {
            anim->CurrentAnimation = it->second;
            anim->CurrentStateName = name;

            anim->CurrentFrameIndex = 0;
            anim->TimeAccumulator = 0.0f;

            anim->Playing = true;

            for (auto& profile : anim->Profiles)
            {
                if (profile.Name == name)
                {
                    anim->Loop = profile.Loop;
                    break;
                }
            }

            if (HasComponent<SpriteRendererComponent>())
            {
                auto& src = GetComponent<SpriteRendererComponent>();
                src.SubTexture = anim->CurrentAnimation->GetFrame(0);
            }
        }
    }

    bool Player::IsMoving()
    {
        auto anim = &GetComponent<AnimationComponent>();
        auto rb = &GetComponent<Rigidbody2DComponent>();

        if (!anim || !rb) return false;
        b2BodyId bodyId = (b2BodyId)rb->RuntimeBody;
        if (!b2Body_IsValid(bodyId)) return false;

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

        auto transform = &GetComponent<TransformComponent>();

        glm::vec2 mousePos = Runic2D::Utils::SceneUtils::GetMouseWorldPosition(GetScene());
        glm::vec2 playerPos = { transform->Translation.x, transform->Translation.y };

        auto& bc = GetComponent<BoxCollider2DComponent>();
        auto& cc = GetComponent<CircleCollider2DComponent>();

        bool changed = false;

        if (mousePos.x < playerPos.x)
        {
            if (transform->Scale.x > 0)
            {
                transform->Scale.x = -fabs(transform->Scale.x);
                bc.Offset.x = fabs(bc.Offset.x);
                cc.Offset.x = fabs(cc.Offset.x);
                changed = true;
            }
        }
        else
        {
            if (transform->Scale.x < 0)
            {
                transform->Scale.x = fabs(transform->Scale.x);
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

}