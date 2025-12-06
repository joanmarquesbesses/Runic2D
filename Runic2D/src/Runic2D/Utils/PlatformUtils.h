#pragma once

#include <string>

namespace Runic2D {
	
	class FileDialogs {
	public:
		// Returns empty string if cancelled
		static std::string OpenFile(const char* filter);
		static std::string SaveFile(const char* filter);
	};

} // namespace Runic2D