#pragma once

#include "State.h"

#include "../Triangle.h"
#include "../Ray.h"
#include <clew.h>
#include "../Texture.h"
#include "../Cube.h"
#include "../misc/PerformanceCounter.h"

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

	PerformanceCounter timer;
	uint64_t timeTaken;

	std::vector<int> pixels;
	int pixelCount;

	Texture* image;

	const int numOfTrianglesPerCube = 12;
	const int numOfPointsInTriangle = 3;

	glm::vec4 rayDir;
	std::vector<glm::vec4> rayOrigins;

	std::vector<glm::vec4> sphereOrigins;
	std::vector<float> sphereRadius;
	std::vector<glm::vec4> sphereColours;

	std::vector<Cube> cubes;

	//SceneCreation
	void createScene1();
	void createScene2();
	void createScene3();

	//OpenCL
	cl_program program;
	cl_context context;
	cl_command_queue cmdQueue;
	cl_kernel kernel;

	void executeRayTracerOpenCL();

	void executeRayTracerCPU();



	//OpenCL Utility Functions
	std::string loadComputeShaderFromFile(std::string path);

	std::string clDeviceTypeToString(cl_device_type type);

	//Debug Rendering to a PNG image
	void encodePNG(const char* filename, std::vector<unsigned char>& imageData, unsigned width, unsigned height);

	//Ray Tracer CPU Functions
	int intersectTri(double orig[3], double dir[3], double vert0[3], double vert1[3], double vert2[3], double *t, double *u, double *v);
	
	float intersectSphere(glm::vec4& rayOrigin, glm::vec4& rayDir, float sphereRadius, glm::vec4& sphereOrigin);

	glm::vec4 collide(Ray& ray, std::vector<Cube> cubes, 
		std::vector<float> sphereRadius, std::vector<glm::vec4> sphereOrigins, std::vector<glm::vec4> sphereColours);
};