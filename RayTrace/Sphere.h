#pragma once

#include <vector>
#include "glm/glm.hpp"

struct Spheres
{
	std::vector<glm::vec4> origins;
	std::vector<float> radius;
	std::vector<glm::vec4> colours;
};