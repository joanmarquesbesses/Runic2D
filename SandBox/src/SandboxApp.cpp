#include <Runic2D.h>
#include <Runic2D/Core/EntryPoint.h>

#include "Platform/OpenGL/OpenGLShader.h"

#include "imgui/imgui.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Sandbox2D.h"

class ExampleLayer : public Runic2D::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_CameraController(1280.0f/720.0f, true),
		m_SquarePosition(0.0f)
	{
		m_SquareVA = Runic2D::VertexArray::Create();

		float squareVertices[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};

		Runic2D::Ref<Runic2D::VertexBuffer> squareVB;
		squareVB.reset(Runic2D::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));

		Runic2D::BufferLayout squareLayout = {
			{ Runic2D::ShaderDataType::Float3, "a_Position" },
			{ Runic2D::ShaderDataType::Float2, "a_TexCoord" }
		};
		squareVB->SetLayout(squareLayout);

		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = {
			0, 1, 2, // First Triangle
			2, 3, 0  // Second Triangle
		};

		Runic2D::Ref<Runic2D::IndexBuffer> squareIB;
		squareIB.reset(Runic2D::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareVA->SetIndexBuffer(squareIB);

		auto m_TextureShader = m_ShaderLibrary.Load("assets/shaders/Texture.glsl");

		m_Texture = Runic2D::Texture2D::Create("assets/textures/Check.png");
		std::dynamic_pointer_cast<Runic2D::OpenGLShader>(m_TextureShader)->Bind();
		std::dynamic_pointer_cast<Runic2D::OpenGLShader>(m_TextureShader)->UploadUniformInt("u_Texture", 0);
	}

	void OnUpdate(Runic2D::Timestep ts) override
	{
		//update
		m_CameraController.OnUpdate(ts);

		//render
		if (Runic2D::Input::IsKeyPressed(R2D_KEY_J))
			m_SquarePosition.x -= m_SquareMoveSpeed * ts;

		else if (Runic2D::Input::IsKeyPressed(R2D_KEY_L))
			m_SquarePosition.x += m_SquareMoveSpeed * ts;

		else if (Runic2D::Input::IsKeyPressed(R2D_KEY_I))
			m_SquarePosition.y += m_SquareMoveSpeed * ts;

		else if (Runic2D::Input::IsKeyPressed(R2D_KEY_K))
			m_SquarePosition.y -= m_SquareMoveSpeed * ts;

		Runic2D::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		Runic2D::RenderCommand::Clear();

		Runic2D::Renderer::BeginScene(m_CameraController.GetCamera());

		auto m_TextureShader = m_ShaderLibrary.Get("Texture");
		std::dynamic_pointer_cast<Runic2D::OpenGLShader>(m_TextureShader)->Bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_SquarePosition);

		m_Texture->Bind(); // Bind the texture before submitting the square
		Runic2D::Renderer::Submit(m_TextureShader, m_SquareVA, transform);

		Runic2D::Renderer::EndScene();

	}

	void OnEvent(Runic2D::Event& e) override
	{
		m_CameraController.OnEvent(e);
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Settings");

		ImGui::Text("Square Position: (%.2f, %.2f)", m_SquarePosition.x, m_SquarePosition.y);

		ImGui::End();
	}

private:
	//drawing triangle
	Runic2D::ShaderLibrary m_ShaderLibrary; // Shader library to manage shaders
	Runic2D::Ref<Runic2D::VertexArray> m_SquareVA;

	Runic2D::Ref<Runic2D::Texture2D> m_Texture; // Texture for the square

	Runic2D::OrthographicCameraController m_CameraController; // Orthographic camera for 2D rendering

	glm::vec3 m_SquarePosition; // Position of the square
	float m_SquareMoveSpeed = 1.0f; // Speed of square movement
};

class SandboxApp : public Runic2D::Application
{
public:
	SandboxApp()
	{
		// Constructor for SandboxApp, can be used to initialize the application.
		//PushLayer(new ExampleLayer()); // Adding an instance of ExampleLayer to the application.
		PushLayer(new Sandbox2D());
	}

	virtual ~SandboxApp()
	{
		// Destructor for SandboxApp, can be used for cleanup.
	}
};

Runic2D::Application* Runic2D::CreateApplication()
{
	// This function creates an instance of SandboxApp, which is the application to run.
	return new SandboxApp();
}