#include "Log.h"

#include <SDL.h>
#include <time.h>

void Log::log(LogType type, std::string message)
{
	//Get current time
	time_t currentTime = time(NULL);

	char textTime[9];

#ifdef _MSC_VER
	//Because Microsoft like to be different they depreceated the normal localtime(), and most compilers implement localtime_r() not _s
	tm timeStruct;
	localtime_s(&timeStruct, &currentTime);

	strftime(textTime, 9, "%H:%M:%S", &timeStruct);

#else

	tm* timeStruct;
	timeStruct = localtime(&currentTime);

	strftime(textTime, 11, "%T", timeStruct); //Apparently only Microsoft don't support the %T.

#endif

	std::string finalMsg = textTime;

	switch (type)
	{
	case I:
		finalMsg += (" " + message);
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", finalMsg.c_str());
		break;

	case W:
		finalMsg += (" " + message);
		SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s", finalMsg.c_str());
		break;

	case E:
		finalMsg += (" " + message);
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", finalMsg.c_str());
		break;

	default:
		finalMsg += (" Uncategorized: " + message);
		SDL_Log("%s", finalMsg.c_str());
		break;

	}
}

void Log::logI(std::string message)
{
	Log::log(Log::I, message);
}

void Log::logW(std::string message)
{
	Log::log(Log::W, message);
}

void Log::logE(std::string message)
{
	Log::log(Log::E, message);
}
