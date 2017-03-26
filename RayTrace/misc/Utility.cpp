#include "Utility.h"

#include <sstream>

#include "Log.h"
#include "DeltaTime.h"

std::string Utility::intToString(int num)
{
	std::stringstream stream;

	stream << num;
	std::string result = stream.str();

	return result;

}

std::string Utility::intToString(uint64_t num)
{
	std::stringstream stream;

	stream << num;
	std::string result = stream.str();

	return result;
}

std::string Utility::floatToString(float num)
{
	std::stringstream stream;

	stream << num;
	std::string result = stream.str();

	return result;
}

std::string Utility::floatToString(float num, unsigned int precision)
{
	std::stringstream stream;
	stream.precision(precision);
	stream << std::fixed << num;
	std::string result = stream.str();

	return result;
}

std::string Utility::vec2ToString(Vec2 num)
{
	std::stringstream stream;

	stream << num.x;
	stream << ", ";
	stream << num.y;
	std::string result = stream.str();

	return result;

}

std::string Utility::sdlVersionToString(SDL_version version)
{
	return (
		Utility::intToString(version.major) + "." +
		Utility::intToString(version.minor) + "." +
		Utility::intToString(version.patch)
		);
}


Vec2 Utility::scaleTo(Vec2 vecToScale, Vec2 screenSize)
{
	vecToScale.x /= 640;
	vecToScale.y /= 480;

	return (vecToScale * screenSize);
}

int Utility::fontSizeScale(int intToScale, Vec2 screenSize)
{
	int percentageChange = (int)(screenSize.x / 640.0f);

	return (intToScale * percentageChange);
}


float Utility::normaliseBetweenMinusOneAndOne(float numberToNormalise, float max, float min)
{
	//find the new max and min number when shifted to a positive only range and the shifted number to normalise
	max += std::abs(min);
	numberToNormalise += std::abs(min);
	min = 0.0f;

	//normalise the number between zero and one
	float normalisedNumber = normaliseFloat(numberToNormalise, max, min);

	//shift it to between -0.5f and 0.5f
	normalisedNumber -= 0.5f;

	//scale it to between -1.0f and 1.0f
	normalisedNumber *= 2.0f;

	//return the normalised number
	return normalisedNumber;
}


float Utility::normaliseFloat(float numberToNormalise, float max, float min)
{
	//normalise the number between zero and one
	float normalisedNumber = (numberToNormalise - min) / (max - min);

	//return the normalised number
	return normalisedNumber;
}


Vec2 Utility::getRectCenter(SDL_Rect rect)
{
	Vec2 center;
	center.x = (float)(rect.x + (rect.w / 2));
	center.y = (float)(rect.y + (rect.h / 2));
	return center;
}

bool Utility::lineIntersection(Vec2 a1, Vec2 a2, Vec2 b1, Vec2 b2)
{
	Vec2 b = a2 - a1;
	Vec2 d = b2 - b1;
	float bDotDPerp = b.x * d.y - b.y * d.x;

	// if b dot d == 0, it means the lines are parallel so have infinite intersection points
	if (bDotDPerp == 0)
		return false;

	Vec2 c = b1 - a1;
	float t = (c.x * d.y - c.y * d.x) / bDotDPerp;
	if (t < 0 || t > 1)
		return false;

	float u = (c.x * b.y - c.y * b.x) / bDotDPerp;
	if (u < 0 || u > 1)
		return false;

	return true;
}

bool Utility::lineIntersection(Vec2 a1, Vec2 a2, Vec2 b1, Vec2 b2, Vec2& intersectPoint)
{
	intersectPoint = 0;

	Vec2 b = a2 - a1;
	Vec2 d = b2 - b1;
	float bDotDPerp = b.x * d.y - b.y * d.x;

	// if b dot d == 0, it means the lines are parallel so have infinite intersection points
	if (bDotDPerp == 0)
		return false;

	Vec2 c = b1 - a1;
	float t = (c.x * d.y - c.y * d.x) / bDotDPerp;
	if (t < 0 || t > 1)
		return false;

	float u = (c.x * b.y - c.y * b.x) / bDotDPerp;
	if (u < 0 || u > 1)
		return false;

	intersectPoint = a1 + b * t;

	return true;
}

bool Utility::lineRectIntersection(Vec2 lineP1, Vec2 lineP2, SDL_Rect rect)
{
	//Could have used this instead: https://wiki.libsdl.org/SDL_IntersectRectAndLine

	//tl - top left/ tr - top right/ bl - bottom left/ br - bottom right
	Vec2 tl((float)rect.x, (float)rect.y);
	Vec2 tr = tl;
	tr.x += rect.w;

	Vec2 bl = tl;
	bl.y += rect.h;
	Vec2 br = tr;
	br.y += rect.h;

	if (
		lineIntersection(lineP1, lineP2, tl, tr) ||
		lineIntersection(lineP1, lineP2, tr, br) ||
		lineIntersection(lineP1, lineP2, br, bl) ||
		lineIntersection(lineP1, lineP2, bl, tl)
	)
	{
		return true;
	}
	else
	{
		return false;
	}
}



SDL_Colour Utility::newSDLColour(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	SDL_Colour colour;
	colour.r = r;
	colour.g = g;
	colour.b = b;
	colour.a = a;
	return colour;
}



//Timer

std::unordered_map<std::string, Utility::Timer::TimerStruct> Utility::Timer::timers;
const float Utility::Timer::TIME_INTERVAL = 1.0f;

Utility::Timer::Timer() {}

void Utility::Timer::update()
{
	//Need auto& to get a reference otherwise it's just a local copy (Really Fun Bug to Find)
	for (auto& timer : timers)
	{
		timer.second.currentTime += (float)(TIME_INTERVAL * DeltaTime::getDT());
	}
}

bool Utility::Timer::createTimer(std::string id, float duration)
{
	//Unordered map can't have duplicates
	if (timers.count(id) == 1)
	{
		Log::logW("Timer id specified already exists");
		return false;
	}

	//Create the Timer
	TimerStruct timer = {0.0f, duration};

	//Push it into the timers unordered_map
	timers.insert(std::make_pair(id,timer));

	return true;
}

float Utility::Timer::stopTimer(std::string id)
{
	if (timers.count(id) == 0) { return -1.0f; }

	float finalCurrentTime = timers[id].currentTime;

	timers.erase(id);

	return finalCurrentTime;
}

bool Utility::Timer::hasTimerFinished(std::string id)
{
	if (!timers.count(id))
	{
		Log::logW("Unknown timer was asked if finished. Timer ID:" +id);
		return false;
	}

	TimerStruct timer = timers[id];
	if (timer.currentTime >= timer.duration)
	{
		//Could call stop timer but would have an extra id check (Which is wasteful)
		timers.erase(id);
		return true;
	}
	else
	{
		return false;
	}
}

void Utility::Timer::cleanup()
{
	timers.clear();
}


//Simple Timer
Utility::SimpleTimer::SimpleTimer(float duration)
	: TIME_INTERVAL(1.0f)
{
	restart(duration);
}

void Utility::SimpleTimer::restart()
{
	running = true;
	timer.currentTime = 0.0f;
}

void Utility::SimpleTimer::restart(float duration)
{
	running = true;
	timer.currentTime = 0.0f;
	timer.duration = duration;
}

void Utility::SimpleTimer::stop()
{
	running = false;
	timer.currentTime = 0.0f;
}

bool Utility::SimpleTimer::hasTimerFinished()
{
	if (timer.currentTime >= timer.duration)
	{
		return true;
	}
	return false;
}

float Utility::SimpleTimer::remainingTime()
{
	return timer.duration - timer.currentTime;
}

void Utility::SimpleTimer::update()
{
	if (running)
	{
		timer.currentTime += DeltaTime::getDT();

		if (timer.currentTime >= timer.duration)
		{
			running = false;
		}
	}
}

float Utility::convertAngleToRadian(float angle)
{
	/*return the radian*/
	return (angle * PI / 180.0f);
}
