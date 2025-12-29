#pragma once
#include "Runic2D.h"

enum class BlockType
{
	Bonce = 0,   
	Plat = 1, 
	Gold = 2,   
	Platino = 3,
	Esmeralda = 4,
	Diamond = 5,
	Count
};

class Block : public Runic2D::ScriptableEntity
{
public:
	virtual void OnCreate() override;
	virtual void OnDestroy() override;
	virtual void OnUpdate(Runic2D::Timestep ts) override;
	virtual void OnCollision(Runic2D::Entity other) override;

	void Initialize(BlockType type);

private:
	void UpdateVisuals(); 
	void Die();
private:
	bool m_PendingDestroy = false;

	BlockType m_Type = BlockType::Bonce;

	int m_Lives = 1;
	int m_MaxLives = 1;

	int tileWidth = 32;
	int tileHeight = 16;
	int heightOffset = 176;

	bool m_IsDying = false;

	float m_AnimationTime = 0.0f;
	float m_FrameDuration = 0.1f;
	int m_CurrentFrame = 0;
	int m_TotalAnimFrames = 5;

	float m_FallSpeed = 5.0f; 
};