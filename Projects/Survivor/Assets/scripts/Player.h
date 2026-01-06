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
	float m_Speed = 20.0f;
	Runic2D::Rigidbody2DComponent* m_Rb2d = nullptr;
};