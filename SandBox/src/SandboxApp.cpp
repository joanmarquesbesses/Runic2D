#include <Runic2D.h>

#include "imgui/imgui.h"

class ExampleLayer : public Runic2D::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f), m_CameraPosition(0.0f)
	{
		// Constructor for ExampleLayer, can be used to initialize the layer.
		m_VertexArray.reset(Runic2D::VertexArray::Create());

		// Vertex and index data
		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f, // Vertex 1: Position (x, y, z) and Color (r, g, b, a)
			 0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f, // Vertex 2: Position (x, y, z) and Color (r, g, b, a)
			 0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f  // Vertex 3: Position (x, y, z) and Color (r, g, b, a)
		};

		std::shared_ptr<Runic2D::VertexBuffer> m_VertexBuffer;
		m_VertexBuffer.reset(Runic2D::VertexBuffer::Create(vertices, sizeof(vertices)));

		Runic2D::BufferLayout layout = {
			{ Runic2D::ShaderDataType::Float3, "a_Position" },
			{ Runic2D::ShaderDataType::Float4, "a_Color" }
		};
		m_VertexBuffer->SetLayout(layout);

		m_VertexArray->AddVertexBuffer(m_VertexBuffer);

		uint32_t indices[3] = { 0, 1, 2 };

		std::shared_ptr<Runic2D::IndexBuffer> m_IndexBuffer;
		m_IndexBuffer.reset(Runic2D::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));;
		m_VertexArray->SetIndexBuffer(m_IndexBuffer);

		m_SquareVA.reset(Runic2D::VertexArray::Create());

		float squareVertices[4 * 7] = {
			-0.5f, -0.5f, 0.0f, 0.2f, 0.2f, 0.8f, 1.0f, // Bottom Left
			 0.5f, -0.5f, 0.0f, 0.5f, 0.3f, 0.3f, 1.0f, // Bottom Right
			 0.5f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f, // Top Right
			-0.5f,  0.5f, 0.0f, 0.3f, 0.2f, 0.5f, 1.0f // Top Left
		};

		std::shared_ptr<Runic2D::VertexBuffer> squareVB;
		squareVB.reset(Runic2D::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));

		Runic2D::BufferLayout squareLayout = {
			{ Runic2D::ShaderDataType::Float3, "a_Position" },
			{ Runic2D::ShaderDataType::Float4, "a_Color" }
		};
		squareVB->SetLayout(squareLayout);

		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = {
			0, 1, 2, // First Triangle
			2, 3, 0  // Second Triangle
		};

		std::shared_ptr<Runic2D::IndexBuffer> squareIB;
		squareIB.reset(Runic2D::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareVA->SetIndexBuffer(squareIB);

		std::string vertexSrc = R"(
			#version 450 core
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;
			uniform mat4 u_ViewProjection;
			out vec3 v_Position;
			out vec4 v_Color; 
			void main() {
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * vec4(a_Position, 1);
			}
		)";

		std::string fragmentSrc = R"(
			#version 450 core
			in vec3 v_Position;
			in vec4 v_Color;
			out vec4 color;
			void main() {
				color = v_Color;
			}
		)";

		m_Shader.reset(new Runic2D::Shader(vertexSrc, fragmentSrc));
	}

	void OnUpdate() override
	{
		if (Runic2D::Input::IsKeyPressed(R2D_KEY_LEFT))
			m_CameraPosition.x -= m_CameraMoveSpeed;

		else if (Runic2D::Input::IsKeyPressed(R2D_KEY_RIGHT))
			m_CameraPosition.x += m_CameraMoveSpeed;

		else if (Runic2D::Input::IsKeyPressed(R2D_KEY_UP))
			m_CameraPosition.y += m_CameraMoveSpeed;

		else if (Runic2D::Input::IsKeyPressed(R2D_KEY_DOWN))
			m_CameraPosition.y -= m_CameraMoveSpeed;

		if (Runic2D::Input::IsKeyPressed(R2D_KEY_A))
			m_CameraRotation -= m_CameraSpeed;

		else if (Runic2D::Input::IsKeyPressed(R2D_KEY_D))
			m_CameraRotation += m_CameraSpeed;

		Runic2D::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		Runic2D::RenderCommand::Clear();

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);

		Runic2D::Renderer::BeginScene(m_Camera);

		Runic2D::Renderer::Submit(m_Shader, m_SquareVA);
		Runic2D::Renderer::Submit(m_Shader, m_VertexArray);

		Runic2D::Renderer::EndScene();

	}

	void OnEvent(Runic2D::Event& event) override
	{
		/*Runic2D::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Runic2D::KeyPressedEvent>(RUNIC2D_BIND_EVENT_FN(ExampleLayer::OnKeyPressedEvent));*/
	}

	void OnImGuiRender() override
	{

	}

private:
	//drawing triangle
	std::shared_ptr<Runic2D::Shader> m_Shader;
	std::shared_ptr<Runic2D::VertexArray> m_VertexArray;
	std::shared_ptr<Runic2D::VertexArray> m_SquareVA;

	Runic2D::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition; // Camera position for the orthographic camera
	float m_CameraMoveSpeed = 0.1f; // Speed of camera movement
	float m_CameraRotation = 0.0f; // Camera rotation angle in degrees
	float m_CameraSpeed = 2.0f; // Speed of camera movement
};

class SandboxApp : public Runic2D::Application
{
public:
	SandboxApp()
	{
		// Constructor for SandboxApp, can be used to initialize the application.
		PushLayer(new ExampleLayer()); // Adding an instance of ExampleLayer to the application.
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