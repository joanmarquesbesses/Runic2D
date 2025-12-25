#pragma once
#include "Runic2D.h"

class Ball : public Runic2D::ScriptableEntity
{
public:
	virtual void OnCreate() override;
	virtual void OnUpdate(Runic2D::Timestep ts) override;
	virtual void OnCollision(Runic2D::Entity other) override;

private:
	void Restart();

private:
	Runic2D::Rigidbody2DComponent* m_Rb2d = nullptr;
	float m_Speed = 7.5f;
};