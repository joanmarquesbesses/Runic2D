#pragma once

#include <string>

namespace Runic2D {
	
	class RUNIC_API FileDialogs {
	public:
		// Returns empty string if cancelled
		static std::string OpenFile(const char* filter, const char* initialDir = nullptr);
		static std::string SaveFile(const char* filter, const char* initialDir = nullptr);
	};

} // namespace Runic2D