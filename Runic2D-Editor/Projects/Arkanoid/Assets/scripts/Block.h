#pragma once
#include "Runic2D.h"

class Block : public Runic2D::ScriptableEntity
{
public:
	virtual void OnCreate() override;
	virtual void OnDestroy() override;
	virtual void OnUpdate(Runic2D::Timestep ts) override;
	virtual void OnCollision(Runic2D::Entity other) override;

private:
	bool m_PendingDestroy = false;
};