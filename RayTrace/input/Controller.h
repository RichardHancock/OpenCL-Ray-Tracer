#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <SDL.h>

#include "../misc/Vec2.h"

class Controller
{
public:

	enum Axis1D {
		LeftTrigger,
		RightTrigger
	};

	enum Axis2D {
		LeftStick,
		RightStick
	};

	enum Button
	{
		A = SDL_CONTROLLER_BUTTON_A,
		B = SDL_CONTROLLER_BUTTON_B,
		X = SDL_CONTROLLER_BUTTON_X,
		Y = SDL_CONTROLLER_BUTTON_Y,
		BACK = SDL_CONTROLLER_BUTTON_BACK,
		GUIDE = SDL_CONTROLLER_BUTTON_GUIDE,
		START = SDL_CONTROLLER_BUTTON_START,
		LEFTSTICK = SDL_CONTROLLER_BUTTON_LEFTSTICK,
		RIGHTSTICK = SDL_CONTROLLER_BUTTON_RIGHTSTICK,
		LEFTSHOULDER = SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
		RIGHTSHOULDER = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
		DPAD_UP = SDL_CONTROLLER_BUTTON_DPAD_UP,
		DPAD_DOWN = SDL_CONTROLLER_BUTTON_DPAD_DOWN,
		DPAD_LEFT = SDL_CONTROLLER_BUTTON_DPAD_LEFT,
		DPAD_RIGHT = SDL_CONTROLLER_BUTTON_DPAD_RIGHT
	};


	//Explanation here: http://www.drdobbs.com/windows/user-defined-hash-functions-for-unordere/231600210?pgno=3
	// Basically non VS compilers require a hash function for all User Defined types.
	struct ButtonHash
	{
		size_t operator()(const Button button) const
		{
			return std::hash<int>()(button);
		}
	};

	Controller(int joyID);

	~Controller();

	bool isValid();

	std::string getName();

	int getJoystickID();

	SDL_JoystickID getJoystickInstanceID();


	bool wasButtonPressed(Button button);

	bool isButtonHeld(Button button);

	bool wasButtonReleased(Button button);

	float getAxis1D(Axis1D axis);

	Vec2 getAxis2D(Axis2D axis);


	void rumblePlay(float strength, uint32_t lengthMS);

	void rumbleStop();


	void processEvents(SDL_Event& e);

	void update();
private:

	const Sint16 AXIS_MIN = -32768;
	const Sint16 AXIS_MAX = 32767;

	enum ButtonState {
		Pressed,
		Held,
		Released,
		None,
		Unknown
	};

	void updateAxis(SDL_Event& e);

	void updateButtons(SDL_Event& e);

	SDL_GameController* gameController;

	int joystickID;

	SDL_JoystickID joystickInstanceID;


	Vec2 leftStick;

	Vec2 rightStick;

	float leftTrigger;

	float rightTrigger;

	std::unordered_map<Button, ButtonState, ButtonHash> buttons;

	//Haptics
	void initializeHaptics(SDL_Joystick* joystick);

	SDL_Haptic* haptic;

	bool rumbleSupported;
};
