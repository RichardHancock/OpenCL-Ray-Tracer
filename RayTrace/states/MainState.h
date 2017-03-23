#pragma once

#include "State.h"

#include "../Triangle.h"
#include "../Ray.h"
#include <clew.h>
#include "../Texture.h"

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

	Triangle tri;
	std::vector<unsigned char> pixels;

	Texture* result;

	std::string loadComputeShaderFromFile(std::string path);

	std::string clDeviceTypeToString(cl_device_type type);

	void encodePNG(const char* filename, std::vector<unsigned char>& image, unsigned width, unsigned height);

	int intersectTri(double orig[3], double dir[3], double vert0[3], double vert1[3], double vert2[3], double *t, double *u, double *v);
	
	int collide(Ray& ray);
};