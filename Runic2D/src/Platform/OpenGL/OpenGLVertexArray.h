#pragma once

#include "Runic2D/Renderer/VertexArray.h"

namespace Runic2D {

	class OpenGLVertexArray : public Runic2D::VertexArray
	{
	public:
		OpenGLVertexArray();
		virtual ~OpenGLVertexArray();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void AddVertexBuffer(const std::shared_ptr<Runic2D::VertexBuffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(const std::shared_ptr<Runic2D::IndexBuffer>& indexBuffer) override;

		virtual const std::vector<std::shared_ptr<Runic2D::VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
		virtual const std::shared_ptr<Runic2D::IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; }
	private:
		uint32_t m_RendererID;
		std::vector<std::shared_ptr<Runic2D::VertexBuffer>> m_VertexBuffers;
		std::shared_ptr<Runic2D::IndexBuffer> m_IndexBuffer;
		uint32_t m_IndexCount = 0;
	};

}