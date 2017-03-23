#pragma once

#include <unordered_map>
#include <pugixml.hpp>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include "misc/Vec2.h"

//Quick Pre-processor key 
#ifdef _WIN32
	//Windows 32/64 bit
#elif __ANDROID__
	//Android (Is also covered by __linux__ preprocessor so must be before it)
#elif EMSCRIPTEN
	//Emscripten (Web Browsers), not sure if this is correct macro
	// Cant develop for Emscripten as it lacks support for a few major SDL 2 add ons.
#elif __APPLE__
	//Mac OSX and iOS (Probably wont be used as I can't test either)
#elif __linux__
	//Linux Kernel
#endif

namespace Setting 
{
	enum FullscreenMode
	{
		Windowed = 0,
		Fullscreen = 1,
		Borderless = 2
	};
}


/** @brief Class that handles the initialization of SDL 2 (and its add ons) across all supported platforms */
class Platform
{
public:

	/**
	@brief Constructor.
	
	@param settingsFilePath Relative pathname of the settings file.
	 */
	Platform(std::string settingsFilePath);

	/** @brief Destructor, Calls SDL's cleanup features for itself and its add ons. */
	~Platform();

	/**
	@brief Initialises the SDL library and its plugins, for the current platform.
	
	@param openGL true to use openGL instead of the 2D SDL renderer.
	
	@return bool - Was successful.
	 */
	bool initSDL(bool openGL);

	void loadSettingsFromFile();


	/**
	 @brief Gets the window.
	
	 @return null if it fails, else the window.
	 */
	SDL_Window* getWindow() { return window; }

	/**
	@brief Gets the SDL 2D renderer.
	Not initialized if OpenGL was requested

	@return nullptr if it fails, else the renderer.
	 */
	SDL_Renderer* getRenderer();
	
	/**
	@brief Gets the GL context.
	
	@return null if it fails, else the context.
	 */
	SDL_GLContext getContext();

	/**
	 @brief Gets window size.
	
	 @return The window size.
	 */
	Vec2 getWindowSize() { return windowSize; }


	/** @brief Outputs the linked and compiled SDL version numbers into the log. */
	void printSDLVersion();


	int getSetting(std::string setting);

	bool isFeatureSupported(std::string feature);
	
private:

	/** @brief The window. */
	SDL_Window* window;


	/** @brief The 2D SDL renderer. Not created with OpenGL enabled */
	SDL_Renderer* renderer;

	/** @brief The GL context. */
	SDL_GLContext context;

	/** @brief Size of the window. */
	Vec2 windowSize;

	//The resolution everything is scaled from
	const Vec2 scale;


	//Settings
	void initSettingsFile();

	bool settingsFileExists();

	const std::string settingsFilePath;

	const std::string defaultSettingsPath;

	Setting::FullscreenMode fullscreenMode;
	
	std::unordered_map<std::string, int> settings;

	//Platform Feature Support (Wraps SDL CPU feature detection)
	std::unordered_map<std::string, bool> features;

	void checkFeatureSupport();

	
};