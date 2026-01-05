#pragma once

#include <Runic2D.h>

class Sandbox2D : public Runic2D::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void OnUpdate(Runic2D::Timestep ts) override;
	virtual void OnImGuiRender() override;
	virtual void OnEvent(Runic2D::Event& e) override;
private:
	bool OnWindowResize(Runic2D::WindowResizeEvent& e);
private:
	Runic2D::Ref<Runic2D::Scene> m_ActiveScene;
};

