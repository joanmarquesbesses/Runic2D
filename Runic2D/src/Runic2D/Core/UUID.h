#pragma once

#include <xhash> // Per hashing si cal

namespace Runic2D {

	class UUID
	{
	public:
		UUID(); // Genera un nou ID aleatori
		UUID(uint64_t uuid); // Constructor per crear un UUID existent (deserialització)
		UUID(const UUID&) = default;

		operator uint64_t() const { return m_UUID; }
	private:
		uint64_t m_UUID;
	};

}

// Hashing personalitzat perquè UUID pugui ser clau en un std::unordered_map
namespace std {
	template<>
	struct hash<Runic2D::UUID>
	{
		std::size_t operator()(const Runic2D::UUID& uuid) const
		{
			return hash<uint64_t>()((uint64_t)uuid);
		}
	};
}