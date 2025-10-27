#include "Sandbox2D.h"

#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static const char* s_MapTiles =
"################"
"#..............#"
"#..............#"
"#.....##.......#"
"#..............#"
"#..a...........#"
"#..............#"
"#..............#"
"#..............#"
"#..............#"
"#..............#"
"#..............#"
"################";

Sandbox2D::Sandbox2D() : Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true) {

}

void Sandbox2D::OnAttach()
{
	R2D_PROFILE_FUNCTION();

	m_Texture = Runic2D::Texture2D::Create("assets/textures/Check.png");
	m_RunicTexture = Runic2D::Texture2D::Create("assets/textures/icon.png");
	
	m_SpriteSheet = Runic2D::Texture2D::Create("assets/game/textures/tilemap_packed.png");
	m_ChestSubTexture = Runic2D::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0, 0 }, { 16, 16 });

	m_MapWidth = 16;
	m_MapHeight = strlen(s_MapTiles)/ m_MapWidth;
	m_CharSubTextures['#'] = Runic2D::SubTexture2D::CreateFromCoords(m_SpriteSheet, {1, 6}, {16, 16});
	m_CharSubTextures['.'] = Runic2D::SubTexture2D::CreateFromCoords(m_SpriteSheet, {6, 6}, {16, 16});


	//init particle
	m_Particle.ColorBegin = { 254.0f / 255.0f, 212.0f / 255.0f, 123.0f / 255.0f, 1.0f };
	m_Particle.ColorEnd = { 254.0f / 255.0f, 109.0f / 255.0f, 41.0f / 255.0f, 1.0f };
	m_Particle.SizeBegin = 0.5f, m_Particle.SizeVariation = 0.3f, m_Particle.SizeEnd = 0.0f;
	m_Particle.LifeTime = 1.0f;
	m_Particle.Velocity = { 0.0f, 0.0f };
	m_Particle.VelocityVariation = { 3.0f, 1.0f };
	m_Particle.Position = { 0.0f, 0.0f };

	m_CameraController.SetZoomLevel(5.0f);

	Runic2D::FrameBufferSpecification fbSpec;
	fbSpec.Width = 1280;
	fbSpec.Height = 720;
	m_FrameBuffer = Runic2D::FrameBuffer::Create(fbSpec);
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
		
		m_FrameBuffer->Bind();

		Runic2D::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Runic2D::RenderCommand::Clear();
	}

	{
		R2D_PROFILE_SCOPE("Renderer Draw")

		static float rotation = 0.0f;
		rotation += ts * 50.0f;

		Runic2D::Renderer2D::BeginScene(m_CameraController.GetCamera());

		Runic2D::Renderer2D::DrawRotatedQuad({ 0.5f, -0.5f , -0.1}, { 0.5f, 1.0f }, glm::radians(rotation), m_SquareColor);
		Runic2D::Renderer2D::DrawRotatedQuad({ 1.0f, 1.0f }, { 0.8f, 0.8f }, glm::radians(-rotation), m_SquareColor);
		Runic2D::Renderer2D::DrawQuad({ -1.0f, 1.0f }, { 1.0f, 1.0f }, m_Texture);
		Runic2D::Renderer2D::DrawQuad({ 0.0f, 0.0f }, { 0.8f, 0.8f }, m_RunicTexture, 1.0f);
		Runic2D::Renderer2D::DrawQuad({ 0.0f, 1.0f }, { 0.8f, 0.8f }, m_RunicTexture, 10.0f);
		Runic2D::Renderer2D::DrawRotatedQuad({ -2.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, 45.0f, m_Texture, 10.0f, glm::vec4(1.0f, 0.7f, 0.7f, 1.0f));

		Runic2D::Renderer2D::EndScene();

		//Runic2D::Renderer2D::BeginScene(m_CameraController.GetCamera());
		//for (float y = -5.0f; y < 5.0f; y += 0.25f) {
		//	for (float x = -5.0f; x < 5.0f; x += 0.25f) {
		//		glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.5f };
		//		Runic2D::Renderer2D::DrawQuad({ x, y }, { 0.2f, 0.2f }, color);
		//	}
		//}
		//Runic2D::Renderer2D::EndScene();

		//Draw tile map
		/*Runic2D::Renderer2D::BeginScene(m_CameraController.GetCamera());
		for(int32_t y = 0; y < m_MapHeight; y++)
		{
			for (int32_t x = 0; x < m_MapWidth; x++)
			{
				char tile = s_MapTiles[(y * m_MapWidth) + x];
				Runic2D::Ref<Runic2D::SubTexture2D> subTexture;
				if (m_CharSubTextures.find(tile) != m_CharSubTextures.end())
					subTexture = m_CharSubTextures[tile];
				else
					subTexture = m_ChestSubTexture;

				Runic2D::Renderer2D::DrawQuad({ x, (float)(y*-1), 0.3 }, { 1.0f, 1.0f }, subTexture);
			}
		}
		Runic2D::Renderer2D::EndScene();*/

		m_FrameBuffer->Unbind();
	}

	if(Runic2D::Input::IsMouseButtonPressed(R2D_MOUSE_BUTTON_LEFT))
	{
		auto [x, y] = Runic2D::Input::GetMousePosition();
		auto width = Runic2D::Application::Get().GetWindow().GetWidth();
		auto height = Runic2D::Application::Get().GetWindow().GetHeight();

		auto bounds = m_CameraController.GetBounds();
		auto camPos = m_CameraController.GetCamera().GetPosition();
		x = (x / width) * bounds.GetWidth() - bounds.GetWidth() * 0.5f;
		y = bounds.GetHeight() * 0.5f - (y / height) * bounds.GetHeight();
		m_Particle.Position = { x + camPos.x, y + camPos.y };
		for (int i = 0; i < 5; i++)
			m_ParticleSystem.Emit(m_Particle);
	}

	m_ParticleSystem.OnUpdate(ts);
	m_ParticleSystem.OnRender(m_CameraController.GetCamera());

}

void Sandbox2D::OnImGuiRender()
{
	R2D_PROFILE_FUNCTION();

	static bool dockspaceOpen = true;
	if (dockspaceOpen) {
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		uint32_t textureID = m_FrameBuffer->GetColorAttachmentRendererID();
		ImGui::Begin("Viewport");
		ImGui::Image((void*)(uintptr_t)textureID, ImVec2{ (float)m_FrameBuffer->GetSpecification().Width, (float)m_FrameBuffer->GetSpecification().Height }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		ImGui::End();

		ImGui::End();
	}
	else {
		float avaragefps = Runic2D::Application::Get().GetAverageFPS();
		auto stats = Runic2D::Renderer2D::GetStats();
		ImGui::Begin("Settings");
		ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
		ImGui::Text("Renderer2D Stats");
		ImGui::Text("Avarage FPS: %.2f", avaragefps);
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quad Count: %d", stats.QuadCount);
		ImGui::Text("Vertex Count: %d", stats.GetTotalVertexCount());
		ImGui::Text("Index Count: %d", stats.GetTotalIndexCount());
		ImGui::End();
	}
}

void Sandbox2D::OnEvent(Runic2D::Event& e)
{
	m_CameraController.OnEvent(e);
}
