#pragma once

#include <string>

namespace Log
{
	/**
	@brief A type of log message
	*/
	enum LogType
	{
		E, ///< Error
		W, ///< Warning
		I  ///< Info
	};

	/**
	@brief Log a message to the console

	Made this one function instead of multiple to reduce clutter. Uses SDL_Log internally to make it
	function on multiple platforms easily.

	@param type    Type of message E: Error, W: Warning, I: Info.
	@param message The message to log.
	*/
	void log(LogType type, std::string message);

	/**
	@brief Log a info message to the console

	@param message The message to log.
	*/
	void logI(std::string message);

	/**
	@brief Log a warning message to the console

	@param message The message to log.
	*/
	void logW(std::string message);

	/**
	@brief Log a error message to the console

	@param message The message to log.
	*/
	void logE(std::string message);
}