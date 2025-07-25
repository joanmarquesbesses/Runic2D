#pragma once
#include <string>

namespace Runic2D {

	class Shader
	{
	public:
		Shader(const std::string& vertexSource, const std::string& fragmentSource);
		~Shader();

		void Bind() const;
		void Unbind() const;
	private:
		uint32_t m_RendererID;
	};

} // namespace Runic2D