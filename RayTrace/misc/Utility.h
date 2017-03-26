#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <string>
#include <SDL.h>
#include <unordered_map>

#include "Vec2.h"
#include "Vec3.h"

/**
@brief An assortment of useful functions (random numbers, logging) that don't fit elsewhere.
*/
namespace Utility
{
	/** @brief PI constant. */
	const float PI = 3.1415926535f;

	/** @brief Half PI constant. */
	const float HALF_PI = PI * 0.5f;

	/** @brief Quarter PI constant. */
	const float QUARTER_PI = PI * 0.25f;


	/**
	 @brief Converts int to a string
	 Created because some compilers still don't support std::to_string()

	 @param num - Number to convert.

	 @return std::string - Converted string.
	 */
	std::string intToString(int num);

	/**
	@brief Converts int to a string
	Created because some compilers still don't support std::to_string()
	@param num - Unsigned 64bit number to convert.
	@return std::string - Converted string.
	*/
	std::string intToString(uint64_t num);

	/**
	 @brief Converts float to a string
	 Created because some compilers still don't support std::to_string()

	 @param num - Number to convert.

	 @return std::string - Converted string.
	 */
	std::string floatToString(float num);

	/**
	@brief Converts float to a string
	Created because some compilers still don't support std::to_string()

	@param num - Number to convert.
	@param precision - How many numbers after the decimal point

	@return std::string - Converted string.
	*/
	std::string floatToString(float num, unsigned int precision);

	/**
	 @brief Converts a Vector 2 to a string.

	 @param num Vector2 to convert.

	 @return A std::string formatted like this "Vec2(x, y)".
	 */
	std::string vec2ToString(Vec2 num);

	/**
	@brief SDL version struct to string e.g "2.0.4".

	@param version The SDL version struct.

	@return A std::string formatted like this "2.0.4" (major.minor.patch).
	 */
	std::string sdlVersionToString(SDL_version version);


	/**
	 @brief Scale a Vec2 relative to the passed in screen size (Bases everything on a base resolution of 640,480).

	 @param vecToScale The vector to scale.
	 @param screenSize Size of the screen.

	 @return A scaled Vec2.
	 */
	Vec2 scaleTo(Vec2 vecToScale, Vec2 screenSize);

	/**
	@brief Scales the font size by screen size to make text appear less blurry.

	@param intToScale The int to scale.
	@param screenSize Size of the screen.

	@return Scaled int.
	 */
	int fontSizeScale(int intToScale, Vec2 screenSize);

	//Normalized

	///A function to normalise a number to between 1.0f and -1.0f, using the range that the number already uses
	float normaliseBetweenMinusOneAndOne(float numberToNormalise, float max, float min);


	///A function to normalise a number to between 0 and 1, using the range that the number already exists in
	float normaliseFloat(float numberToNormalise, float max, float min);

	/**
	Converts the angle to a radian.
	@param angle The angle to convert.
	@returns The converted radian.
	*/
	float convertAngleToRadian(float angle);

	/**
	@brief Returns the center position of a rectangle.

	@param rect - Rectangle struct.

	@return Center position of the rectangle.
	 */
	Vec2 getRectCenter(SDL_Rect rect);

	/**
	@brief Checks if two lines intersect Code originally from: http://stackoverflow.com/a/3746601.

	@param a1 - Line A point 1.
	@param a2 - Line A point 2.
	@param b1 - Line B point 1.
	@param b2 - Line B point 2.

	@return Is there an intersection.
	 */
	bool lineIntersection(Vec2 a1, Vec2 a2, Vec2 b1, Vec2 b2);

	/**
	@brief Checks if two lines intersect Code originally from: http://stackoverflow.com/a/3746601.

	@param a1					   - Line A point 1.
	@param a2					   - Line A point 2.
	@param b1					   - Line B point 1.
	@param b2					   - Line B point 2.
	@param [in,out] intersectPoint - The point where intersection occurred.

	@return Is there an intersection.
	 */
	bool lineIntersection(Vec2 a1, Vec2 a2, Vec2 b1, Vec2 b2, Vec2& intersectPoint);

	/**
	@brief Checks for Intersection between a line and a rect.

	@param lineP1 - Line Point 1.
	@param lineP2 - Line Point 2.
	@param rect   - A rectangle.

	@return Is there an intersection.
	 */
	bool lineRectIntersection(Vec2 lineP1, Vec2 lineP2, SDL_Rect rect);


	//Colour
	/**
	 @brief Creates a new colour.

	 @param r Red Channel.
	 @param g Green Channel.
	 @param b Blue Channel.
	 @param a Alpha Channel.

	 @return A SDL_Colour.
	 */
	SDL_Colour newSDLColour(uint8_t r, uint8_t g, uint8_t b, uint8_t a);


	//Timer Management

	/**
	@brief creates a timer for use in game

	*/
	class Timer
	{
	private:
		///a structure for the individual timers
		struct TimerStruct
		{
			float currentTime;
			float duration;
		};
	public:

		/**
		@brief updates the time in the timer.
		 */
		static void update();

		/**
		@brief Creates a new timer with a unique id and a duration.

		@param id	    - The id of the timer.
		@param duration - The duration of the timer.

		@return bool - false if id is not unique.
		 */
		static bool createTimer(std::string id, float duration);

		/**
		@brief Stops the time and returns the time taken.

		@param id - The id of the timer.

		@return float - the time taken.
		 */
		static float stopTimer(std::string id);

		/**
		@brief Checks to see if the timer has finished.

		@param id - The id of the timer.

		@return bool - if the timer has ended.
		 */
		static bool hasTimerFinished(std::string id);

		/**
		@brief Deletes all timers
		*/
		static void cleanup();

	private:
		//No one should call this so it is private
		Timer();

		/** @brief The time interval. */
		static const float TIME_INTERVAL;

		/**
		@brief Stores the Timers

		1 - currentTime
		2 - duration
		*/
		static std::unordered_map<std::string, TimerStruct> timers;
	};

	/** @brief A simple timer. */
	class SimpleTimer
	{
	private:
		///a structure for an individual timer
		struct TimerStruct
		{
			float currentTime;
			float duration;
		};
	public:
		/**
		 @brief Constructor.

		 @param duration The duration of the timer.
		 */
		SimpleTimer(float duration);

		/** @brief Restarts this SimpleTimer with the same duration. */
		void restart();

		/**
		 @brief Restarts this SimpleTimer with a new duration.

		 @param duration The duration.
		 */
		void restart(float duration);

		/** @brief Stops this SimpleTimer. */
		void stop();

		/**
		 @brief Query if this SimpleTimer has timer finished.

		 @return true if timer finished, false if not.
		 */
		bool hasTimerFinished();

		float remainingTime();


		/**
		 @brief Updates the Timer (increments the timer).
		 */
		void update();

	private:
		const float TIME_INTERVAL;

		/** @brief The timer. */
		TimerStruct timer;

		/** @brief Is the timer running. */
		bool running;
	};
}
