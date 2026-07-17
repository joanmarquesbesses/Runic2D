#pragma once

#include "Buffer.h"

#include <vector>

namespace Runic2D {

	class RUNIC_API BufferStreamWriter
	{
	public:
		BufferStreamWriter() = default;

		template<typename T>
		void WriteRaw(const T& type)
		{
			size_t size = sizeof(T);
			size_t currentSize = m_Buffer.size();
			m_Buffer.resize(currentSize + size);
			memcpy(m_Buffer.data() + currentSize, &type, size);
		}

		void WriteData(const void* data, uint32_t size)
		{
			size_t currentSize = m_Buffer.size();
			m_Buffer.resize(currentSize + size);
			memcpy(m_Buffer.data() + currentSize, data, size);
		}

		void WriteString(const std::string& str)
		{
			uint32_t size = (uint32_t)str.size();
			WriteRaw(size);
			WriteData(str.data(), size);
		}

		Buffer GetBuffer() const
		{
			return Buffer::Copy(m_Buffer.data(), (uint32_t)m_Buffer.size());
		}

	private:
		std::vector<uint8_t> m_Buffer;
	};

}
