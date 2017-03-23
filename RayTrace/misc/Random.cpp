#include "Random.h"

#include <time.h>
#include <stdlib.h>

#include "Log.h"

bool Random::initialized = false;

void Random::init(unsigned int seed)
{
	if (seed == 0)
		seed = (unsigned int)time(NULL);

	srand(seed);
	initialized = true;
}

int Random::getInt(int min, int max)
{
	if (!initialized)
	{
		Log::logW("Random function called without calling Random::init(). Number returned is non-random.");
	}

	return rand() % (max - min + 1) + min;
}

float Random::getFloat(float min, float max)
{
	if (!initialized)
	{
		Log::logW("Random function called without calling Random::init(). Number returned is non-random.");
	}

	// Not written by me it's from: http://stackoverflow.com/a/5289624
	// Could have used C++11 for random floats, but I think this is adequate.
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = max - min;
	float r = random * diff;
	return min + r;
}
