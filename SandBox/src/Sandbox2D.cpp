#include "Sandbox2D.h"

#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>

template<typename Fn>
class Timer
{
public:
	Timer(const char* name, Fn&& func)
		: m_Name(name), m_Func(func), m_Stopped(false)
	{
		m_StartTimepoint = std::chrono::high_resolution_clock::now();
	}

	void Stop()
	{
		auto endTimepoint = std::chrono::high_resolution_clock::now();

		auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

		m_Stopped = true;

		float duration = (end - start) * 0.001f;

		m_Func({ m_Name, duration });
	}

	~Timer()
	{
		if (!m_Stopped)
			Stop();
	}
private:
	const char* m_Name;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
	bool m_Stopped;
	Fn m_Func;
};

#define PROFILE_SCOPE(name) Timer timer##__LINE__(name, [&](ProfileResult profileResult) { m_ProfileResults.push_back(profileResult); });

Sandbox2D::Sandbox2D() : Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true) {

}

void Sandbox2D::OnAttach()
{
	m_Texture = Runic2D::Texture2D::Create("assets/textures/Check.png");
}

void Sandbox2D::OnDetach()
{
}

void Sandbox2D::OnUpdate(Runic2D::Timestep ts)
{
	PROFILE_SCOPE("Sandbox2D::OnUpdate")

		//update
	{
		PROFILE_SCOPE("CameraController::OnUpdate")
		m_CameraController.OnUpdate(ts);
	}

	//render
	{
		PROFILE_SCOPE("Renderer Prep")
		Runic2D::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Runic2D::RenderCommand::Clear();
	}

	{
		PROFILE_SCOPE("Renderer Draw")
		Runic2D::Renderer2D::BeginScene(m_CameraController.GetCamera());

		Runic2D::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, m_Texture);
		Runic2D::Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, m_SquareColor);
		Runic2D::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 1.0f }, m_SquareColor);

		Runic2D::Renderer2D::EndScene();
	}
}

void Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));

	//Profile
	for (const auto& result : m_ProfileResults) {
		char label[50];
		strcpy(label, " %.3fms ");
		strcat(label, result.Name);
		ImGui::Text(label, result.Time);
	}
	m_ProfileResults.clear();

	ImGui::End();
}

void Sandbox2D::OnEvent(Runic2D::Event& e)
{
	m_CameraController.OnEvent(e);
}
