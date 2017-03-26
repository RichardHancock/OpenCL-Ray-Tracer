#include "PerformanceCounter.h"

#include "Log.h"

#ifdef _MSC_VER
LARGE_INTEGER PerformanceCounter::frequency;
#endif // _MSC_VER

void PerformanceCounter::initSubsystem()
{
#ifdef _MSC_VER
	if (QueryPerformanceFrequency(&frequency) == 0)
	{
		//Not huge problem just means that the performance counter module will not work.
		Log::logW("Could not initalize Performance Counter Subsystem: " + GetLastError());
		frequency.QuadPart = 0;
	}
#endif // _MSC_VER
	
	Log::logI("Performance Counter Subsystem Initialized");
}


PerformanceCounter::PerformanceCounter()
	: running(false)
{

}

void PerformanceCounter::startCounter()
{
	if (!running)
	{
		//This only fails if the function is unavailable on this system so I dont call it on stop
		// as they are the same function and stop can't be called until start.
		if (QueryPerformanceCounter(&startTime) == 0)
		{
			Log::logW("Performance Counter could not start: " + GetLastError());
			return;
		}
		running = true;
	}
}

uint64_t PerformanceCounter::stopCounter()
{
	if (running)
	{
		QueryPerformanceCounter(&endTime);
		running = false;

		LARGE_INTEGER timeElapsed;
		timeElapsed.QuadPart = endTime.QuadPart - startTime.QuadPart;

		//Convert to microseconds (This is done first to prevent loss of precision)
		timeElapsed.QuadPart *= 1000000;
		//Convert to ticks per second
		timeElapsed.QuadPart /= frequency.QuadPart;

		return timeElapsed.QuadPart;
	}

	return 0;
}
