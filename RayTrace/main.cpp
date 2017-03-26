#include <SDL.h>

#include "Platform.h"
#include "misc/Utility.h"
#include "misc/Random.h"
#include "misc/Log.h"
#include "input/InputManager.h"
#include "misc/DeltaTime.h"
#include "states/StateManager.h"
#include "states/MainState.h"
#include "misc/PerformanceCounter.h"

#ifdef _WIN32
#include <windows.h>

//This forces NVIDIA hybrid GPU's (Intel and Nvidia integrated) to use the high performance NVidia chip rather than the Intel.
//This was recommended by NVidia's policies: http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
extern "C"
{
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
#endif

int main(int, char**)
{
	//Get Settings Dir
	std::string settingsPath = SDL_GetPrefPath("RH", "GCP A2");
	settingsPath += "settings.xml";

	//Initialise platform class and all dependencies
	Platform* platform = new Platform(settingsPath);
	platform->loadSettingsFromFile();

	if (!platform->initSDL(false))
	{
		Log::logE("SDL Failed to initialize");
		exit(1);
	}


	Random::init();
	DeltaTime::init();
	PerformanceCounter::initSubsystem();


	SDL_Renderer* renderer = platform->getRenderer();
	SDL_RenderSetLogicalSize(renderer, 640, 480);

	StateManager* stateManager = new StateManager((int)platform->getWindowSize().x, (int)platform->getWindowSize().y);

	stateManager->addState(new MainState(stateManager, platform));

	bool quit = false;

	while (!quit)
	{
		//Event/Input Handling
		quit = stateManager->eventHandler();

		//Update
		DeltaTime::update();

		Utility::Timer::update();

		stateManager->update();

		//Render
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(renderer);

		stateManager->render();

		SDL_RenderPresent(renderer);

		InputManager::update();

		if (DeltaTime::getDT() < (1.0f / 60.0f))
		{
			SDL_Delay((unsigned int)(((1.0f / 60.0f) - DeltaTime::getDT())*1000.0f));
		}
	}

	InputManager::cleanup();
	delete platform;
	SDL_Quit();

	exit(0);
}
