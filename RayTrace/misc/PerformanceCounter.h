#pragma once

#include <cstdint>

//If Visual Studio
#ifdef _MSC_VER
#include <Windows.h>
#endif // _MSC_VER

/**
@brief	A class that provides high accuracy timers for profiling a program 
	I've decided to use Window's peformance counters as C++ chrono is not
	fully working in < VS 2015. and internally chrono just calls these functions anyway.

@todo Implement other platforms using std::chrono
*/
class PerformanceCounter
{
public:

	/** @brief	Initializes the static elements of this class */
	static void initSubsystem();

	/** @brief	Default constructor. */
	PerformanceCounter();

	/** @brief	Starts the counter. */
	void startCounter();

	/**
	 @brief	Stops the counter and returns the elapsed time in microseconds.
	
	 @return	An uint64_t containing elapsed time in microseconds.
	 */
	uint64_t stopCounter();

private:

	/** @brief	Is the counter running. */
	bool running;

#ifdef _MSC_VER

	/** @brief	The time the counter started. */
	LARGE_INTEGER startTime;

	/** @brief	The time the count ended. */
	LARGE_INTEGER endTime;

	/** 
	@brief	The frequency of the counter.
	Consistent value after system boot
	 */
	static LARGE_INTEGER frequency;
#endif // _MSC_VER
};