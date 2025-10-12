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
	Runic2D::OrthographicCameraController m_CameraController;

	//temp
	Runic2D::Ref<Runic2D::VertexArray> m_SquareVA;
	Runic2D::Ref<Runic2D::Shader> m_FlatColorShader;

	Runic2D::Ref<Runic2D::Texture2D> m_Texture;

	glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };

};

