#pragma once

#include "Buffer.h"

namespace Runic2D {

	class RUNIC_API BufferStreamReader
	{
	public:
		BufferStreamReader(Buffer buffer)
			: m_Buffer(buffer), m_Cursor(0) {
		}

		template<typename T>
		void ReadRaw(T& type)
		{
			if (m_Cursor + sizeof(T) > m_Buffer.Size)
			{
				R2D_CORE_ERROR("BufferStreamReader: Buffer overflow at cursor {0}!", m_Cursor);
				memset(&type, 0, sizeof(T));
				return;
			}
			R2D_CORE_ASSERT(m_Cursor + sizeof(T) <= m_Buffer.Size, "Buffer overflow!");
			memcpy(&type, m_Buffer.Data + m_Cursor, sizeof(T));
			m_Cursor += sizeof(T);
		}

		void ReadData(void* dest, uint32_t size)
		{
			if (m_Cursor + size > m_Buffer.Size)
			{
				R2D_CORE_ERROR("BufferStreamReader: Buffer overflow reading {0} bytes at cursor {1}!", size, m_Cursor);
				memset(dest, 0, size);
				return;
			}
			R2D_CORE_ASSERT(m_Cursor + size <= m_Buffer.Size, "Buffer overflow!");
			memcpy(dest, m_Buffer.Data + m_Cursor, size);
			m_Cursor += size;
		}

		void ReadString(std::string& str)
		{
			uint32_t size;
			ReadRaw(size);
			str.resize(size);
			ReadData(str.data(), size);
		}

		uint32_t GetCursor() const { return m_Cursor; }
		void SetCursor(uint32_t cursor) { m_Cursor = cursor; }

	private:
		Buffer m_Buffer;
		uint32_t m_Cursor;
	};

}
