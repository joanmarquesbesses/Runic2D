#pragma once

#include "Runic2D.h"

class Player : public Runic2D::ScriptableEntity
{
public:
	virtual void OnCreate() override;
	virtual void OnDestroy() override;
	virtual void OnUpdate(Runic2D::Timestep ts) override;
	virtual void OnCollision(Runic2D::Entity other) override;

private:
	void HandleMovement(Runic2D::Timestep ts);
	void HandleAnimation();
	void PlayAnimation(const std::string& name);
	bool IsMoving() const;
	bool CanChangeDirection() const;

private:
	Runic2D::Rigidbody2DComponent* m_Rb = nullptr;
	Runic2D::AnimationComponent* m_Anim = nullptr;
	Runic2D::TransformComponent* m_Transform = nullptr;

	float m_MoveSpeed = 5.0f; 

	enum class State
	{
		Idle,
		Run,
		Attack,
		Hit, 
		Dead 
	};

	State m_State = State::Idle; 
};