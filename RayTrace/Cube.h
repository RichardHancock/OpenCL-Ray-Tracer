#pragma once

#include "glm/glm.hpp"
#include <vector>

class Cube
{
public:
	
	Cube(glm::vec3 colour);

	std::vector<glm::vec4> getTriangles();

	void rotate(glm::vec3 newRotation);

	void scale(glm::vec3 newScale);

	void translate(glm::vec3 newTranslation);

	glm::vec3 getColour() { return colour; }
private:

	std::vector<glm::vec4> triangles;

	glm::vec3 colour;
};