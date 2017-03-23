#include "Controller.h"
#include "../misc/Log.h"
#include "../misc/Utility.h"
#include <assert.h>


Controller::Controller(int joyID)
	: joystickID(joyID), leftStick(0), rightStick(0), leftTrigger(0), rightTrigger(0), 
	rumbleSupported(false)
{

	buttons = {
		{ A , None },
		{ B , None },
		{ X , None },
		{ Y , None },
		{ BACK , None },
		{ GUIDE , None },
		{ START , None },
		{ LEFTSTICK , None },
		{ RIGHTSTICK , None },
		{ LEFTSHOULDER , None },
		{ RIGHTSHOULDER , None },
		{ DPAD_UP , None },
		{ DPAD_DOWN , None },
		{ DPAD_LEFT , None },
		{ DPAD_RIGHT , None }
	};

	gameController = SDL_GameControllerOpen(joyID);
	if (gameController == nullptr)
	{
		Log::logE("Controller (" + Utility::intToString(joyID) + ") did not Init");
		Log::logE(SDL_GetError());
		return;
	}

	SDL_Joystick* joy = SDL_GameControllerGetJoystick(gameController);
	joystickInstanceID = SDL_JoystickInstanceID(joy);


	initializeHaptics(joy);
}

Controller::~Controller()
{
	if (haptic != nullptr)
	{
		SDL_HapticStopAll(haptic);
		SDL_HapticClose(haptic);
		haptic = nullptr;
	}

	SDL_GameControllerClose(gameController);
	gameController = nullptr;

	buttons.clear();

}

bool Controller::isValid()
{
	return (gameController != nullptr);
}

std::string Controller::getName()
{
	return std::string(SDL_GameControllerName(gameController));
}

int Controller::getJoystickID()
{
	return joystickID;
}

SDL_JoystickID Controller::getJoystickInstanceID()
{
	return joystickInstanceID;
}


bool Controller::wasButtonPressed(Button button)
{
	return (buttons.count(button) == 1 && buttons[button] == Pressed);
}

bool Controller::isButtonHeld(Button button)
{
	return (buttons.count(button) == 1 && (buttons[button] == Pressed || buttons[button] == Held));
}

bool Controller::wasButtonReleased(Button button)
{
	return (buttons.count(button) == 1 && buttons[button] == Released);
}

float Controller::getAxis1D(Axis1D axis)
{
	switch (axis)
	{
	case Controller::LeftTrigger:
		return leftTrigger;
		break;

	case Controller::RightTrigger:
		return rightTrigger;
		break;

	default:
		Log::logW("Invalid Axis1D requested");
		return 0.0f;
		break;
	}
}

Vec2 Controller::getAxis2D(Axis2D axis)
{
	switch (axis)
	{
	case Controller::LeftStick:
		return leftStick;
		break;
	
	case Controller::RightStick:
		return rightStick;
		break;

	default:
		Log::logW("Invalid Axis2D requested");
		return Vec2(0);
		break;
	}
}


void Controller::processEvents(SDL_Event& e)
{
	switch (e.type)
	{
	case SDL_CONTROLLERAXISMOTION:
		updateAxis(e);
		break;

	case SDL_CONTROLLERBUTTONUP:
	case SDL_CONTROLLERBUTTONDOWN:
		updateButtons(e);
		break;
	}
}

void Controller::updateAxis(SDL_Event& e)
{
	Sint16 value = e.caxis.value;

	switch (e.caxis.axis)
	{
	case SDL_CONTROLLER_AXIS_LEFTX:
		leftStick.x = Utility::normaliseBetweenMinusOneAndOne(value, AXIS_MAX, AXIS_MIN);
		break;
	case SDL_CONTROLLER_AXIS_LEFTY:
		leftStick.y = Utility::normaliseBetweenMinusOneAndOne(value, AXIS_MAX, AXIS_MIN);
		break;

	case SDL_CONTROLLER_AXIS_RIGHTX:
		rightStick.x = Utility::normaliseBetweenMinusOneAndOne(value, AXIS_MAX, AXIS_MIN);
		break;
	case SDL_CONTROLLER_AXIS_RIGHTY:
		rightStick.y = Utility::normaliseBetweenMinusOneAndOne(value, AXIS_MAX, AXIS_MIN);
		break;

	case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
		leftTrigger = Utility::normaliseFloat(value, AXIS_MAX, 0);
		break;
	case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
		rightTrigger = Utility::normaliseFloat(value, AXIS_MAX, 0);
		break;
	}
}

void Controller::updateButtons(SDL_Event& e)
{
	ButtonState buttonState = (e.cbutton.type == SDL_CONTROLLERBUTTONDOWN ? Pressed : Released);
	
	//They provide a uint8 instead of the actual enum so it needs casting.
	Button button = (Button)e.cbutton.button;

	assert(buttons.count(button) == 1);

	buttons[button] = buttonState;
}

void Controller::initializeHaptics(SDL_Joystick* joystick)
{
	if (SDL_JoystickIsHaptic(joystick) == 1)
	{
		haptic = SDL_HapticOpenFromJoystick(joystick);
		if (haptic != nullptr)
		{
			Log::logI(this->getName() + "'s haptic features opened successfully");
		}
		else
		{
			Log::logW(this->getName() + "'s haptic features could not be opened due to an error: " + SDL_GetError());
			return;
		}
	}
	else
	{
		Log::logI(this->getName() + " does not support haptic features");
		haptic = nullptr;
		return;
	}

	//Init Rumble
	switch (SDL_HapticRumbleSupported(haptic))
	{
	case SDL_TRUE:
		rumbleSupported = true;
		break;

	case SDL_FALSE:
		Log::logI(this->getName() + " does not support the Rumble Haptic");
		rumbleSupported = false;
		return;
		break;

	default:
		Log::logW(this->getName() + 
			" had an error occur when querying its support of rumble features: " + SDL_GetError());
		rumbleSupported = false;
		return;
		break;

	}

	if (SDL_HapticRumbleInit(haptic) != 0)
	{
		Log::logW(this->getName() + " had an error occur when initializing Rumble haptic: "
			+ SDL_GetError());
		rumbleSupported = false;
		return;
	}
}


void Controller::rumblePlay(float strength, uint32_t lengthMS)
{
	if (rumbleSupported)
	{
		if (SDL_HapticRumblePlay(haptic, strength, lengthMS) != 0)
		{
			Log::logW(this->getName() + " had an error occur when trying to play a rumble haptic: "
				+ SDL_GetError());
		}
	}
}

void Controller::rumbleStop()
{
	if (rumbleSupported)
	{
		if (SDL_HapticRumbleStop(haptic) != 0)
		{
			Log::logW(this->getName() + " had an error occur when trying to stop a rumble haptic: "
				+ SDL_GetError());
		}
	}
}


void Controller::update()
{
	//Buttons
	for (auto& button : buttons)
	{
		ButtonState buttonState = button.second;

		if (buttonState == Pressed) {
			button.second = Held;
		}
		else if (buttonState == Released) {
			button.second = None;
		}
	}

}

