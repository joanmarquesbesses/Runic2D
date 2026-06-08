#pragma once

#include "Systems/UpgradeDatabase.h"

namespace Survivor {

	class Player : public Runic2D::ScriptableEntity
	{
	public:
		virtual void OnCreate() override;
		virtual void OnDestroy() override;
		virtual void OnUpdate(Runic2D::Timestep ts) override;
		virtual void OnFixedUpdate(Runic2D::Timestep ts) override;
		virtual void OnCollision(Runic2D::Entity other) override;

		Runic2D::Entity GetOrbitalManager() const { return m_OrbitalManager; }
		void ApplyUpgradeEffect(UpgradeType type, int newLevel);

	private:
		void HandleMovement(Runic2D::Timestep ts);
		void HandleAnimation();
		void PlayAnimation(const std::string& name);
		bool IsMoving();
		bool CanChangeDirection() const;
		void TryAttack();
		void Shoot(glm::vec2 mousePos, glm::vec2 spawnPos, glm::vec2 direction);

	private:
		float m_MoveSpeed = 5.0f;
		glm::vec2 m_InputVelocity = { 0.0f, 0.0f };

		enum class State
		{
			Idle,
			Run,
			Attack,
			Hit,
			Death
		};

		State m_State = State::Idle;

		bool m_HasFired = false;

		Runic2D::Entity m_OrbitalManager;

		PhysicsSystem* m_PhysicsSystem = nullptr;
	};

}