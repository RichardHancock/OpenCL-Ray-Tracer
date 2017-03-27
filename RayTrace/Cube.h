#pragma once

#include "glm/glm.hpp"
#include <vector>

/** @brief	A representation of a cube. */
class Cube
{
public:

	/**
	 @brief	Constructor.
	
	 @param [in,out]	colour	The colour.
	 */
	Cube(glm::vec4& colour);

	/**
	 @brief	Gets the triangles that form this cube.
	
	 @return	The triangles vertices.
	 */
	std::vector<glm::vec4> getTriangles();

	/**
	 @brief	Rotates the cube by the given new rotation.
	
	 @param	newRotation	The new rotation.
	 */
	void rotate(glm::vec3 newRotation);

	/**
	 @brief	Scales the cube by the given new scale.
	
	 @param	newScale	The new scale.
	 */
	void scale(glm::vec3 newScale);

	/**
	 @brief	Translates the cube by the given new translation.
	
	 @param	newTranslation	The new translation.
	 */
	void translate(glm::vec3 newTranslation);

	/**
	 @brief	Gets the colour of the cube.
	
	 @return	The colour.
	 */
	glm::vec4 getColour() { return colour; }
private:

	/** @brief	The triangles vertices. */
	std::vector<glm::vec4> triangles;

	/** @brief	The colour of the cube. */
	glm::vec4 colour;
};