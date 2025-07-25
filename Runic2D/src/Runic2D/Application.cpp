#include "R2Dpch.h"
#include "Application.h"

#include <glad/glad.h>

#include "Input.h"

namespace Runic2D {

#define R2D_BIND_EVENT_FN(fn) std::bind(&Application::fn, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		R2D_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;
		R2D_CORE_INFO("Runic2D Engine Initialized");

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(R2D_BIND_EVENT_FN(OnEvent));
		
		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		// Vertex and index data
		float vertices[3 * 3] = {
			-0.5f, -0.5f, 0.0f,
			 0.5f, -0.5f, 0.0f,
			 0.0f,  0.5f, 0.0f
		};

		unsigned int indices[3] = { 0, 1, 2 };

		// Create Vertex Array Object (VAO)
		glCreateVertexArrays(1, &m_VertexArray);

		// Create and fill Vertex Buffer Object (VBO)
		glCreateBuffers(1, &m_VertexBuffer);
		glNamedBufferData(m_VertexBuffer, sizeof(vertices), vertices, GL_STATIC_DRAW);

		// Link VBO to VAO
		glVertexArrayVertexBuffer(m_VertexArray, 0, m_VertexBuffer, 0, 3 * sizeof(float));
		glEnableVertexArrayAttrib(m_VertexArray, 0);
		glVertexArrayAttribFormat(m_VertexArray, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(m_VertexArray, 0, 0); // Attribute 0 uses binding index 0

		// Create and fill Index Buffer Object (IBO/EBO)
		glCreateBuffers(1, &m_IndexBuffer);
		glNamedBufferData(m_IndexBuffer, sizeof(indices), indices, GL_STATIC_DRAW);

		// Link IBO to VAO
		glVertexArrayElementBuffer(m_VertexArray, m_IndexBuffer);

		std::string vertexSrc = R"(
			#version 450 core
			layout(location = 0) in vec3 a_Position;
			out vec3 v_Position;
			void main() {
				v_Position = a_Position;
				gl_Position = vec4(a_Position, 1.0);
			}
		)";

		std::string fragmentSrc = R"(
			#version 450 core
			out vec4 color;
			in vec3 v_Position;
			void main() {
				color = vec4(v_Position * 0.5 + 0.5, 1.0); // Red color
			}
		)";

		m_Shader.reset(new Shader(vertexSrc, fragmentSrc));
	}

	Application::~Application()
	{
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay)
	{
		m_LayerStack.PushOverlay(overlay);
		overlay->OnAttach();
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(R2D_BIND_EVENT_FN(OnWindowClose));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();) {
			(*--it)->OnEvent(e);
			if (e.Handled())
				break;
		}
	}

	void Application::Run()
	{
		while (m_Running) {
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			m_Shader->Bind();
			glBindVertexArray(m_VertexArray);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);

			for (Layer* layer : m_LayerStack) {
				layer->OnUpdate();
			}

			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack) {
				layer->OnImGuiRender();
			}
			m_ImGuiLayer->End();

			m_Window->OnUpdate();
		}
	}

}
