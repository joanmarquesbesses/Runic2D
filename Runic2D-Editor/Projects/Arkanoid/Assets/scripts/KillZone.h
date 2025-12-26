#pragma once
#include "Runic2D.h"

class KillZone : public Runic2D::ScriptableEntity
{
public:
	virtual void OnCreate() override {}
	virtual void OnDestroy() override {}
	virtual void OnUpdate(Runic2D::Timestep ts) override {}
	virtual void OnCollision(Runic2D::Entity other) override;
};