#pragma once

namespace Runic2D {

	class VertexBuffer
	{
	public:
		static VertexBuffer* Create(const void* vertices, uint32_t size);
		virtual ~VertexBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
	};

	class IndexBuffer
	{
	public:
		static IndexBuffer* Create(const uint32_t* indices, uint32_t count);
		virtual ~IndexBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual uint32_t GetCount() const = 0;
	};

} // namespace Runic2D