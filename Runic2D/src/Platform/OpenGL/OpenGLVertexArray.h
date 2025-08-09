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

		virtual void AddVertexBuffer(const Ref<Runic2D::VertexBuffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(const Ref<Runic2D::IndexBuffer>& indexBuffer) override;

		virtual const std::vector<Ref<Runic2D::VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
		virtual const Ref<Runic2D::IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; }
	private:
		uint32_t m_RendererID;
		std::vector<Ref<Runic2D::VertexBuffer>> m_VertexBuffers;
		Ref<Runic2D::IndexBuffer> m_IndexBuffer;
		uint32_t m_IndexCount = 0;
	};

}