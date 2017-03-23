#pragma once

class DeltaTime
{
public:
	static float getDT();

	static void init();

	static void update();

private:

	static float deltaTime;

	static unsigned int lastTime;
};