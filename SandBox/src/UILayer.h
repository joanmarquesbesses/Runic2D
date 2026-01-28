#pragma once

#include <Runic2D.h>

#include "../../Projects/Survivor/Assets/scripts/GameContext.h"

class UILayer : public Runic2D::Layer
{
public:
	UILayer(Runic2D::Ref<GameContext> context);
	virtual ~UILayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void OnUpdate(Runic2D::Timestep ts) override;
	virtual void OnImGuiRender() override;
	virtual void OnEvent(Runic2D::Event& e) override;
private:
	bool OnKeyPressed(Runic2D::KeyPressedEvent& e);
	bool OnWindowResize(Runic2D::WindowResizeEvent& e);
	void ShowFPSCounter();
private:
	Runic2D::Ref<GameContext> m_Context;
	Runic2D::Ref<Runic2D::Scene> m_ActiveScene;
	Runic2D::Entity m_TextFPS;
	Runic2D::Entity m_TimerText;
	bool showFPS = false;

};

