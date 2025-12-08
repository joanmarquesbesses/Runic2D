#include "R2Dpch.h"
#include "Math.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Runic2D::Math
{
	bool Runic2D::Math::DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale)
	{
		using namespace glm;

		// Copia de la matriu per no modificar l'original
		mat4 LocalMatrix(transform);

		// Normalització (si l'escala és gairebé 0, fallem)
		if (epsilonEqual(LocalMatrix[3][3], static_cast<float>(0), epsilon<float>()))
			return false;

		// 1. Aïllar la Perspectiva (si n'hi hagués, la netegem)
		if (
			epsilonNotEqual(LocalMatrix[0][3], static_cast<float>(0), epsilon<float>()) ||
			epsilonNotEqual(LocalMatrix[1][3], static_cast<float>(0), epsilon<float>()) ||
			epsilonNotEqual(LocalMatrix[2][3], static_cast<float>(0), epsilon<float>()))
		{
			LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<float>(0);
			LocalMatrix[3][3] = static_cast<float>(1);
		}

		// 2. Extreure Translació
		translation = vec3(LocalMatrix[3]);
		LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

		vec3 Row[3];

		// 3. Extreure Escala i Cisalla (Shear)
		for (length_t i = 0; i < 3; ++i)
			for (length_t j = 0; j < 3; ++j)
				Row[i][j] = LocalMatrix[i][j];

		// Calcular escala i normalitzar files
		scale.x = length(Row[0]);
		Row[0] = detail::scale(Row[0], static_cast<float>(1));

		scale.y = length(Row[1]);
		Row[1] = detail::scale(Row[1], static_cast<float>(1));

		scale.z = length(Row[2]);
		Row[2] = detail::scale(Row[2], static_cast<float>(1));

		// (Opcional) Corregir si el sistema de coordenades està invertit (Determinant negatiu)
		/*
		vec3 Pdum3 = cross(Row[1], Row[2]);
		if (dot(Row[0], Pdum3) < 0)
		{
			for (length_t i = 0; i < 3; i++)
			{
				scale[i] *= static_cast<float>(-1);
				Row[i] *= static_cast<float>(-1);
			}
		}
		*/

		// 4. Extreure Rotació (LA PART NOVA I MILLORADA)
		// Ara 'Row' conté una matriu de rotació pura (sense escala ni translació).
		// La convertim a Quaternió i després a Euler Angles.

		// Construïm una matriu 4x4 pura de rotació a partir de les files normalitzades
		mat4 rotationMatrix(1.0f);
		rotationMatrix[0] = vec4(Row[0], 0.0f);
		rotationMatrix[1] = vec4(Row[1], 0.0f);
		rotationMatrix[2] = vec4(Row[2], 0.0f);

		quat orientation = quat_cast(rotationMatrix);

		// eulerAngles retorna radians (X, Y, Z aka Pitch, Yaw, Roll)
		rotation = eulerAngles(orientation);

		return true;
	}
}