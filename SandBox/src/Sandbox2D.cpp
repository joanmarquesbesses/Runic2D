#include "Sandbox2D.h"

#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Sandbox2D::Sandbox2D() : Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true) {

}

void Sandbox2D::OnAttach()
{
	R2D_PROFILE_FUNCTION();

	m_Texture = Runic2D::Texture2D::Create("assets/textures/Check.png");
}

void Sandbox2D::OnDetach()
{
	R2D_PROFILE_FUNCTION();

}

void Sandbox2D::OnUpdate(Runic2D::Timestep ts)
{
	R2D_PROFILE_FUNCTION();

	m_CameraController.OnUpdate(ts);

	//render
	{
		R2D_PROFILE_SCOPE("Renderer Prep")
		Runic2D::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Runic2D::RenderCommand::Clear();
	}

	{
		R2D_PROFILE_SCOPE("Renderer Draw")
		Runic2D::Renderer2D::BeginScene(m_CameraController.GetCamera());

		Runic2D::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, m_Texture);
		Runic2D::Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, m_SquareColor);
		Runic2D::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 1.0f }, m_SquareColor);

		Runic2D::Renderer2D::EndScene();
	}
}

void Sandbox2D::OnImGuiRender()
{
	R2D_PROFILE_FUNCTION();

	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));

	ImGui::End();
}

void Sandbox2D::OnEvent(Runic2D::Event& e)
{
	m_CameraController.OnEvent(e);
}
