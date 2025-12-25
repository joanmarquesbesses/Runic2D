#include "Block.h"

using namespace Runic2D;

void Block::OnCreate()
{
}

void Block::OnDestroy()
{
}

void Block::OnUpdate(Timestep ts)
{
	if (m_PendingDestroy)
	{
		if (m_PendingDestroy)
		{
			Destroy(); 
		}
	}
}

void Block::OnCollision(Entity other)
{
	if (other.GetComponent<NativeScriptComponent>().ClassName == "Ball") {
		m_PendingDestroy = true;
	}
	m_PendingDestroy = true;
}