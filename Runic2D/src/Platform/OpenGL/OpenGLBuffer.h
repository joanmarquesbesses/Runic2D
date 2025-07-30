#pragma once

#include "Runic2D/Renderer/Buffer.h"

namespace Runic2D {

	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(const void* vertices, uint32_t size);
		virtual ~OpenGLVertexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		uint32_t GetRendererID() const { return m_RendererID; }

	private:
		uint32_t m_RendererID;
	};

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(const uint32_t* indices, uint32_t count);
		virtual ~OpenGLIndexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual uint32_t GetCount() const override { return m_Count; }

		uint32_t GetRendererID() const { return m_RendererID; }
	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	};

} // namespace Runic2D