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
	m_RunicTexture = Runic2D::Texture2D::Create("assets/textures/icon.png");

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
	Runic2D::Renderer2D::ResetStats();
	{
		R2D_PROFILE_SCOPE("Renderer Prep")
		Runic2D::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Runic2D::RenderCommand::Clear();
	}

	{
		R2D_PROFILE_SCOPE("Renderer Draw")

		static float rotation = 0.0f;
		rotation += ts * 50.0f;

		Runic2D::Renderer2D::BeginScene(m_CameraController.GetCamera());

		Runic2D::Renderer2D::DrawRotatedQuad({ 0.5f, -0.5f , -0.1}, { 0.5f, 1.0f }, rotation, m_SquareColor);
		Runic2D::Renderer2D::DrawRotatedQuad({ 1.0f, 1.0f }, { 0.8f, 0.8f }, -rotation, m_SquareColor);
		Runic2D::Renderer2D::DrawQuad({ -1.0f, 1.0f }, { 1.0f, 1.0f }, m_Texture);
		Runic2D::Renderer2D::DrawQuad({ 0.0f, 0.0f }, { 0.8f, 0.8f }, m_RunicTexture, 1.0f);
		Runic2D::Renderer2D::DrawQuad({ 0.0f, 1.0f }, { 0.8f, 0.8f }, m_RunicTexture, 10.0f);
		Runic2D::Renderer2D::DrawRotatedQuad({ -2.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, 45.0f, m_Texture, 10.0f, glm::vec4(1.0f, 0.7f, 0.7f, 1.0f));

		Runic2D::Renderer2D::EndScene();

		Runic2D::Renderer2D::BeginScene(m_CameraController.GetCamera());
		for (float y = -5.0f; y < 5.0f; y += 0.25f) {
			for (float x = -5.0f; x < 5.0f; x += 0.25f) {
				glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.5f };
				Runic2D::Renderer2D::DrawQuad({ x, y }, { 0.2f, 0.2f }, color);
			}
		}
		Runic2D::Renderer2D::EndScene();
	}
}

void Sandbox2D::OnImGuiRender()
{
	R2D_PROFILE_FUNCTION();
	int avaragefps = Runic2D::Application::Get().GetAverageFPS();
	auto stats = Runic2D::Renderer2D::GetStats();
	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
	ImGui::Text("Renderer2D Stats");
	ImGui::Text("Avarage FPS: %d", avaragefps);
	ImGui::Text("Draw Calls: %d", stats.DrawCalls);
	ImGui::Text("Quad Count: %d", stats.QuadCount);
	ImGui::Text("Vertex Count: %d", stats.GetTotalVertexCount());
	ImGui::Text("Index Count: %d", stats.GetTotalIndexCount());
	ImGui::End();
}

void Sandbox2D::OnEvent(Runic2D::Event& e)
{
	m_CameraController.OnEvent(e);
}
