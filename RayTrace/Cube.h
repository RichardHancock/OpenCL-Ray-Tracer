#pragma once

#include "glm/glm.hpp"
#include <vector>

class Cube
{
public:
	
	Cube(glm::vec4& colour);

	std::vector<glm::vec4> getTriangles();

	void rotate(glm::vec3 newRotation);

	void scale(glm::vec3 newScale);

	void translate(glm::vec3 newTranslation);

	glm::vec4 getColour() { return colour; }
private:

	std::vector<glm::vec4> triangles;

	glm::vec4 colour;
};