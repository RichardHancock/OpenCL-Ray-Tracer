#pragma once

/** @brief	A class that contains methods to get the current memory usage of the program. */
class MemoryCounter
{
public:

	/**
	 @brief	Gets current physical + virtual memory usage.
	
	 @return	The memory usage in bytes.
	 */
	static long long getMemoryUsage();
};