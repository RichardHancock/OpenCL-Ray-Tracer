#pragma once

#include "glm/glm.hpp"

/** @brief	A ray. */
struct Ray
{
	/** @brief	The origin of the ray. */
	glm::vec4 origin;

	/** @brief	The direction of the ray. */
	glm::vec4 direction;
};