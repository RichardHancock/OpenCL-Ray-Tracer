#include "Cube.h"

#include "glm/gtc/matrix_transform.hpp"
#include <iostream>

Cube::Cube(glm::vec4& newColour)
	: colour(newColour)
{
	triangles.reserve(36);
	triangles.push_back(glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f));//tri 1 start
	triangles.push_back(glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f));
	triangles.push_back(glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f));//tri 1 end
	triangles.push_back(glm::vec4(1.0f, 1.0f, -1.0f, 1.0f));
	triangles.push_back(glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f));
	triangles.push_back(glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f));
	triangles.push_back(glm::vec4(1.0f, -1.0f, 1.0f, 1.0f));
	triangles.push_back(glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f));
	triangles.push_back(glm::vec4(1.0f, -1.0f, -1.0f, 1.0f));
	triangles.push_back(glm::vec4(1.0f, 1.0f, -1.0f, 1.0f));
	triangles.push_back(glm::vec4(1.0f, -1.0f, -1.0f, 1.0f));
	triangles.push_back(glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f));
	triangles.push_back(glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f));
	triangles.push_back(glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f));
	triangles.push_back(glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f));
	triangles.push_back(glm::vec4(1.0f, -1.0f, 1.0f, 1.0f));
	triangles.push_back(glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f));
	triangles.push_back(glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f));
	triangles.push_back(glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f));
	triangles.push_back(glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f));
	triangles.push_back(glm::vec4(1.0f, -1.0f, 1.0f, 1.0f));
	triangles.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	triangles.push_back(glm::vec4(1.0f, -1.0f, -1.0f, 1.0f));
	triangles.push_back(glm::vec4(1.0f, 1.0f, -1.0f, 1.0f));
	triangles.push_back(glm::vec4(1.0f, -1.0f, -1.0f, 1.0f));
	triangles.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	triangles.push_back(glm::vec4(1.0f, -1.0f, 1.0f, 1.0f));
	triangles.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	triangles.push_back(glm::vec4(1.0f, 1.0f, -1.0f, 1.0f));
	triangles.push_back(glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f));
	triangles.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	triangles.push_back(glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f));
	triangles.push_back(glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f));
	triangles.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	triangles.push_back(glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f));
	triangles.push_back(glm::vec4(1.0f, -1.0f, 1.0f, 1.0f));
}

std::vector<glm::vec4> Cube::getTriangles()
{
	return triangles;
}

void Cube::rotate(glm::vec3 newRotation)
{
	glm::mat4 rotationMat = glm::rotate(glm::mat4(1.0f), newRotation.z, glm::vec3(0, 0, 1));
	rotationMat = glm::rotate(rotationMat, newRotation.y, glm::vec3(0, 1, 0));
	rotationMat = glm::rotate(rotationMat, newRotation.x, glm::vec3(1, 0, 0));

	for (auto& triangle : triangles)
	{
		triangle = rotationMat * triangle;
	}
}

void Cube::scale(glm::vec3 newScale)
{
	glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), newScale);

	for (auto& trianglePoint : triangles)
	{
		trianglePoint = scaleMat * trianglePoint;
	}
}

void Cube::translate(glm::vec3 newtranslation)
{
	glm::mat4 translateMat = glm::translate(glm::mat4(1.0f), newtranslation);

	for (auto& trianglePoint : triangles)
	{
		trianglePoint = translateMat * trianglePoint;
	}
}
