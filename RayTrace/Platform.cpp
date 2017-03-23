#include "Platform.h"

#include <string>
#include <fstream>

#include "misc/Log.h"
#include "misc/Utility.h"


Platform::Platform(std::string settingsFilePath)
	: scale(Vec2(640, 480)),
	settingsFilePath(settingsFilePath),
	defaultSettingsPath("resources/defaultSettings.xml")
{
	window = nullptr;
	renderer = nullptr;
}

Platform::~Platform()
{
	IMG_Quit();
	Mix_CloseAudio();
	Mix_Quit();
	TTF_Quit();
	
	if (context != 0) 
	{
		SDL_GL_DeleteContext(context);
	}

	if (renderer != nullptr)
	{
		SDL_DestroyRenderer(renderer);
	}

	SDL_DestroyWindow(window);
}

bool Platform::initSDL(bool openGL)
{
	///@todo abort program on every error rather than just waiting

	bool status = true;
	
	//Android doesn't need this at SDL internally does this
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) < 0) 
	{ 
		status = false; 
		Log::logE("SDL Init failed: " + std::string(SDL_GetError()));
	}
	
	//SDL Version Information for Debug
	printSDLVersion();


	//SDL TTF Initialization
	if (TTF_Init() < 0)
	{
		status = false;
		Log::logE("SDL_ttf init failed: " + std::string(TTF_GetError()));
	}

	//SDL Mixer Initialization
	Mix_Init(MIX_INIT_OGG);
	//Initialize SDL_Mixer with some standard audio formats/freqs. Also set channels to 2 for stereo sound.
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		status = false;
		Log::logE("SDL_mixer init failed: " + std::string(Mix_GetError()));
	}
	
	//SDL Image Initialization
	int flags= IMG_INIT_PNG;
	int result = IMG_Init(flags);
	
	// If the inputed flags are not returned, an error has occurred
	if((result & flags) != flags) 
	{
		Log::logE("Failed to Initialise SDL_Image and png support: "+ std::string(IMG_GetError()));
	}


	//Set OpenGL params
	///@todo Make these configurable
	if (openGL)
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		if (settings["MSAA"] != 0)
		{
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, settings["MSAA"]);
		}
	}
	
	
	uint32_t windowFlags = SDL_WINDOW_OPENGL; 

	switch (fullscreenMode)
	{
	case Setting::Windowed:
		Log::logI("Window Mode: Windowed");
		break;

	case Setting::Fullscreen:
		Log::logI("Window Mode: Fullscreen");
		windowFlags |= SDL_WINDOW_FULLSCREEN;
		break;
	
	case Setting::Borderless:
		Log::logI("Window Mode: Borderless");
		windowFlags |= SDL_WINDOW_BORDERLESS;
		break;
	}

	window = SDL_CreateWindow(
		"GCP A2 by Richard Hancock",
		SDL_WINDOWPOS_CENTERED, 
		SDL_WINDOWPOS_CENTERED,
		(int) windowSize.x, 
		(int) windowSize.y,
		windowFlags
		);


	if (!window) 
	{ 
		status = false;
		Log::logE("Window failed to be created: " + 
			std::string(SDL_GetError()));
	}
	

	if (openGL) 
	{
		context = SDL_GL_CreateContext(window);

		if (context == nullptr) {
			status = false;
			Log::logE("GL context failed to be created: " + std::string(SDL_GetError()));
		}

	}
	else 
	{
		renderer = SDL_CreateRenderer(window, -1, 0);
		
		if (renderer == nullptr)
		{
			Log::logE("SDL Renderer failed to be created: " + std::string(SDL_GetError()));
			status = false;
		}
	}


	int width;
	int height;

	SDL_GetWindowSize(window, &width, &height);

	windowSize.x = (float)width;
	windowSize.y = (float)height;
	
	Log::logI("Window Dimensions: " + Utility::intToString(width) +
		"x" + Utility::intToString(height));

	//Feature Detection
	checkFeatureSupport();

	//Platform Details
	Log::logI("CPU Cores: " + Utility::intToString(SDL_GetCPUCount()));
	Log::logI("CPU L1 Cache: " + Utility::intToString(SDL_GetCPUCacheLineSize()) + "KB");
	Log::logI("RAM: " + Utility::intToString(SDL_GetSystemRAM()) + "MB");


	return status;
}


SDL_Renderer* Platform::getRenderer()
{
	if (renderer != nullptr)
	{
		return renderer;
	}
	else
	{
		Log::logE("SDL Renderer requested but it does not exist (Due to either an error or OpenGL being requested instead)");
		return nullptr;
	}
}

SDL_GLContext Platform::getContext()
{
	if (context != 0)
	{
		return context;
	}
	else
	{
		Log::logE("OpenGL Context requested but it does not exist (Due to either an error or OpenGL not being requested)");
		return 0;
	}
}


void Platform::printSDLVersion()
{
	SDL_version compiled;
	SDL_version linked;

	SDL_VERSION(&compiled);
	SDL_GetVersion(&linked);

	Log::logI("SDL Version:");
	Log::logI("Compiled: " + Utility::sdlVersionToString(compiled));
	Log::logI("Linked: " + Utility::sdlVersionToString(linked));
}


void Platform::loadSettingsFromFile()
{
	//Check if file exists, if not create the base settings file
	if (!settingsFileExists())
	{
		initSettingsFile();
	}

	pugi::xml_document settingsFile;
	pugi::xml_parse_result result = settingsFile.load_file(settingsFilePath.c_str());

	if (!result)
	{
		Log::logE("Settings File could not be parsed:");
		Log::logE(" - Path: " + settingsFilePath);
		Log::logE(" - Error Description: " + std::string(result.description()));

		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
			"Settings File Missing or Corrupted",
			"Settings File Missing or Corrupted",
			NULL);

		exit(-1);
	} 

	//TODO make this less repetitive.
	//TODO need to check if the setting is actually being fetched or whether the default value is used instead (0).

	//Resolution
	pugi::xml_node resolution = settingsFile.child("resolution");
	Vec2 tempWindowSize = {
		(float) atoi(resolution.attribute("width").value()),
		(float) atoi(resolution.attribute("height").value())
	};
	Log::logI("Window Size from Settings File: " + Utility::vec2ToString(tempWindowSize));

	if (tempWindowSize.x < 640 || tempWindowSize.y < 480)
	{
		Log::logE("Windows size is too small, please increase (min 640x480) in settings file");
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
			"Resolution Invalid",
			"Resolution in settings file is too small, please increase to at least 640x480.",
			NULL);
		exit(-1);
	}

	windowSize = tempWindowSize;


	//Fullscreen
	int tempFullscreenMode = atoi(settingsFile.child("fullscreen").child_value());
	Log::logI("Fullscreen Mode from Settings File: " + Utility::intToString(tempFullscreenMode));

	if (tempFullscreenMode < 0 || tempFullscreenMode > 2)
	{
		Log::logE("Fullsceen Mode Setting is invalid, please reset to 0, 1 or 2 in settings file");
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
			"Fullscreen Mode Invalid",
			"Fullsceen Mode Setting is invalid, please reset to 0, 1 or 2 in settings file.",
			NULL);
		exit(-1);
	}

	fullscreenMode = (Setting::FullscreenMode) tempFullscreenMode;

	
	//MSAA
	int msaaSamples = atoi(settingsFile.child("MSAA").child_value());
	Log::logI("Anti Aliasing sample count from Settings File: " + Utility::intToString(msaaSamples));

	if (msaaSamples < 0 || msaaSamples > 16)
	{
		Log::logE("MSAA Setting is invalid, please reset to 0 - 16 in settings file");
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
			"MSAA Setting Invalid",
			"MSAA Setting is invalid, please reset to 0 - 16 in settings file.",
			NULL);
		exit(-1);
	}

	settings["MSAA"] = msaaSamples;
	
}

void Platform::initSettingsFile()
{
	std::ifstream defaultSettings(defaultSettingsPath);

	if (!defaultSettings.is_open())
	{
		Log::logE("Default Settings file missing");
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
			"Default Settings file missing",
			"Default Settings file missing. Please reinstall the program.",
			NULL);
		exit(-1);
	}


	Log::logI("Creating Settings File at " + settingsFilePath);
	std::ofstream newSettingsFile(settingsFilePath, std::fstream::trunc);

	if (!newSettingsFile.is_open())
	{
		Log::logE("Could not create Settings file");
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
			"Could not create Settings file",
			"Could not create Settings file. This could be a permissions issue.",
			NULL);
		exit(-1);
	}


	while (!defaultSettings.eof())
	{
		char line[1024];

		defaultSettings.getline(line, 1024);
		newSettingsFile << line << std::endl;
	}
}

bool Platform::settingsFileExists()
{
	std::ifstream settingsFile(settingsFilePath);
	
	return settingsFile.is_open();
}


int Platform::getSetting(std::string setting)
{
	if (settings.count(setting) == 0)
	{
		Log::logW(setting + " setting does not exist, returning 0.");
		return 0;
	}

	return settings[setting];
}


bool Platform::isFeatureSupported(std::string feature)
{
	if (features.count(feature) == 0)
	{
		Log::logW(feature + " feature is unknown, assuming unsupported.");
		return false;
	}

	return features[feature];
}

void Platform::checkFeatureSupport()
{
	features["3DNow"] = SDL_Has3DNow() == SDL_TRUE;
	features["AVX"] = SDL_HasAVX() == SDL_TRUE;
	features["AVX2"] = SDL_HasAVX2() == SDL_TRUE;
	features["AltiVec"] = SDL_HasAltiVec() == SDL_TRUE;
	features["MMX"] = SDL_HasMMX() == SDL_TRUE;
	features["RDTSC"] = SDL_HasRDTSC() == SDL_TRUE;
	features["SSE"] = SDL_HasSSE() == SDL_TRUE;
	features["SSE2"] = SDL_HasSSE2() == SDL_TRUE;
	features["SSE3"] = SDL_HasSSE3() == SDL_TRUE;
	features["SSE41"] = SDL_HasSSE41() == SDL_TRUE;
	features["SSE42"] = SDL_HasSSE42() == SDL_TRUE;

	Log::logI("Platform Features:");
	for (auto feature : features)
	{
		Log::logI(" - " + feature.first + ": " + (feature.second ? "Yes" : "No"));
	}
}