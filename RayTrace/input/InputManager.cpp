#include "InputManager.h"

#include <assert.h>

#include "../misc/Utility.h"
#include "../misc/Log.h"

//Static Declarations
std::unordered_map<SDL_Keycode, InputManager::ButtonState> InputManager::keys = {
	{ SDLK_UNKNOWN, None },
	{ SDLK_RETURN, None },
	{ SDLK_ESCAPE, None },
	{ SDLK_BACKSPACE, None },
	{ SDLK_TAB, None },
	{ SDLK_SPACE, None },
	{ SDLK_EXCLAIM, None },
	{ SDLK_QUOTEDBL, None },
	{ SDLK_HASH, None },
	{ SDLK_PERCENT, None },
	{ SDLK_DOLLAR, None },
	{ SDLK_AMPERSAND, None },
	{ SDLK_QUOTE, None },
	{ SDLK_LEFTPAREN, None },
	{ SDLK_RIGHTPAREN, None },
	{ SDLK_ASTERISK, None },
	{ SDLK_PLUS, None },
	{ SDLK_COMMA, None },
	{ SDLK_MINUS, None },
	{ SDLK_PERIOD, None },
	{ SDLK_SLASH, None },
	{ SDLK_0, None },
	{ SDLK_1, None },
	{ SDLK_2, None },
	{ SDLK_3, None },
	{ SDLK_4, None },
	{ SDLK_5, None },
	{ SDLK_6, None },
	{ SDLK_7, None },
	{ SDLK_8, None },
	{ SDLK_9, None },
	{ SDLK_COLON, None },
	{ SDLK_SEMICOLON, None },
	{ SDLK_LESS, None },
	{ SDLK_EQUALS, None },
	{ SDLK_GREATER, None },
	{ SDLK_QUESTION, None },
	{ SDLK_AT, None },
	{ SDLK_LEFTBRACKET, None },
	{ SDLK_BACKSLASH, None },
	{ SDLK_RIGHTBRACKET, None },
	{ SDLK_CARET, None },
	{ SDLK_UNDERSCORE, None },
	{ SDLK_BACKQUOTE, None },
	{ SDLK_a, None },
	{ SDLK_b, None },
	{ SDLK_c, None },
	{ SDLK_d, None },
	{ SDLK_e, None },
	{ SDLK_f, None },
	{ SDLK_g, None },
	{ SDLK_h, None },
	{ SDLK_i, None },
	{ SDLK_j, None },
	{ SDLK_k, None },
	{ SDLK_l, None },
	{ SDLK_m, None },
	{ SDLK_n, None },
	{ SDLK_o, None },
	{ SDLK_p, None },
	{ SDLK_q, None },
	{ SDLK_r, None },
	{ SDLK_s, None },
	{ SDLK_t, None },
	{ SDLK_u, None },
	{ SDLK_v, None },
	{ SDLK_w, None },
	{ SDLK_x, None },
	{ SDLK_y, None },
	{ SDLK_z, None },
	{ SDLK_CAPSLOCK, None },
	{ SDLK_F1, None },
	{ SDLK_F2, None },
	{ SDLK_F3, None },
	{ SDLK_F4, None },
	{ SDLK_F5, None },
	{ SDLK_F6, None },
	{ SDLK_F7, None },
	{ SDLK_F8, None },
	{ SDLK_F9, None },
	{ SDLK_F10, None },
	{ SDLK_F11, None },
	{ SDLK_F12, None },
	{ SDLK_PRINTSCREEN, None },
	{ SDLK_SCROLLLOCK, None },
	{ SDLK_PAUSE, None },
	{ SDLK_INSERT, None },
	{ SDLK_HOME, None },
	{ SDLK_PAGEUP, None },
	{ SDLK_DELETE, None },
	{ SDLK_END, None },
	{ SDLK_PAGEDOWN, None },
	{ SDLK_RIGHT, None },
	{ SDLK_LEFT, None },
	{ SDLK_DOWN, None },
	{ SDLK_UP, None },
	{ SDLK_NUMLOCKCLEAR, None },
	{ SDLK_KP_DIVIDE, None },
	{ SDLK_KP_MULTIPLY, None },
	{ SDLK_KP_MINUS, None },
	{ SDLK_KP_PLUS, None },
	{ SDLK_KP_ENTER, None },
	{ SDLK_KP_1, None },
	{ SDLK_KP_2, None },
	{ SDLK_KP_3, None },
	{ SDLK_KP_4, None },
	{ SDLK_KP_5, None },
	{ SDLK_KP_6, None },
	{ SDLK_KP_7, None },
	{ SDLK_KP_8, None },
	{ SDLK_KP_9, None },
	{ SDLK_KP_0, None },
	{ SDLK_KP_PERIOD, None },
	{ SDLK_APPLICATION, None },
	{ SDLK_POWER, None },
	{ SDLK_KP_EQUALS, None },
	{ SDLK_F13, None },
	{ SDLK_F14, None },
	{ SDLK_F15, None },
	{ SDLK_F16, None },
	{ SDLK_F17, None },
	{ SDLK_F18, None },
	{ SDLK_F19, None },
	{ SDLK_F20, None },
	{ SDLK_F21, None },
	{ SDLK_F22, None },
	{ SDLK_F23, None },
	{ SDLK_F24, None },
	{ SDLK_EXECUTE, None },
	{ SDLK_HELP, None },
	{ SDLK_MENU, None },
	{ SDLK_SELECT, None },
	{ SDLK_STOP, None },
	{ SDLK_AGAIN, None },
	{ SDLK_UNDO, None },
	{ SDLK_CUT, None },
	{ SDLK_COPY, None },
	{ SDLK_PASTE, None },
	{ SDLK_FIND, None },
	{ SDLK_MUTE, None },
	{ SDLK_VOLUMEUP, None },
	{ SDLK_VOLUMEDOWN, None },
	{ SDLK_KP_COMMA, None },
	{ SDLK_KP_EQUALSAS400, None },
	{ SDLK_ALTERASE, None },
	{ SDLK_SYSREQ, None },
	{ SDLK_CANCEL, None },
	{ SDLK_CLEAR, None },
	{ SDLK_PRIOR, None },
	{ SDLK_RETURN2, None },
	{ SDLK_SEPARATOR, None },
	{ SDLK_OUT, None },
	{ SDLK_OPER, None },
	{ SDLK_CLEARAGAIN, None },
	{ SDLK_CRSEL, None },
	{ SDLK_EXSEL, None },
	{ SDLK_KP_00, None },
	{ SDLK_KP_000, None },
	{ SDLK_THOUSANDSSEPARATOR, None },
	{ SDLK_DECIMALSEPARATOR, None },
	{ SDLK_CURRENCYUNIT, None },
	{ SDLK_CURRENCYSUBUNIT, None },
	{ SDLK_KP_LEFTPAREN, None },
	{ SDLK_KP_RIGHTPAREN, None },
	{ SDLK_KP_LEFTBRACE, None },
	{ SDLK_KP_RIGHTBRACE, None },
	{ SDLK_KP_TAB, None },
	{ SDLK_KP_BACKSPACE, None },
	{ SDLK_KP_A, None },
	{ SDLK_KP_B, None },
	{ SDLK_KP_C, None },
	{ SDLK_KP_D, None },
	{ SDLK_KP_E, None },
	{ SDLK_KP_F, None },
	{ SDLK_KP_XOR, None },
	{ SDLK_KP_POWER, None },
	{ SDLK_KP_PERCENT, None },
	{ SDLK_KP_LESS, None },
	{ SDLK_KP_GREATER, None },
	{ SDLK_KP_AMPERSAND, None },
	{ SDLK_KP_DBLAMPERSAND, None },
	{ SDLK_KP_VERTICALBAR, None },
	{ SDLK_KP_DBLVERTICALBAR, None },
	{ SDLK_KP_COLON, None },
	{ SDLK_KP_HASH, None },
	{ SDLK_KP_SPACE, None },
	{ SDLK_KP_AT, None },
	{ SDLK_KP_EXCLAM, None },
	{ SDLK_KP_MEMSTORE, None },
	{ SDLK_KP_MEMRECALL, None },
	{ SDLK_KP_MEMCLEAR, None },
	{ SDLK_KP_MEMADD, None },
	{ SDLK_KP_MEMSUBTRACT, None },
	{ SDLK_KP_MEMMULTIPLY, None },
	{ SDLK_KP_MEMDIVIDE, None },
	{ SDLK_KP_PLUSMINUS, None },
	{ SDLK_KP_CLEAR, None },
	{ SDLK_KP_CLEARENTRY, None },
	{ SDLK_KP_BINARY, None },
	{ SDLK_KP_OCTAL, None },
	{ SDLK_KP_DECIMAL, None },
	{ SDLK_KP_HEXADECIMAL, None },
	{ SDLK_LCTRL, None },
	{ SDLK_LSHIFT, None },
	{ SDLK_LALT, None },
	{ SDLK_LGUI, None },
	{ SDLK_RCTRL, None },
	{ SDLK_RSHIFT, None },
	{ SDLK_RALT, None },
	{ SDLK_RGUI, None },
	{ SDLK_MODE, None },
	{ SDLK_AUDIONEXT, None },
	{ SDLK_AUDIOPREV, None },
	{ SDLK_AUDIOSTOP, None },
	{ SDLK_AUDIOPLAY, None },
	{ SDLK_AUDIOMUTE, None },
	{ SDLK_MEDIASELECT, None },
	{ SDLK_WWW, None },
	{ SDLK_MAIL, None },
	{ SDLK_CALCULATOR, None },
	{ SDLK_COMPUTER, None },
	{ SDLK_AC_SEARCH, None },
	{ SDLK_AC_HOME, None },
	{ SDLK_AC_BACK, None },
	{ SDLK_AC_FORWARD, None },
	{ SDLK_AC_STOP, None },
	{ SDLK_AC_REFRESH, None },
	{ SDLK_AC_BOOKMARKS, None },
	{ SDLK_BRIGHTNESSDOWN, None },
	{ SDLK_BRIGHTNESSUP, None },
	{ SDLK_DISPLAYSWITCH, None },
	{ SDLK_KBDILLUMTOGGLE, None },
	{ SDLK_KBDILLUMDOWN, None },
	{ SDLK_KBDILLUMUP, None },
	{ SDLK_EJECT, None },
	{ SDLK_SLEEP, None },
};
std::unordered_map<uint8_t, InputManager::ButtonState> InputManager::mouseButtons = {
	{ SDL_BUTTON_LEFT, None },
	{ SDL_BUTTON_MIDDLE, None },
	{ SDL_BUTTON_RIGHT, None },
	{ SDL_BUTTON_X1, None },
	{ SDL_BUTTON_X2, None }
};
std::vector<Controller*> InputManager::gamepads;
Vec2 InputManager::mousePos;
Vec2 InputManager::mouseDirection;
Vec2 InputManager::mouseWheelDirection;
float InputManager::deadzone = 0.2f;


InputManager::InputManager()
{

}

void InputManager::cleanup()
{
	keys.clear();
	mouseButtons.clear();

	for (auto& gamepad : gamepads)
	{
		delete gamepad;
	}

	gamepads.clear();
}

//Keyboard
bool InputManager::wasKeyPressed(SDL_Keycode key)
{
	assert(keys.count(key) == 1);

	return (keys[key] == Pressed);
}

bool InputManager::isKeyHeld(SDL_Keycode key)
{
	assert(keys.count(key) == 1);

	return (keys[key] == Pressed || keys[key] == Held);
}

bool InputManager::wasKeyReleased(SDL_Keycode key)
{
	assert(keys.count(key) == 1);

	return (keys[key] == Released);;
}

//Quick shortcuts
bool InputManager::ctrl()
{
	return (isKeyHeld(SDLK_LCTRL) || isKeyHeld(SDLK_RCTRL));
}

bool InputManager::alt()
{
	return (isKeyHeld(SDLK_LALT) || isKeyHeld(SDLK_RALT));
}

bool InputManager::shift()
{
	return (isKeyHeld(SDLK_LSHIFT) || isKeyHeld(SDLK_RSHIFT));
}


//Mouse
bool InputManager::wasMouseButtonPressed(uint8_t button)
{
	assert(mouseButtons.count(button) == 1);

	return (mouseButtons[button] == Pressed);
}

bool InputManager::isMouseButtonHeld(uint8_t button)
{
	assert(mouseButtons.count(button) == 1);

	return (mouseButtons[button] == Pressed || mouseButtons[button] == Held);
}

bool InputManager::wasMouseButtonReleased(uint8_t button)
{
	assert(mouseButtons.count(button) == 1);

	return (mouseButtons[button] == Released);;
}

//Mouse Vectors
Vec2 InputManager::getMousePos() { return mousePos; }

Vec2 InputManager::getMouseDirection() { return mouseDirection; }

Vec2 InputManager::getMouseWheelDirection() { return mouseWheelDirection; }


//Game Controllers
void InputManager::checkForActiveControllers()
{
	for (int curJoy = 0; curJoy < SDL_NumJoysticks(); curJoy++)
	{
		if (SDL_IsGameController(curJoy))
		{

			gamepads.push_back(new Controller(curJoy));
			if (gamepads.back()->isValid())
			{
				Log::logI(gamepads.back()->getName() + " Connected.");
			}
			else
			{
				delete gamepads.back();
				gamepads.pop_back();
			}
		}
	}
}

bool InputManager::wasControllerButtonPressed(int controller, Controller::Button button)
{
	if (!isGamepadValid(controller))
	{
		//Log::logW("Invalid Gamepad requested");
		return false;
	}

	return gamepads[controller]->wasButtonPressed(button);
}

bool InputManager::isControllerButtonHeld(int controller, Controller::Button button)
{
	if (!isGamepadValid(controller))
	{
		//Log::logW("Invalid Gamepad requested");
		return false;
	}

	return gamepads[controller]->isButtonHeld(button);
}

bool InputManager::wasControllerButtonReleased(int controller, Controller::Button button)
{
	if (!isGamepadValid(controller))
	{
		//Log::logW("Invalid Gamepad requested");
		return false;
	}

	return gamepads[controller]->wasButtonReleased(button);
}

float InputManager::getControllerAxis1D(int controller, Controller::Axis1D axis)
{
	if (!isGamepadValid(controller))
	{
		//Log::logW("Invalid Gamepad requested");
		return 0.0f;
	}

	return gamepads[controller]->getAxis1D(axis);
}

Vec2 InputManager::getControllerAxis2D(int controller, Controller::Axis2D axis)
{
	if (!isGamepadValid(controller))
	{
		//Log::logW("Invalid Gamepad requested");
		return Vec2(0.0f);
	}

	return gamepads[controller]->getAxis2D(axis);
}

void InputManager::playControllerRumble(int controller, float strength, uint32_t lengthMS)
{
	if (isGamepadValid(controller))
	{
		gamepads[controller]->rumblePlay(strength, lengthMS);
	}
}

void InputManager::stopControllerRumble(int controller)
{
	if (isGamepadValid(controller))
	{
		gamepads[controller]->rumbleStop();
	}
}

uint8_t InputManager::getNumControllers()
{
	uint8_t controllers = 0;

	for (unsigned int curPad = 0; curPad < gamepads.size(); curPad++)
	{
		if (isGamepadValid(curPad))
		{
			controllers++;
		}
	}

	return controllers;
}

bool InputManager::isControllerAxisInUse(int controller, Controller::Axis2D axis)
{
	Vec2 value = getControllerAxis2D(controller, axis);

	//check if the axis is over the deadzone in any direction
	return (value.x <= -deadzone || value.x >= deadzone || value.y <= -deadzone || value.y >= deadzone);
}

void InputManager::setDeadZone(float newDeadzone)
{
	deadzone = newDeadzone;
}


//Event Processing
void InputManager::processKeyEvent(SDL_Event& e)
{
	ButtonState eventType = Unknown;

	switch (e.key.type)
	{
	case SDL_KEYDOWN:
		eventType = Pressed;
		break;
	case SDL_KEYUP:
		eventType = Released;
		break;
	}

	assert(eventType != Unknown);

	assert(keys.count(e.key.keysym.sym) == 1);

	keys[e.key.keysym.sym] = eventType;
}

void InputManager::processMouseEvent(SDL_Event& e)
{
	ButtonState buttonState = Unknown;

	switch (e.type)
	{
	case SDL_MOUSEBUTTONDOWN:
		buttonState = Pressed;
		break;

	case SDL_MOUSEBUTTONUP:
		buttonState = Released;
		break;

	case SDL_MOUSEMOTION:
		mousePos = { (float)e.motion.x, (float)e.motion.y };
		mouseDirection = { (float)e.motion.xrel, (float)e.motion.yrel };
		return;
		break;

	case SDL_MOUSEWHEEL:
		mouseWheelDirection = { (float)e.wheel.x, (float)e.wheel.y };
		return;
		break;

	default:
		Log::logE("Unhandled event passed through to processMouseEvent");
		assert(false);
		break;
	}

	assert(buttonState != Unknown);

	assert(mouseButtons.count(e.button.button) == 1);

	mouseButtons[e.button.button] = buttonState;
}

void InputManager::processGameControllerEvent(SDL_Event& e)
{
	Sint32 joyInstanceID = getJoystickInstanceIDFromEvent(e);

	switch (e.type)
	{
	case SDL_CONTROLLERAXISMOTION:
	case SDL_CONTROLLERBUTTONDOWN:
	case SDL_CONTROLLERBUTTONUP:

		for (unsigned int curPad = 0; curPad < gamepads.size(); curPad++)
		{
			if (isGamepadValid(curPad) &&
				gamepads[curPad]->getJoystickInstanceID() == (SDL_JoystickID)joyInstanceID)
			{
				gamepads[curPad]->processEvents(e);
			}
		}

		break;

	case SDL_CONTROLLERDEVICEADDED:
		addController(e);
		break;

	case SDL_CONTROLLERDEVICEREMOVED:
		removeController(e);
		break;
	}
}


void InputManager::update()
{
	//Keyboard
	for (auto& key : keys)
	{
		ButtonState keyState = key.second;

		if (keyState == Pressed) {
			key.second = Held;
		}
		else if (keyState == Released) {
			key.second = None;
		}
	}

	//Mouse
	for (auto& button : mouseButtons)
	{
		ButtonState buttonState = button.second;

		if (buttonState == Pressed) {
			button.second = Held;
		}
		else if (buttonState == Released) {
			button.second = None;
		}
	}

	mouseDirection = { 0 };
	mouseWheelDirection = { 0 };

	//Gamepads
	for (unsigned int curPad = 0; curPad < gamepads.size(); curPad++)
	{
		if (isGamepadValid(curPad))
		{
			gamepads[curPad]->update();
		}
	}

}

void InputManager::printDebugInfo()
{
	//Mouse Tests
	if (InputManager::wasMouseButtonPressed(SDL_BUTTON_LEFT)) {
		Log::logI("Mouse Pos: " + Utility::vec2ToString(InputManager::getMousePos()));
	}
	if (InputManager::wasMouseButtonPressed(SDL_BUTTON_MIDDLE)) {
		Log::logI("Mouse Dir: " + Utility::vec2ToString(InputManager::getMouseDirection()));
	}
	if (InputManager::wasMouseButtonPressed(SDL_BUTTON_RIGHT)) {
		Log::logI("Mouse wheel: " + Utility::vec2ToString(InputManager::getMouseWheelDirection()));
	}

	//Controller Tests
	if (InputManager::wasControllerButtonPressed(0, Controller::A))
	{
		Log::logI("LeftAxis: " +
			Utility::vec2ToString(InputManager::getControllerAxis2D(0, Controller::LeftStick)));
		Log::logI("RightAxis: " +
			Utility::vec2ToString(InputManager::getControllerAxis2D(0, Controller::RightStick)));
		Log::logI("LeftTrigger: " +
			Utility::floatToString(InputManager::getControllerAxis1D(0, Controller::LeftTrigger)));
		Log::logI("RightTrigger: " +
			Utility::floatToString(InputManager::getControllerAxis1D(0, Controller::RightTrigger)));
	}

	//Rumble Tests
	if (InputManager::wasControllerButtonPressed(0, Controller::RIGHTSHOULDER))
	{
		InputManager::playControllerRumble(0, 1.0f, 2000);
	}
}


bool InputManager::isGamepadValid(int controller)
{
	return (controller < (int)gamepads.size() &&
		gamepads[controller] != nullptr &&
		gamepads[controller]->isValid()
		);
}

SDL_JoystickID InputManager::getJoystickInstanceIDFromEvent(SDL_Event& e)
{
	SDL_JoystickID joyID;

	switch (e.type)
	{
	case SDL_CONTROLLERAXISMOTION:
		joyID = e.caxis.which;
		break;

	case SDL_CONTROLLERBUTTONDOWN:
	case SDL_CONTROLLERBUTTONUP:
		joyID = e.cbutton.which;
		break;

	case SDL_CONTROLLERDEVICEADDED:
	case SDL_CONTROLLERDEVICEREMOVED:
	case SDL_CONTROLLERDEVICEREMAPPED:
		joyID = e.cdevice.which;
		break;

	default:
		joyID = -1;
		break;
	}

	return joyID;
}

void InputManager::addController(SDL_Event& e)
{
	//Check if not already added by joystickID (Not instance ID).
	int joystickID = e.cdevice.which;
	int firstFreeJoy = -1;

	for (unsigned int curJoy = 0; curJoy < gamepads.size(); curJoy++)
	{
		if (isGamepadValid(curJoy))
		{
			if (gamepads[curJoy]->getJoystickID() == joystickID)
			{
				return;
			}
		}
		else
		{
			//Only set this variable if it hasn't been set already
			firstFreeJoy = (firstFreeJoy == -1 ? curJoy : firstFreeJoy);
		}
	}


	//If no old controllers to replace create a new one if we haven't reached the max
	if (gamepads.size() < MAX_GAMEPADS)
	{
		addNewController(joystickID, firstFreeJoy);
	}

}

void InputManager::addNewController(int joystickID, int arrayPos)
{
	if (SDL_IsGameController(joystickID))
	{

		if (arrayPos == -1)
		{
			gamepads.push_back(new Controller(joystickID));
			arrayPos = gamepads.size() - 1;
		}
		else
		{
			if (gamepads[arrayPos] != nullptr)
			{
				delete gamepads[arrayPos];
			}

			gamepads[arrayPos] = new Controller(joystickID);
		}


		if (gamepads[arrayPos]->isValid())
		{
			Log::logI(gamepads[arrayPos]->getName() + " Connected.");
		}
		else
		{
			delete gamepads[arrayPos];
		}
	}
}

void InputManager::removeController(SDL_Event& e)
{
	for (unsigned int curPad = 0; curPad < gamepads.size(); curPad++)
	{
		if (isGamepadValid(curPad) &&
			gamepads[curPad]->getJoystickInstanceID() == (SDL_JoystickID)e.cdevice.which)
		{
			Log::logI(gamepads[curPad]->getName() + " Disconnected");
			delete gamepads[curPad];
			gamepads[curPad] = nullptr;
		}
	}
}
