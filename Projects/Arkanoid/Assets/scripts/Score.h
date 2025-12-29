#pragma once

#include "Runic2D.h"

class Score : public Runic2D::ScriptableEntity
{
public:
	virtual void OnCreate() override;
	virtual void OnDestroy() override;
	virtual void OnUpdate(Runic2D::Timestep ts) override;

	void AddScore(int amount);

private:
	int m_Score = 0;
};