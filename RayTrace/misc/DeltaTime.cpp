#include "DeltaTime.h"

#include <SDL.h>

#include "Log.h"

float DeltaTime::deltaTime = 0.0f;
unsigned int DeltaTime::lastTime = 0;

float DeltaTime::getDT()
{
	return deltaTime;
}

void DeltaTime::init()
{
	lastTime = SDL_GetTicks();
	Log::logI("DeltaTime SubSystem Initialised");
}

void DeltaTime::update()
{
	unsigned int current = SDL_GetTicks();
	deltaTime = (float)(current - lastTime) / 1000.0f;
	lastTime = current;
}
