#include "MainState.h"

#include <fstream>
#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp"
#include "../glm/gtc/type_ptr.hpp"

#include "../lodepng.h"
#include "../misc/Utility.h"
#include "../misc/Random.h"


//Macros
// Ref: http://cs.lth.se/tomas_akenine-moller
#define EPSILON 0.000001
#define CROSS(dest,v1,v2) \
          dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
          dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
          dest[2]=v1[0]*v2[1]-v1[1]*v2[0];
#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])
#define SUB(dest,v1,v2) \
          dest[0]=v1[0]-v2[0]; \
          dest[1]=v1[1]-v2[1]; \
          dest[2]=v1[2]-v2[2]; 

MainState::MainState(StateManager * manager, Platform * platform)
	: State(manager, platform)
{
	image = nullptr;

	stateName = "Main State";

	//Ray Tracer Init
	pixelCount = (int)(platform->getWindowSize().x * platform->getWindowSize().y);
	

	glm::mat4 proj = glm::perspective(45.0f, 4.0f / 3.0f, 0.0f, 100.0f);

	rayDir = proj * glm::vec4(0, 0, 1, 1);

	rayOrigins.reserve(pixelCount); // 1 ray for each pixel

	//Create Rays
	for (unsigned int y = 0; y < platform->getWindowSize().y; y++)
	{
		for (unsigned int x = 0; x < platform->getWindowSize().x; x++)
		{
			rayOrigins.push_back(glm::vec4(x, y, 0.0f, 1.0f));
		}
	}

	openCLInit();

	//UI
	font = TTF_OpenFont("resources/fonts/OpenSans-Regular.ttf", 24);
	textColour.r = 255;
	textColour.g = 255;
	textColour.b = 255;

	mode = new Texture(TTF_RenderText_Blended(font, "Mode: CPU", textColour), platform->getRenderer());
	modeSwitch = new Texture(TTF_RenderText_Blended(font, "F1 to Switch", textColour), platform->getRenderer());
	timeTakenUI = new Texture(TTF_RenderText_Blended(font, "Time: N/A", textColour), platform->getRenderer());
	sceneNumberUI = new Texture(TTF_RenderText_Blended(font, "Scene 1", textColour), platform->getRenderer());
	sceneSwitch = new Texture(TTF_RenderText_Blended(font, "F2 to Switch", textColour), platform->getRenderer());
	pleaseWait = new Texture(TTF_RenderText_Blended(font, "Please Wait...", textColour), platform->getRenderer());

	start = true;
	rayTracingInProgress = false;
	currentScene = 1;
	sceneChange = true;
}

MainState::~MainState()
{
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(cmdQueue);
	clReleaseContext(context);


	TTF_CloseFont(font);
	delete mode;
	delete modeSwitch;
	delete timeTakenUI;
	delete sceneNumberUI;
	delete sceneSwitch;
	delete pleaseWait;

	if (image != nullptr)
		delete image;
}

bool MainState::eventHandler()
{
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
		case SDL_QUIT:
			return true;
			break;

		case SDL_KEYUP:
		case SDL_KEYDOWN:
			InputManager::processKeyEvent(e);
			break;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEMOTION:
		case SDL_MOUSEWHEEL:
			InputManager::processMouseEvent(e);

			break;

		case SDL_CONTROLLERAXISMOTION:
		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
		case SDL_CONTROLLERDEVICEADDED:
		case SDL_CONTROLLERDEVICEREMOVED:
			InputManager::processGameControllerEvent(e);
			break;
		}
	}

	if (InputManager::wasKeyReleased(SDLK_ESCAPE))
	{
		return true;
	}

	return false;
}

void MainState::update()
{
	if (InputManager::wasKeyReleased(SDLK_F1) && !start && !rayTracingInProgress)
	{
		//Switch mode 
		currentMode = (currentMode == CPU ? OpenCL : CPU);

		delete mode;

		if (currentMode == CPU)
		{
			mode = new Texture(TTF_RenderText_Blended(font, "Mode: CPU", textColour), platform->getRenderer());
		}
		else
		{
			mode = new Texture(TTF_RenderText_Blended(font, "Mode: OpenCL", textColour), platform->getRenderer());
		}

		start = true;
	}

	if (InputManager::wasKeyReleased(SDLK_F2) && !start && !rayTracingInProgress)
	{
		currentScene++; // Increment to next scene

		if (currentScene > 3) // if past last scene return to first
			currentScene = 1;


		std::string sceneNumberStr = "Scene " + Utility::intToString(currentScene);
		delete sceneNumberUI;
		sceneNumberUI = new Texture(TTF_RenderText_Blended(font, sceneNumberStr.c_str(), textColour), platform->getRenderer());

		std::cout << "Scene changed to " << sceneNumberStr << std::endl;

		sceneChange = true;
		start = true;
	}

	if (InputManager::wasKeyReleased(SDLK_SPACE) && !start && !rayTracingInProgress)
	{
		start = true;
	}


	if (rayTracingInProgress)
	{
		//Clear Previous image if any
		if (image != nullptr)
			delete image;

		if (sceneChange) // If the scene has been changed, rebuild scene
		{
			//Clear previous scene
			sphereOrigins.clear();
			sphereRadius.clear();
			sphereColours.clear();
			cubes.clear();

			switch (currentScene)
			{
			case 1:
				createScene1();
				break;
			case 2:
				createScene2();
				break;
			case 3:
				createScene3();
				break;
			default:
				std::cout << "Invalid Scene Requested" << std::endl;
			}

			sceneChange = false;
		}

		//Prepare pixel array
		pixels.clear();
		//Dimensions * 4 Bytes (RGBA)
		pixels.reserve(pixelCount * 4);

		if (currentMode == CPU)
		{
			executeRayTracerCPU();
		}
		else
		{
			executeRayTracerOpenCL();
		}

		generateImageFromPixels();

		rayTracingInProgress = false;
	}

	//Having this separated allows me to render the please wait message
	// as program will pause when waiting for ray tracer to finish.
	if (start)
	{
		start = false;
		rayTracingInProgress = true;
	}
	
}

void MainState::render()
{
	if (image != nullptr)
		image->draw(Vec2(0.0f));

	mode->draw(Vec2(5.0f, 450.0f));
	modeSwitch->draw(Vec2(5.0f, 410.0f));
	timeTakenUI->draw(Vec2(240.0f, 450.0f));
	sceneNumberUI->draw(Vec2(500.0f, 450.0f));
	sceneSwitch->draw(Vec2(500.0f, 410.0f));

	if (rayTracingInProgress)
		pleaseWait->draw(Vec2(240.0f, 230.0f));
}

//Ref: http://cs.lth.se/tomas_akenine-moller
int MainState::intersectTri(double orig[3], double dir[3],
	double vert0[3], double vert1[3], double vert2[3],
	double *t, double *u, double *v)
{
	double edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
	double det, inv_det;

	/* find vectors for two edges sharing vert0 */
	SUB(edge1, vert1, vert0);
	SUB(edge2, vert2, vert0);

	/* begin calculating determinant - also used to calculate U parameter */
	CROSS(pvec, dir, edge2);

	/* if determinant is near zero, ray lies in plane of triangle */
	det = DOT(edge1, pvec);

	if (det > -EPSILON && det < EPSILON)
		return 0;
	inv_det = 1.0 / det;

	/* calculate distance from vert0 to ray origin */
	SUB(tvec, orig, vert0);

	/* calculate U parameter and test bounds */
	*u = DOT(tvec, pvec) * inv_det;
	if (*u < 0.0 || *u > 1.0)
		return 0;

	/* prepare to test V parameter */
	CROSS(qvec, tvec, edge1);

	/* calculate V parameter and test bounds */
	*v = DOT(dir, qvec) * inv_det;
	if (*v < 0.0 || *u + *v > 1.0)
		return 0;

	/* calculate t, ray intersects triangle */
	*t = DOT(edge2, qvec) * inv_det;

	return 1;
}

float MainState::intersectSphere(glm::vec4& inRayOrigin, glm::vec4& inRayDirection, float inSphereRadius, glm::vec4& inSphereOrigin)
{
		glm::vec4 L = inSphereOrigin - inRayOrigin;
		float tca = glm::dot(L, inRayDirection);

		if (tca < 0)
		{
			return 0.0f;
		}

		float distanceSquared = glm::dot(L, L) - tca * tca;
		float radiusSquared = inSphereRadius * inSphereRadius;

		if (distanceSquared > radiusSquared)
		{
			return 0.0f;
		}

		float thc = sqrt((radiusSquared) - distanceSquared);
		float t0 = tca - thc;
		//float t1 = tca + thc;

		//return glm::distance(t0, t1);
		return t0;


		//distance = Utility::normaliseFloat(distance, s.radius + s.radius, 0.0f) * 255.0f;
}

//Quick function to convert params to be suitable for the intersect code
glm::vec4 MainState::collide(Ray& inRay, std::vector<Cube> inCubes, 
	std::vector<float> inSphereRadius, std::vector<glm::vec4> inSphereOrigins, std::vector<glm::vec4> inSphereColours)
{
	double rayOrigin[3]{ inRay.origin.x, inRay.origin.y, inRay.origin.z };
	double rayDirection[3]{ inRay.direction.x, inRay.direction.y, inRay.direction.z };

	double tri0[3];
	double tri1[3];
	double tri2[3];

	double t = 0;
	double u = 0;
	double v = 0;

	glm::vec4 closestColour = glm::vec4(0, 0, 0, 255.0f);
	float closest = 300000.0f; //Set to high number so it will always be beaten

	//Process Cubes
	for (auto cube : cubes)
	{
		std::vector<glm::vec4> tris = cube.getTriangles(); //Get all tri verts from cube

		for (unsigned int triIndex = 0; triIndex < tris.size(); triIndex += 3)
		{
			//Set Triangles into array format for intersect test
			tri0[0] = tris[triIndex].x;
			tri0[1] = tris[triIndex].y;
			tri0[2] = tris[triIndex].z;

			tri1[0] = tris[triIndex + 1].x;
			tri1[1] = tris[triIndex + 1].y;
			tri1[2] = tris[triIndex + 1].z;

			tri2[0] = tris[triIndex + 2].x;
			tri2[1] = tris[triIndex + 2].y;
			tri2[2] = tris[triIndex + 2].z;

			//If ray intersects triangle set this triangle as the closest
			if (intersectTri(rayOrigin, rayDirection, tri0, tri1, tri2, &t, &u, &v) == 1)
			{
				if ((float)t < closest)
				{
					closest = (float)t;
					closestColour = cube.getColour();
				}
			}
		}
	}

	//Process Spheres
	assert(sphereOrigins.size() == sphereRadius.size());

	for (unsigned int sphereIndex = 0; sphereIndex < sphereOrigins.size(); sphereIndex++)
	{
		float distance = intersectSphere(inRay.origin, inRay.direction, sphereRadius[sphereIndex], sphereOrigins[sphereIndex]);
		
		if (distance == 0.0f)
			continue;

		if (distance < closest)
		{
			closest = distance;
			closestColour = sphereColours[sphereIndex];
		}
	}

	//Check any object is closer than the default setting, if not return black colour as no intersects occurred
	if (closest == 300000.0f)
	{
		return closestColour;
	}
	else
	{
		float colourScalar = 255.0f - (Utility::normaliseFloat(closest, 180.0f, 0.0f) * 255.0f);
		closestColour = colourScalar * closestColour;
		closestColour.w = 255.0f; //Reset to full on alpha channel
		return closestColour;
	}
}

void MainState::encodePNG(const char* filename, std::vector<unsigned char>& imageData, unsigned width, unsigned height)
{
	//Encode the image
	unsigned error = lodepng::encode(filename, imageData, width, height);

	//if there's an error, display it
	if (error) std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
}

void MainState::createScene1()
{
	//Very small scene quick to render. Clipping objects is intended to show depth.

	//Spheres
	sphereOrigins.push_back(glm::vec4(300.0f, 250.0f, -85.0f, 1));
	sphereOrigins.push_back(glm::vec4(500.0f, 250.0f, -85.0f, 1));

	sphereRadius.push_back(50);
	sphereRadius.push_back(30);

	sphereColours.push_back(glm::vec4(0.0f, 1.0f, 1.0f, 255.0f));
	sphereColours.push_back(glm::vec4(1.0f, 0.0f, 1.0f, 255.0f));


	//Cubes
	Cube cube1(glm::vec4(1.0f, 1.0f, 0.0f, 255.0f));
	cube1.scale(glm::vec3(40.0f));
	cube1.rotate(glm::vec3(0.0f, 0.0f, Utility::convertAngleToRadian(30.0f)));
	cube1.rotate(glm::vec3(0.0f, Utility::convertAngleToRadian(30.0f), 0.0f));
	cube1.translate(glm::vec3(70.0f, 60.0f, -60.0f));
	cubes.push_back(cube1);

	Cube cube2(glm::vec4(0.0f, 1.0f, 1.0f, 255.0f));
	cube2.scale(glm::vec3(30.0f));
	cube2.rotate(glm::vec3(0.0f, 0.0f, Utility::convertAngleToRadian(80.0f)));
	cube2.rotate(glm::vec3(0.0f, Utility::convertAngleToRadian(250.0f), 0.0f));
	cube2.translate(glm::vec3(150.0f, 60.0f, -70.0f));
	cubes.push_back(cube2);

	Cube cube3(glm::vec4(0.0f, 0.0f, 1.0f, 255.0f));
	cube3.scale(glm::vec3(10.0f));
	cube3.rotate(glm::vec3(0.0f, 0.0f, Utility::convertAngleToRadian(160.0f)));
	cube3.rotate(glm::vec3(Utility::convertAngleToRadian(210.0f), 0.0f, 0.0f));
	cube3.translate(glm::vec3(150.0f, 400.0f, -40.0f));
	cubes.push_back(cube3);

	Cube cube4(glm::vec4(1.0f, 0.0f, 0.0f, 255.0f));
	cube4.scale(glm::vec3(50.0f));
	cube4.rotate(glm::vec3(0.0f, 0.0f, Utility::convertAngleToRadian(80.0f)));
	cube4.rotate(glm::vec3(0.0f, Utility::convertAngleToRadian(250.0f), 0.0f));
	cube4.translate(glm::vec3(450.0f, 200.0f, -80.0f));
	cubes.push_back(cube4);
}

void MainState::createScene2()
{
	//Partially Random but manually placed, clipping is on purpose to show depth

	//Spheres
	sphereOrigins.push_back(glm::vec4(100.0f, 150.0f, -85.0f, 1));
	sphereOrigins.push_back(glm::vec4(300.0f, 400.0f, -65.0f, 1));
	sphereOrigins.push_back(glm::vec4(350.0f, 150.0f, -85.0f, 1));
	sphereOrigins.push_back(glm::vec4(200.0f, 250.0f, -85.0f, 1));
	sphereOrigins.push_back(glm::vec4(200.0f, 350.0f, -45.0f, 1));
	sphereOrigins.push_back(glm::vec4(600.0f, 450.0f, -125.0f, 1));
	sphereOrigins.push_back(glm::vec4(20.0f, 450.0f, -64.0f, 1));
	sphereOrigins.push_back(glm::vec4(620.0f, 250.0f, -115.0f, 1));

	sphereRadius.push_back(50);
	sphereRadius.push_back(30);
	sphereRadius.push_back(15);
	sphereRadius.push_back(25);
	sphereRadius.push_back(20);
	sphereRadius.push_back(42);
	sphereRadius.push_back(42);
	sphereRadius.push_back(32);

	for (unsigned int i = 0; i < sphereRadius.size(); i++)
	{
		sphereColours.push_back(glm::vec4(
			Random::getFloat(0.05f, 1.0f),
			Random::getFloat(0.05f, 1.0f),
			Random::getFloat(0.05f, 1.0f),
			255.0f
		));
	}



	//Cubes
	Cube cube1(glm::vec4(1.0f, 1.0f, 0.0f, 255.0f));
	cube1.scale(glm::vec3(40.0f));
	cube1.rotate(glm::vec3(0.0f, 0.0f, Utility::convertAngleToRadian(30.0f)));
	cube1.rotate(glm::vec3(0.0f, Utility::convertAngleToRadian(30.0f), 0.0f));
	cube1.translate(glm::vec3(70.0f, 60.0f, -60.0f));
	cubes.push_back(cube1);

	Cube cube2(glm::vec4(0.0f, 1.0f, 1.0f, 255.0f));
	cube2.scale(glm::vec3(30.0f));
	cube2.rotate(glm::vec3(0.0f, 0.0f, Utility::convertAngleToRadian(80.0f)));
	cube2.rotate(glm::vec3(0.0f, Utility::convertAngleToRadian(250.0f), 0.0f));
	cube2.translate(glm::vec3(150.0f, 60.0f, -70.0f));
	cubes.push_back(cube2);

	Cube cube3(glm::vec4(0.0f, 0.0f, 1.0f, 255.0f));
	cube3.scale(glm::vec3(10.0f));
	cube3.rotate(glm::vec3(0.0f, 0.0f, Utility::convertAngleToRadian(160.0f)));
	cube3.rotate(glm::vec3(Utility::convertAngleToRadian(210.0f), 0.0f, 0.0f));
	cube3.translate(glm::vec3(150.0f, 400.0f, -40.0f));
	cubes.push_back(cube3);

	Cube cube4(glm::vec4(1.0f, 0.0f, 0.0f, 255.0f));
	cube4.scale(glm::vec3(50.0f));
	cube4.rotate(glm::vec3(0.0f, 0.0f, Utility::convertAngleToRadian(80.0f)));
	cube4.rotate(glm::vec3(0.0f, Utility::convertAngleToRadian(250.0f), 0.0f));
	cube4.translate(glm::vec3(450.0f, 200.0f, -80.0f));
	cubes.push_back(cube4);

	Cube cube5(glm::vec4(
		Random::getFloat(0.05f, 1.0f),
		Random::getFloat(0.05f, 1.0f),
		Random::getFloat(0.05f, 1.0f),
		255.0f));
	cube5.scale(glm::vec3(30.0f));
	cube5.rotate(glm::vec3(Utility::convertAngleToRadian(170.0f), 0.0f, 0.0f));
	cube5.rotate(glm::vec3(0.0f, Utility::convertAngleToRadian(150.0f), 0.0f));
	cube5.translate(glm::vec3(450.0f, 400.0f, -60.0f));
	cubes.push_back(cube5);

	Cube cube6(glm::vec4(
		Random::getFloat(0.05f, 1.0f),
		Random::getFloat(0.05f, 1.0f),
		Random::getFloat(0.05f, 1.0f), 
		255.0f));
	cube6.scale(glm::vec3(50.0f));
	cube6.rotate(glm::vec3(0.0f, 0.0f, Utility::convertAngleToRadian(80.0f)));
	cube6.rotate(glm::vec3(Utility::convertAngleToRadian(350.0f), 0.0f, 0.0f));
	cube6.translate(glm::vec3(50.0f, 300.0f, -100.0f));
	cubes.push_back(cube6);

	Cube cube7(glm::vec4(
		Random::getFloat(0.05f, 1.0f),
		Random::getFloat(0.05f, 1.0f),
		Random::getFloat(0.05f, 1.0f),
		255.0f));
	cube7.scale(glm::vec3(70.0f));
	cube7.rotate(glm::vec3(Utility::convertAngleToRadian(160.0f), 0.0f, 0.0f));
	cube7.rotate(glm::vec3(0.0f, Utility::convertAngleToRadian(250.0f), 0.0f));
	cube7.translate(glm::vec3(530.0f, 300.0f, -100.0f));
	cubes.push_back(cube7);

	Cube cube8(glm::vec4(
		Random::getFloat(0.05f, 1.0f),
		Random::getFloat(0.05f, 1.0f),
		Random::getFloat(0.05f, 1.0f),
		255.0f));
	cube8.scale(glm::vec3(25.0f));
	cube8.rotate(glm::vec3(0.0f, 0.0f, Utility::convertAngleToRadian(190.0f)));
	cube8.rotate(glm::vec3(0.0f, Utility::convertAngleToRadian(140.0f), 0.0f));
	cube8.translate(glm::vec3(230.0f, 150.0f, -40.0f));
	cubes.push_back(cube8);

	Cube cube9(glm::vec4(
		Random::getFloat(0.05f, 1.0f),
		Random::getFloat(0.05f, 1.0f),
		Random::getFloat(0.05f, 1.0f),
		255.0f));
	cube9.scale(glm::vec3(50.0f));
	cube9.rotate(glm::vec3(0.0f, Utility::convertAngleToRadian(130.0f), 0.0f));
	cube9.rotate(glm::vec3(Utility::convertAngleToRadian(150.0f), 0.0f, 9.9f));
	cube9.rotate(glm::vec3(0.0f, 0.0f, Utility::convertAngleToRadian(50.0f)));
	cube9.translate(glm::vec3(510.0f, 50.0f, -90.0f));
	cubes.push_back(cube9);

	Cube cube10(glm::vec4(
		Random::getFloat(0.05f, 1.0f),
		Random::getFloat(0.05f, 1.0f),
		Random::getFloat(0.05f, 1.0f),
		255.0f));
	cube10.scale(glm::vec3(24.0f));
	cube10.rotate(glm::vec3(0.0f, 0.0f, Utility::convertAngleToRadian(280.0f)));
	cube10.rotate(glm::vec3(0.0f, Utility::convertAngleToRadian(20.0f), 0.0f));
	cube10.translate(glm::vec3(350.0f, 340.0f, -40.0f));
	cubes.push_back(cube10);
}

void MainState::createScene3()
{
	//Auto Generated (Expect lots of overlapping and some really broken looking shapes)
	for (unsigned int sphereIndex = 0; sphereIndex < 100; sphereIndex++)
	{
		sphereOrigins.push_back(glm::vec4(
			Random::getFloat(0.0f, 630.0f),
			Random::getFloat(0.0f, 470.0f), 
			-Random::getFloat(20.0f, 100.0f), 
			1.0f
		));
		
		sphereRadius.push_back(Random::getFloat(5.0f, 30.0f));

		sphereColours.push_back(glm::vec4(
			Random::getFloat(0.05f, 1.0f),
			Random::getFloat(0.05f, 1.0f),
			Random::getFloat(0.05f, 1.0f),
			255.0f));
	}

	for (unsigned int cubeIndex = 0; cubeIndex < 100; cubeIndex++)
	{
		Cube cube(glm::vec4(
			Random::getFloat(0.05f, 1.0f),
			Random::getFloat(0.05f, 1.0f),
			Random::getFloat(0.05f, 1.0f),
			255.0f));

		cube.scale(glm::vec3(Random::getFloat(5.0f, 30.0f)));

		cube.rotate(glm::vec3(0.0f, 0.0f, Utility::convertAngleToRadian(Random::getFloat(0.0f, 359.0f))));
		cube.rotate(glm::vec3(0.0f, Utility::convertAngleToRadian(Random::getFloat(0.0f, 359.0f)), 0.0f));
		cube.rotate(glm::vec3(Utility::convertAngleToRadian(Random::getFloat(0.0f, 359.0f)), 0.0f, 0.0f));

		cube.translate(glm::vec3(
			Random::getFloat(0.0f, 630.0f),
			Random::getFloat(0.0f, 470.0f),
			-Random::getFloat(30.0f, 100.0f)
		));

		cubes.push_back(cube);
	}
}

void MainState::executeRayTracerOpenCL()
{
	std::cout << "OpenCL Ray Tracer Begin" << std::endl;

	//Break the cubes up into arrays for easy sending to OpenCL
	std::vector<glm::vec4> cubeVertices;
	std::vector<glm::vec4> cubeColours;
	for (auto& cube : cubes)
	{
		auto tempVertices = cube.getTriangles();

		cubeVertices.insert(cubeVertices.end(), tempVertices.begin(), tempVertices.end());

		cubeColours.push_back(cube.getColour());
	}

	int numCubes = cubes.size();
	int numSpheres = sphereOrigins.size();


	//OpenCL Starts
	timer.startCounter();
	cl_int errorCode;

	//Create all buffers for the memory
	cl_mem outputBuffer = clCreateBuffer(
		context,
		CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY,
		(sizeof(int) * 4) * pixelCount,
		NULL, &errorCode);

	if (outputBuffer == NULL)
	{
		std::cout << "OpenCL could not create the output buffer, errorcode: " << getErrorString(errorCode) << std::endl;
	}


	cl_mem sphereOriginsBuffer = clCreateBuffer(
		context,
		CL_MEM_READ_ONLY,
		sizeof(glm::vec4) * sphereOrigins.size(),
		NULL, &errorCode
	);
	if (sphereOriginsBuffer == NULL)
	{
		std::cout << "OpenCL could not create the sphere origins buffer, errorcode: " << getErrorString(errorCode) << std::endl;
	}

	cl_mem sphereRadiusBuffer = clCreateBuffer(
		context,
		CL_MEM_READ_ONLY,
		sizeof(float) * sphereRadius.size(),
		NULL, &errorCode
	);
	if (sphereRadiusBuffer == NULL)
	{
		std::cout << "OpenCL could not create the sphere radius buffer, errorcode: " << getErrorString(errorCode) << std::endl;
	}

	cl_mem sphereColoursBuffer = clCreateBuffer(
		context,
		CL_MEM_READ_ONLY,
		sizeof(glm::vec4) * sphereColours.size(),
		NULL, &errorCode
	);
	if (sphereColoursBuffer == NULL)
	{
		std::cout << "OpenCL could not create the sphere colours buffer, errorcode: " << getErrorString(errorCode) << std::endl;
	}

	cl_mem cubeVerticesBuffer = clCreateBuffer(
		context,
		CL_MEM_READ_ONLY,
		sizeof(glm::vec4) * cubeVertices.size(),
		NULL, &errorCode
	);
	if (cubeVerticesBuffer == NULL)
	{
		std::cout << "OpenCL could not create the sphere colours buffer, errorcode: " << getErrorString(errorCode) << std::endl;
	}

	cl_mem cubeColoursBuffer = clCreateBuffer(
		context,
		CL_MEM_READ_ONLY,
		sizeof(glm::vec4) * cubeColours.size(),
		NULL, &errorCode
	);
	if (cubeColoursBuffer == NULL)
	{
		std::cout << "OpenCL could not create the cube colours buffer, errorcode: " << getErrorString(errorCode) << std::endl;
	}


	cl_mem rayOriginsBuffer = clCreateBuffer(
		context,
		CL_MEM_READ_ONLY,
		sizeof(glm::vec4) * pixelCount,
		NULL, &errorCode
	);
	if (rayOriginsBuffer == NULL)
	{
		std::cout << "OpenCL could not create the ray origins buffer, errorcode: " << getErrorString(errorCode) << std::endl;
	}

	//Setting Kernel Args
	clSetKernelArg(kernel, 0, sizeof(outputBuffer), (void*)&outputBuffer);
	clSetKernelArg(kernel, 1, sizeof(int), (void*)&numSpheres);
	clSetKernelArg(kernel, 2, sizeof(sphereOriginsBuffer), (void*)&sphereOriginsBuffer);
	clSetKernelArg(kernel, 3, sizeof(sphereRadiusBuffer), (void*)&sphereRadiusBuffer);
	clSetKernelArg(kernel, 4, sizeof(sphereColoursBuffer), (void*)&sphereColoursBuffer);
	clSetKernelArg(kernel, 5, sizeof(int), (void*)&numCubes);
	clSetKernelArg(kernel, 6, sizeof(cubeVerticesBuffer), (void*)&cubeVerticesBuffer);
	clSetKernelArg(kernel, 7, sizeof(cubeColoursBuffer), (void*)&cubeColoursBuffer);
	clSetKernelArg(kernel, 8, sizeof(rayOriginsBuffer), (void*)&rayOriginsBuffer);
	clSetKernelArg(kernel, 9, sizeof(glm::vec4), (void*)&rayDir);

	//Passing Data to Buffers
	// SPHERES
	errorCode = clEnqueueWriteBuffer(
		cmdQueue,
		sphereOriginsBuffer,
		CL_TRUE,
		0,
		sizeof(glm::vec4) * sphereOrigins.size(),
		&sphereOrigins[0],
		0,
		NULL,
		NULL
	);
	if (errorCode != CL_SUCCESS)
	{
		std::cout << "OpenCL could not write to the sphereOriginsBuffer, errorcode: " << getErrorString(errorCode) << std::endl;
	}

	errorCode = clEnqueueWriteBuffer(
		cmdQueue,
		sphereRadiusBuffer,
		CL_TRUE,
		0,
		sizeof(float) * sphereRadius.size(),
		&sphereRadius[0],
		0,
		NULL,
		NULL
	);
	if (errorCode != CL_SUCCESS)
	{
		std::cout << "OpenCL could not write to the sphereRadiusBuffer, errorcode: " << getErrorString(errorCode) << std::endl;
	}

	errorCode = clEnqueueWriteBuffer(
		cmdQueue,
		sphereColoursBuffer,
		CL_TRUE,
		0,
		sizeof(glm::vec4) * sphereColours.size(),
		&sphereColours[0],
		0,
		NULL,
		NULL
	);
	if (errorCode != CL_SUCCESS)
	{
		std::cout << "OpenCL could not write to the sphereColoursBuffer, errorcode: " << getErrorString(errorCode) << std::endl;
	}

	//CUBES
	errorCode = clEnqueueWriteBuffer(
		cmdQueue,
		cubeVerticesBuffer,
		CL_TRUE,
		0,
		sizeof(glm::vec4) * cubeVertices.size(),
		&cubeVertices[0],
		0,
		NULL,
		NULL
	);
	if (errorCode != CL_SUCCESS)
	{
		std::cout << "OpenCL could not write to the cubeVerticesBuffer, errorcode: " << getErrorString(errorCode) << std::endl;
	}

	errorCode = clEnqueueWriteBuffer(
		cmdQueue,
		cubeColoursBuffer,
		CL_TRUE,
		0,
		sizeof(glm::vec4) * cubeColours.size(),
		&cubeColours[0],
		0,
		NULL,
		NULL
	);
	if (errorCode != CL_SUCCESS)
	{
		std::cout << "OpenCL could not write to the cubeColoursBuffer, errorcode: " << getErrorString(errorCode) << std::endl;
	}

	//RAYS
	errorCode = clEnqueueWriteBuffer(
		cmdQueue,
		rayOriginsBuffer,
		CL_TRUE,
		0,
		sizeof(glm::vec4) * pixelCount,
		&rayOrigins[0],
		0,
		NULL,
		NULL
	);
	if (errorCode != CL_SUCCESS)
	{
		std::cout << "OpenCL could not write to the rayOriginsBuffer, errorcode: " << getErrorString(errorCode) << std::endl;
	}

	//Start the Parallel processing
	size_t globalWorkSize = pixelCount;
	errorCode = clEnqueueNDRangeKernel(
		cmdQueue,
		kernel,
		1,
		NULL,
		&globalWorkSize,
		NULL,
		0,
		NULL,
		NULL
	);
	if (errorCode != CL_SUCCESS)
	{
		std::cout << "OpenCL could not enqueue a execute kernel command, errorcode: " << getErrorString(errorCode) << std::endl;
	}

	//Retrieve results of the processing (Will block execution until returned)
	int *ptr = (int*)clEnqueueMapBuffer(
		cmdQueue,
		outputBuffer,
		CL_TRUE,
		CL_MAP_READ,
		0,
		(sizeof(int) * 4) * pixelCount,
		0,
		NULL,
		NULL,
		&errorCode);
	if (errorCode != CL_SUCCESS)
	{
		std::cout << "OpenCL could not enqueue a execute map buffer command, errorcode: " << getErrorString(errorCode) << std::endl;
	}


	//Calculate Timer
	timeTaken = timer.stopCounter();
	
	float timeTakenMilliSeconds = (timeTaken / 1000.0f); //Convert to MilliSeconds

	//Update UI
	delete timeTakenUI;
	std::string timeTakenStr = "Time: " + Utility::floatToString(timeTakenMilliSeconds, 4) + "ms";
	timeTakenUI = new Texture(TTF_RenderText_Blended(font, timeTakenStr.c_str(), textColour), platform->getRenderer());

	std::cout << "Time Taken: " << timeTaken << " microseconds" << std::endl;
	std::cout << "OpenCL Ray Trace Finished (Timer Stopped), Converting data to pixels" << std::endl << std::endl;

	//Convert array to vector for simplicity;
	pixels.assign(ptr, ptr + (pixelCount * 4));
	
	//Clear raw pixel ptr 
	errorCode = clEnqueueUnmapMemObject(
		cmdQueue,
		outputBuffer,
		ptr,
		0,
		NULL,
		NULL
	);
	if (errorCode != CL_SUCCESS)
	{
		std::cout << "OpenCL could not enqueue a execute um-map buffer command, errorcode: " << getErrorString(errorCode) << std::endl;
	}


	//Clear OpenCL Memory for this scene
	clFlush(cmdQueue);
	clFinish(cmdQueue);
	clReleaseMemObject(outputBuffer);
	clReleaseMemObject(sphereColoursBuffer);
	clReleaseMemObject(sphereOriginsBuffer);
	clReleaseMemObject(sphereRadiusBuffer);
	clReleaseMemObject(cubeColoursBuffer);
	clReleaseMemObject(cubeVerticesBuffer);
	clReleaseMemObject(rayOriginsBuffer);
}

void MainState::executeRayTracerCPU()
{
	std::cout << "CPU Ray Tracer Begin" << std::endl;
	timer.startCounter();

	for (auto rayOrigin : rayOrigins)
	{
		Ray ray;
		ray.origin = rayOrigin;
		ray.direction = rayDir;

		glm::vec4 resultColour = glm::vec4(0.0f, 0.0f, 0.0f, 255.0f);

		resultColour = collide(ray, cubes, sphereRadius, sphereOrigins, sphereColours);


		pixels.push_back((int)resultColour.r);
		pixels.push_back((int)resultColour.g);
		pixels.push_back((int)resultColour.b);
		pixels.push_back((int)resultColour.a);
	}


	//Calculate Timer
	timeTaken = timer.stopCounter();

	float timeTakenMilliSeconds = (timeTaken / 1000.0f); //Convert to MilliSeconds

	//Update UI
	delete timeTakenUI;
	std::string timeTakenStr = "Time: " + Utility::floatToString(timeTakenMilliSeconds, 4) + "ms";
	timeTakenUI = new Texture(TTF_RenderText_Blended(font, timeTakenStr.c_str(), textColour), platform->getRenderer());

	std::cout << "Time Taken: " << timeTaken << " microseconds" << std::endl;
	std::cout << "CPU Ray Trace Finished (Timer Stopped), Converting data to pixels" << std::endl << std::endl;
	//encodePNG("ray.png", pixels, platform->getWindowSize().x, platform->getWindowSize().y);
}

void MainState::generateImageFromPixels()
{
	std::vector<SDL_Colour> pixelColours;
	pixelColours.reserve(pixels.size() / 4);

	SDL_Surface* surface;
	Uint32 rmask, gmask, bmask, amask;

	/* SDL interprets each pixel as a 32-bit number, so our masks must depend
	on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif

	surface = SDL_CreateRGBSurface(0, (int)platform->getWindowSize().x, (int)platform->getWindowSize().y, 32,
		rmask, gmask, bmask, amask);

	if (surface == NULL) {
		fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0));


	std::vector<SDL_Rect> rects;

	for (unsigned int y = 0; y < platform->getWindowSize().y; y++)
	{
		for (unsigned int x = 0; x < platform->getWindowSize().x; x++)
		{
			SDL_Rect rect;

			rect.h = rect.w = 1;
			rect.x = x;
			rect.y = y;

			rects.push_back(rect);
		}
	}

	for (unsigned int pixelIndex = 0; pixelIndex < pixels.size(); pixelIndex += 4)
	{
		SDL_Colour colour;
		colour.r = (uint8_t)pixels[pixelIndex];
		colour.g = (uint8_t)pixels[pixelIndex + 1];
		colour.b = (uint8_t)pixels[pixelIndex + 2];
		colour.a = (uint8_t)pixels[pixelIndex + 3];

		pixelColours.push_back(colour);

		SDL_FillRect(surface, &rects[pixelIndex / 4], SDL_MapRGB(surface->format,
			(uint8_t)pixels[pixelIndex],
			(uint8_t)pixels[pixelIndex + 1],
			(uint8_t)pixels[pixelIndex + 2])
		);
	}

	//SDL_SetRenderDrawColor(platform->getRenderer(), 0, 0, 0, 255);

	pixels.clear();

	image = new Texture(surface, platform->getRenderer());
}

std::string MainState::loadComputeShaderFromFile(std::string path)
{
	std::string shader;
	std::ifstream file(path, std::ios::in);

	if (!file.is_open())
	{
		std::cout << "Compute Shader couldn't be opened: " << path << std::endl;
		return "";
	}

	std::string line;
	while (!file.eof())
	{
		//file.getline(line,(std::streamsize) 2000);
		std::getline(file, line);
		line.append("\n");
		shader.append(line);

		//char line[512];
		//file.getline(line, 511);
		//shader += line;
	}
	file.close();

	return shader;
}

std::string MainState::clDeviceTypeToString(cl_device_type type)
{
	std::string result;

	if (type & CL_DEVICE_TYPE_CPU)
		result += "CPU ";

	if (type & CL_DEVICE_TYPE_GPU)
		result += "GPU ";

	if (type & CL_DEVICE_TYPE_ACCELERATOR)
		result += "Accelerator ";

	if (type & CL_DEVICE_TYPE_DEFAULT)
		result += "Default ";

	if (type & CL_DEVICE_TYPE_CUSTOM)
		result += "Custom ";


	if (result.empty())
		result = "Unknown";

	return result;
}

const char * MainState::getErrorString(cl_int error)
{
	//Copied from stack overflow as I am not writing these out manually.
	//Ref: http://stackoverflow.com/a/24336429/3262098
	switch (error) 
	{
		// run-time and JIT compiler errors
		case 0: return "CL_SUCCESS";
		case -1: return "CL_DEVICE_NOT_FOUND";
		case -2: return "CL_DEVICE_NOT_AVAILABLE";
		case -3: return "CL_COMPILER_NOT_AVAILABLE";
		case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
		case -5: return "CL_OUT_OF_RESOURCES";
		case -6: return "CL_OUT_OF_HOST_MEMORY";
		case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
		case -8: return "CL_MEM_COPY_OVERLAP";
		case -9: return "CL_IMAGE_FORMAT_MISMATCH";
		case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
		case -11: return "CL_BUILD_PROGRAM_FAILURE";
		case -12: return "CL_MAP_FAILURE";
		case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
		case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
		case -15: return "CL_COMPILE_PROGRAM_FAILURE";
		case -16: return "CL_LINKER_NOT_AVAILABLE";
		case -17: return "CL_LINK_PROGRAM_FAILURE";
		case -18: return "CL_DEVICE_PARTITION_FAILED";
		case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

			// compile-time errors
		case -30: return "CL_INVALID_VALUE";
		case -31: return "CL_INVALID_DEVICE_TYPE";
		case -32: return "CL_INVALID_PLATFORM";
		case -33: return "CL_INVALID_DEVICE";
		case -34: return "CL_INVALID_CONTEXT";
		case -35: return "CL_INVALID_QUEUE_PROPERTIES";
		case -36: return "CL_INVALID_COMMAND_QUEUE";
		case -37: return "CL_INVALID_HOST_PTR";
		case -38: return "CL_INVALID_MEM_OBJECT";
		case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
		case -40: return "CL_INVALID_IMAGE_SIZE";
		case -41: return "CL_INVALID_SAMPLER";
		case -42: return "CL_INVALID_BINARY";
		case -43: return "CL_INVALID_BUILD_OPTIONS";
		case -44: return "CL_INVALID_PROGRAM";
		case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
		case -46: return "CL_INVALID_KERNEL_NAME";
		case -47: return "CL_INVALID_KERNEL_DEFINITION";
		case -48: return "CL_INVALID_KERNEL";
		case -49: return "CL_INVALID_ARG_INDEX";
		case -50: return "CL_INVALID_ARG_VALUE";
		case -51: return "CL_INVALID_ARG_SIZE";
		case -52: return "CL_INVALID_KERNEL_ARGS";
		case -53: return "CL_INVALID_WORK_DIMENSION";
		case -54: return "CL_INVALID_WORK_GROUP_SIZE";
		case -55: return "CL_INVALID_WORK_ITEM_SIZE";
		case -56: return "CL_INVALID_GLOBAL_OFFSET";
		case -57: return "CL_INVALID_EVENT_WAIT_LIST";
		case -58: return "CL_INVALID_EVENT";
		case -59: return "CL_INVALID_OPERATION";
		case -60: return "CL_INVALID_GL_OBJECT";
		case -61: return "CL_INVALID_BUFFER_SIZE";
		case -62: return "CL_INVALID_MIP_LEVEL";
		case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
		case -64: return "CL_INVALID_PROPERTY";
		case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
		case -66: return "CL_INVALID_COMPILER_OPTIONS";
		case -67: return "CL_INVALID_LINKER_OPTIONS";
		case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

			// extension errors
		case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
		case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
		case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
		case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
		case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
		case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
		default: return "Unknown OpenCL error";
	}
}

void MainState::openCLInit()
{
	//OpenCL Init
	bool clpresent = 0 == clewInit();
	if (!clpresent)
	{
		std::cout << "Open CL Library not found, Exiting" << std::endl;
	}

	cl_int error = 0;
	cl_platform_id platform_ids[4];
	cl_uint num_platforms = 0;
	error = clGetPlatformIDs(4, platform_ids, &num_platforms);
	if (error != CL_SUCCESS)
	{
		std::cout << "something went wrong, errorcode " << error << std::endl;
	}
	std::cout << "Number of OpenCL Platforms: " << num_platforms << std::endl;


	cl_device_id deviceID = nullptr;
	bool gpuFound = false;

	for (unsigned int platformIndex = 0; platformIndex < num_platforms; platformIndex++)
	{
		std::cout << "Platform " << platformIndex << ": " << std::endl;
		char words[1000];
		clGetPlatformInfo(platform_ids[platformIndex], CL_PLATFORM_NAME, 1000, words, NULL);
		std::cout << " - Name: " << words << std::endl;
		clGetPlatformInfo(platform_ids[platformIndex], CL_PLATFORM_VERSION, 1000, words, NULL);
		std::cout << " - Version: " << words << std::endl;
		clGetPlatformInfo(platform_ids[platformIndex], CL_PLATFORM_PROFILE, 1000, words, NULL);
		std::cout << " - Profile: " << words << std::endl;
		clGetPlatformInfo(platform_ids[platformIndex], CL_PLATFORM_VENDOR, 1000, words, NULL);
		std::cout << " - Vendor: " << words << std::endl;
		clGetPlatformInfo(platform_ids[platformIndex], CL_PLATFORM_EXTENSIONS, 1000, words, NULL);
		std::cout << " - Extensions: " << words << std::endl << std::endl;

		cl_device_id devices[4];
		cl_uint numDevices = 0;
		error = clGetDeviceIDs(platform_ids[platformIndex], CL_DEVICE_TYPE_ALL, 4, devices, &numDevices);
		if (error != CL_SUCCESS)
		{
			std::cout << "something went wrong, errorcode " << error << std::endl;
		}

		std::cout << " - Number of OpenCL Devices: " << numDevices << std::endl;
		for (unsigned int deviceIndex = 0; deviceIndex < numDevices; deviceIndex++)
		{
			cl_bool boolean = false;
			cl_uint uInt = 0;

			std::cout << "-- Device Index: " << deviceIndex << std::endl;
			clGetDeviceInfo(devices[deviceIndex], CL_DEVICE_NAME, sizeof(words), words, NULL);
			std::cout << " -- Name: " << words << std::endl;

			cl_device_type deviceType;
			clGetDeviceInfo(devices[deviceIndex], CL_DEVICE_TYPE, sizeof(cl_device_type), &deviceType, NULL);
			std::cout << " -- Type: " << clDeviceTypeToString(deviceType) << std::endl;

			//If the device is a GPU and we haven't already found one, save it
			if (deviceType & CL_DEVICE_TYPE_GPU && !gpuFound)
			{
				deviceID = devices[deviceIndex];
				gpuFound = true;
			}


			clGetDeviceInfo(devices[deviceIndex], CL_DEVICE_AVAILABLE, sizeof(cl_bool), &boolean, NULL);
			std::cout << " -- Is Available: " << boolean << std::endl;

			clGetDeviceInfo(devices[deviceIndex], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_uint), &uInt, NULL);
			std::cout << " -- Max Clock Frequency: " << uInt << "MHz" << std::endl;

			clGetDeviceInfo(devices[deviceIndex], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &uInt, NULL);
			std::cout << " -- Max Compute Units: " << uInt << std::endl;

			clGetDeviceInfo(devices[deviceIndex], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_uint), &uInt, NULL);
			std::cout << " -- Max Clock Frequency: " << uInt << "MHz" << std::endl << std::endl;
		}

		//If no GPU is found save the first available device as a backup
		if (platformIndex == 0 && !gpuFound)
		{
			deviceID = devices[0];
		}
	}

	if (!gpuFound)
	{
		std::cout << "No GPU Found, OpenCL will attempt fallback to device 0" << std::endl;
	}
		

	context = clCreateContext(NULL, 1, &deviceID, NULL, NULL, &error);

	if (context == NULL)
	{
		std::cout << "OpenCL could not create a context, errorcode: " << error << std::endl;
		//return -1;
	}

	cmdQueue = clCreateCommandQueue(context, deviceID, NULL, &error);
	if (cmdQueue == NULL)
	{
		std::cout << "OpenCL could not create a command queue, errorcode: " << error << std::endl;
		//return -1;
	}

	std::string shaderRaw = loadComputeShaderFromFile("resources/shaders/rayTracer.cl");
	const char* shader = shaderRaw.c_str();

	//std::cout << shaderRaw << std::endl;

	program = clCreateProgramWithSource(context, 1, &shader, NULL, &error);
	if (program == NULL)
	{
		std::cout << "OpenCL could not create a program, errorcode: " << error << std::endl;
		//return -1;
	}

	error = clBuildProgram(program, 1, &deviceID, NULL, NULL, NULL);
	if (error != CL_SUCCESS)
	{
		std::cout << "OpenCL could not build program, errorcode: " << error << std::endl;

		char log[128000]; //128KB log, should be more than enough
		if (clGetProgramBuildInfo(program, deviceID, CL_PROGRAM_BUILD_LOG, 128000, &log, NULL) != CL_SUCCESS)
		{
			std::cout << "OpenCL could not get build error log" << std::endl;
			//return -1;
		}
		else
		{
			std::cout << "Build Log:" << std::endl << log << std::endl;
		}
		//return -1;
	}

	kernel = clCreateKernel(program, "rayTracer", &error);
	if (kernel == NULL)
	{
		std::cout << "OpenCL could not create a kernel, errorcode: " << error << std::endl;
		//return -1;
	}
}
