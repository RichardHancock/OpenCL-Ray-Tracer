#include "MainState.h"

#include <fstream>
#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp"
#include "../glm/gtc/type_ptr.hpp"

#include "../lodepng.h"
#include "../Sphere.h"
#include "../misc/Utility.h"


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
	result = nullptr;

	stateName = "Main State";

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


	cl_device_id deviceID;

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

			clGetDeviceInfo(devices[deviceIndex], CL_DEVICE_AVAILABLE, sizeof(cl_bool), &boolean, NULL);
			std::cout << " -- Is Available: " << boolean << std::endl;

			clGetDeviceInfo(devices[deviceIndex], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_uint), &uInt, NULL);
			std::cout << " -- Max Clock Frequency: " << uInt << "MHz" << std::endl;

			clGetDeviceInfo(devices[deviceIndex], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &uInt, NULL);
			std::cout << " -- Max Compute Units: " << uInt << std::endl;

			clGetDeviceInfo(devices[deviceIndex], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_uint), &uInt, NULL);
			std::cout << " -- Max Clock Frequency: " << uInt << "MHz" << std::endl << std::endl;
		}

		if (platformIndex == 0)
			deviceID = devices[0];

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

		char log[2048];
		if (clGetProgramBuildInfo(program, deviceID, CL_PROGRAM_BUILD_LOG, 2048, &log, NULL) != CL_SUCCESS)
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


	//Send in Array of ray origins and Di331
	//cl_uchar
	//size_t outputSize = (sizeof(cl_uchar) * 4) * (platform->getWindowSize().x * platform->getWindowSize().y);

	//cl_mem outputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, outputSize, NULL, &error);
	/*if (outputBuffer == NULL)
	{
		std::cout << "OpenCL could not create the out buffer, errorcode: " << error << std::endl;
	}*/

	//cl_mem 


	//Create Tri
	tri.a = glm::vec3(150, 0, 0);
	tri.b = glm::vec3(0, 300, 0);
	tri.c = glm::vec3(300, 300, 0);

	//Dimensions * 4 Bytes (RGBA)
	pixels.reserve((platform->getWindowSize().x * platform->getWindowSize().y) * 4);


}

MainState::~MainState()
{
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
	if (result == nullptr)
	{
		//glm::mat4 proj = glm::ortho(0.0f, 300.0f, 300.0f, 0.0f, 0.0f, 1000.0f);
		glm::mat4 proj = glm::perspective(45.0f, 16.0f / 9.0f, 0.0f, 100.0f);
		Sphere s;
		s.radius = 50;
		s.origin = glm::vec4(50.0f, 50.0f, -50.0f, 1);

		glm::vec4 rayDir = proj * glm::vec4(0, 0, 1, 1);

		int pixelCount = (platform->getWindowSize().x * platform->getWindowSize().y);

		std::vector<glm::vec4> rayOrigins;
		rayOrigins.reserve(pixelCount * 4);

		for (unsigned int y = 0; y < platform->getWindowSize().y; y++)
		{
			for (unsigned int x = 0; x < platform->getWindowSize().x; x++)
			{
				rayOrigins.push_back(glm::vec4(x, y, 0.0f, 1.0f));
			}
		}


		std::vector<glm::vec4> sphereOrigins;
		sphereOrigins.reserve(1);
		sphereOrigins.push_back(glm::vec4(50.0f, 50.0f, -50.0f, 1));
		
		std::vector<float> sphereRadius;
		sphereRadius.reserve(1);
		sphereRadius.push_back(50);


		//OpenCL
		cl_int errorCode;

		cl_mem outputBuffer = clCreateBuffer(
			context,
			CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY,
			(sizeof(int) * 4) * pixelCount,
			NULL, &errorCode);

		if (outputBuffer == NULL)
		{
			std::cout << "OpenCL could not create the output buffer, errorcode: " << errorCode << std::endl;
		}


		cl_mem sphereOriginsBuffer = clCreateBuffer(
			context,
			CL_MEM_READ_ONLY,
			sizeof(glm::vec4) * sphereOrigins.size(),
			NULL, &errorCode
		);
		if (sphereOriginsBuffer == NULL)
		{
			std::cout << "OpenCL could not create the sphere origins buffer, errorcode: " << errorCode << std::endl;
		}
		
		cl_mem sphereRadiusBuffer = clCreateBuffer(
			context,
			CL_MEM_READ_ONLY,
			sizeof(float) * sphereRadius.size(),
			NULL, &errorCode
		);
		if (sphereRadiusBuffer == NULL)
		{
			std::cout << "OpenCL could not create the sphere radius buffer, errorcode: " << errorCode << std::endl;
		}


		cl_mem rayOriginsBuffer = clCreateBuffer(
			context,
			CL_MEM_READ_ONLY,
			sizeof(glm::vec4) * pixelCount,
			NULL, &errorCode
		);
		if (rayOriginsBuffer == NULL)
		{
			std::cout << "OpenCL could not create the ray origins buffer, errorcode: " << errorCode << std::endl;
		}
		
		//Setting Kernel Args
		clSetKernelArg(kernel, 0, sizeof(outputBuffer), (void*) &outputBuffer);
		clSetKernelArg(kernel, 1, sizeof(sphereOriginsBuffer), (void*) &sphereOriginsBuffer);
		clSetKernelArg(kernel, 2, sizeof(sphereRadiusBuffer), (void*) &sphereRadiusBuffer);
		clSetKernelArg(kernel, 3, sizeof(rayOriginsBuffer), (void*) &rayOriginsBuffer);
		clSetKernelArg(kernel, 4, sizeof(glm::vec4), (void*) &rayDir);

		//Passing Data to Buffers
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
			std::cout << "OpenCL could not write to the sphereOriginsBuffer, errorcode: " << errorCode << std::endl;
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
			std::cout << "OpenCL could not write to the sphereRadiusBuffer, errorcode: " << errorCode << std::endl;
		}
		
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
			std::cout << "OpenCL could not write to the rayOriginsBuffer, errorcode: " << errorCode << std::endl;
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
			std::cout << "OpenCL could not enqueue a execute kernel command, errorcode: " << errorCode << std::endl;
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
			std::cout << "OpenCL could not enqueue a execute kernel command, errorcode: " << errorCode << std::endl;
		}

		pixels.assign(ptr, ptr + (pixelCount * 4));
		
		std::cout << "";

		/*
		for (unsigned int y = 0; y < platform->getWindowSize().y; y++)
		{
			for (unsigned int x = 0; x < platform->getWindowSize().x; x++)
			{
				int result = 1;

				Ray ray;
				ray.origin = glm::vec4(x, y, 0, 1);
				ray.direction = proj * glm::vec4(0, 0, 1, 1);
				//ray.direction = glm::vec4(0, 0, 1, 1);

				//ray.direction = glm::vec4(glm::unProject(glm::vec3(ray.direction), glm::mat4(1.0f), proj, glm::vec4(0.0f, 0.0f, 300.0f, 300.0f)), 1.0f);

				//std::cout << "Ray:" << ray.direction.x << ", " << ray.direction.y << ", " << ray.direction.z << ", " << ray.direction.w << std::endl;

				do {
					glm::vec4 L = s.origin - ray.origin;
					float tca = glm::dot(L, ray.direction);

					if (tca < 0)
					{
						result = 0;
						break;
					}

					float distanceSquared = glm::dot(L, L) - tca * tca;
					if (distanceSquared > s.radius * s.radius)
					{
						result = 0;
						break;
					}

					float thc = sqrt((s.radius * s.radius) - distanceSquared);
					float t0 = tca - thc;
					float t1 = tca + thc;

					float distance = glm::distance(t0, t1);

					

					distance = Utility::normaliseFloat(distance, s.radius + s.radius, 0.0f) * 255.0f;

					result = (int)(distance);

				} while (0);
				//int result = collide(ray);

				//if Result equal 1 set red channel to max
				//pixels.push_back(result == 1 ? 255 : 0);

				if (result < 0)
				{
					result = 0;
				}

				pixels.push_back((unsigned char)result);
				pixels.push_back(0);
				pixels.push_back(0);
				pixels.push_back(255);

				//Also print to console
				//std::cout << result;
			}
			//New line for each row
			//std::cout << std::endl;
		}*/


		//encodePNG("ray.png", pixels, platform->getWindowSize().x, platform->getWindowSize().y);
	}
	
}

void MainState::render()
{
	if (result == nullptr)
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

		surface = SDL_CreateRGBSurface(0, platform->getWindowSize().x, platform->getWindowSize().y, 32,
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
				/*
				SDL_SetRenderDrawColor(platform->getRenderer(),
					pixelColours[x * (y + 1)].r,
					pixelColours[x * (y + 1)].g,
					pixelColours[x * (y + 1)].b,
					pixelColours[x * (y + 1)].a
				);*/

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
			colour.r = pixels[pixelIndex];
			colour.g = pixels[pixelIndex + 1];
			colour.b = pixels[pixelIndex + 2];
			colour.a = pixels[pixelIndex + 3];

			pixelColours.push_back(colour);

			SDL_FillRect(surface, &rects[pixelIndex / 4], SDL_MapRGB(surface->format,
				pixels[pixelIndex],
				pixels[pixelIndex + 1],
				pixels[pixelIndex + 2])
			);
		}

		//SDL_SetRenderDrawColor(platform->getRenderer(), 0, 0, 0, 255);

		pixels.clear();

		result = new Texture(surface, platform->getRenderer());
	}

	if (result != nullptr)
		result->draw(Vec2(0.0f));
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

//Quick function to convert params to be suitable for the intersect code
int MainState::collide(Ray& ray)
{
	double rayOrigin[3]{ ray.origin.x, ray.origin.y, ray.origin.z };
	double rayDir[3]{ ray.direction.x, ray.direction.y, ray.direction.z };

	double tri0[3]{ tri.a.x, tri.a.y, tri.a.z };
	double tri1[3]{ tri.b.x, tri.b.y, tri.b.z };
	double tri2[3]{ tri.c.x, tri.c.y, tri.c.z };

	double t = 0;
	double u = 0;
	double v = 0;

	return intersectTri(rayOrigin, rayDir, tri0, tri1, tri2, &t, &u, &v);
}

void MainState::encodePNG(const char* filename, std::vector<unsigned char>& image, unsigned width, unsigned height)
{
	//Encode the image
	unsigned error = lodepng::encode(filename, image, width, height);

	//if there's an error, display it
	if (error) std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
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