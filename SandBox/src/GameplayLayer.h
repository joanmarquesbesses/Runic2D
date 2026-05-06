#pragma once

#include <Runic2D.h>

class GameplayLayer : public Runic2D::Layer
{
public:
	GameplayLayer();

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void OnUpdate(Runic2D::Timestep ts) override;
	virtual void OnFixedUpdate(Runic2D::Timestep ts) override;
	virtual void OnEvent(Runic2D::Event& e) override;
private:
	bool OnKeyPressed(Runic2D::KeyPressedEvent& e);
	bool OnWindowResize(Runic2D::WindowResizeEvent& e);
	void ShowColliderOverlay();
private:
	bool m_ShowPhysicsColliders = false;
};

