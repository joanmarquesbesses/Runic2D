#pragma once

#include "Runic2D/Core/Core.h"

namespace Runic2D {

	struct RUNIC_API Buffer {
		uint8_t* Data = nullptr;
		uint32_t Size = 0;

		Buffer() {};

		Buffer(uint32_t size) {
			Allocate(size);
		}

		static Buffer Copy(const void* data, uint32_t size) {
			Buffer buffer;
			buffer.Allocate(size);
			memcpy(buffer.Data, data, size);
			return buffer;
		}

		void Allocate(uint32_t size) {
			Release();
			Data = new uint8_t[size];
			Size = size;
		}

		void Release() {
			if (Data) {
				delete[] Data;
				Data = nullptr;
			}
			Size = 0;
		}

		template<typename T>
		T* As() const {
			return (T*)Data;
		}

		operator bool() const {
			return Data != nullptr;
		}
		
	};
}