#pragma once

#include "State.h"

#include "../Triangle.h"
#include "../Ray.h"
#include <clew.h>
#include "../Texture.h"
#include "../Cube.h"

class StateManager;

class MainState : public State
{
public:
	MainState(StateManager* manager, Platform* platform);

	virtual ~MainState();

	/**
	@brief Handles any events such as keyboard/mouse input
	@return Continue the game
	*/
	virtual bool eventHandler();

	/**
	@brief Update any internal values
	*/
	virtual void update();

	/**
	@brief Render any sprites relevant to the state
	*/
	virtual void render();
protected:

	std::vector<int> pixels;

	Texture* image;

	const int numOfTrianglesPerCube = 12;
	const int numOfPointsInTriangle = 3;

	//OpenCL
	cl_program program;
	cl_context context;
	cl_command_queue cmdQueue;
	cl_kernel kernel;

	std::string loadComputeShaderFromFile(std::string path);

	std::string clDeviceTypeToString(cl_device_type type);

	void encodePNG(const char* filename, std::vector<unsigned char>& imageData, unsigned width, unsigned height);

	int intersectTri(double orig[3], double dir[3], double vert0[3], double vert1[3], double vert2[3], double *t, double *u, double *v);
	
	float intersectSphere(glm::vec4& rayOrigin, glm::vec4& rayDir, float sphereRadius, glm::vec4& sphereOrigin);

	glm::vec4 collide(Ray& ray, std::vector<Cube> cubes, 
		std::vector<float> sphereRadius, std::vector<glm::vec4> sphereOrigins, std::vector<glm::vec4> sphereColours);
};