#pragma once

class Random
{
public:

	/**
	@brief Initialize the Random Number functions

	Call this before any random functions, else random will be the same
	sequence of numbers for each program execution.

	@param seed - Seed for the random number generator. Leave blank to use time(NULL) as seed.
	*/
	static void init(unsigned int seed = 0);

	/**
	@brief Generate a random integer number between min and max.

	@param min - Minimum Number for the range.
	@param max - Maximum Number for the range.

	@return int - Random int between min and max.
	 */
	static int getInt(int min, int max);

	/**
	@brief Generate a random float number between min and max.

	@param min - Minimum Number for the range.
	@param max - Maximum Number for the range.

	@return float - Random float between min and max.
	 */
	static float getFloat(float min, float max);

private:

	static bool initialized;
};
