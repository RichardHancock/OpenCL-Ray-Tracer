#pragma once

#include <vector>
#include <unordered_map>
#include <SDL.h>

#include "../misc/Vec2.h"
#include "Controller.h"

/** @brief	Manager for keyboard, mouse and game controller inputs. */
class InputManager
{
public:
	/** @brief	Clean up memory used by Input Manager */
	static void cleanup();

	//Keyboard

	/**
	@brief	Was the specified key pressed.

	@param	key	The key code.

	@return	true if pressed, false if not.
	*/
	static bool wasKeyPressed(SDL_Keycode key);

	/**
	@brief	Is the specified key held.

	@param	key	The key code.

	@return	true if held, false if not.
	*/
	static bool isKeyHeld(SDL_Keycode key);

	/**
	@brief	Was the specified key released.

	@param	key	The key code.

	@return	true if released, false if not.
	*/
	static bool wasKeyReleased(SDL_Keycode key);


	//Quick Shortcuts.

	/**
	@brief	Quick shortcut function to check if the ctrl modifier key is held.

	@return	true if it pressed, false if not.
	*/
	static bool ctrl();

	/**
	@brief	Quick shortcut function to check if the alt modifier key is held.

	@return	true if it pressed, false if not.
	*/

	static bool alt();

	/**
	@brief	Quick shortcut function to check if the shift modifier key is held.

	@return	true if it pressed, false if not.
	*/
	static bool shift();


	//Mouse
	/**
	@brief Was mouse button pressed.
	One of SDL_BUTTON_LEFT/MIDDLE/RIGHT/X1/X2

	@param button The mouse button to check.

	@return true if pressed, false if not.
	*/
	static bool wasMouseButtonPressed(uint8_t button);

	/**
	@brief Is mouse button held.
	One of SDL_BUTTON_LEFT/MIDDLE/RIGHT/X1/X2

	@param button The mouse button to check.

	@return true if held, false if not.
	*/
	static bool isMouseButtonHeld(uint8_t button);

	/**
	@brief Was mouse button released.
	One of SDL_BUTTON_LEFT/MIDDLE/RIGHT/X1/X2

	@param button The mouse button to check.

	@return true if released, false if not.
	*/
	static bool wasMouseButtonReleased(uint8_t button);


	//Mouse Vectors

	/**
	@brief	Gets mouse position on the screen.

	@return	The mouse position.
	*/
	static Vec2 getMousePos();

	/**
	@brief	Gets mouse direction vec. The movement vector since the last mouse movement.

	@return	The mouse direction.
	*/
	static Vec2 getMouseDirection();

	/**
	@brief	Gets mouse wheel direction.

	@todo Give range of output in the docs
	@return	The mouse wheel direction.
	*/
	static Vec2 getMouseWheelDirection();


	// Game Controllers

	/** @brief	Check for already active controllers on setup. */
	static void checkForActiveControllers();

	/**
	@brief	Was the specified controller button pressed.

	@param	controller	The controller index.
	@param	button	  	The button enum.

	@return	true if pressed, false if not or controller doesn't exist.
	*/
	static bool wasControllerButtonPressed(int controller, Controller::Button button);

	/**
	@brief	Is the specified controller button held.

	@param	controller	The controller index.
	@param	button	  	The button enum.

	@return	true if held, false if not or controller doesn't exist.
	*/
	static bool isControllerButtonHeld(int controller, Controller::Button button);

	/**
	@brief	Was the specified controller button released.

	@param	controller	The controller index.
	@param	button	  	The button enum.

	@return	true if released, false if not or controller doesn't exist.
	*/
	static bool wasControllerButtonReleased(int controller, Controller::Button button);

	/**
	@brief	Gets the controller's requested 1 dimensional axis (Triggers).

	@param	controller	The controller index.
	@param	axis	  	The controller axis.

	@return	Float containing the controller's axis position.
	*/
	static float getControllerAxis1D(int controller, Controller::Axis1D axis);

	/**
	@brief	Gets the controller's requested axis 2 dimensional (Triggers).

	@param	controller	The controller index.
	@param	axis	  	The controller axis.

	@return	Vec2 containing the controller's axis position.
	*/
	static Vec2 getControllerAxis2D(int controller, Controller::Axis2D axis);

	/**
	@brief	Play a rumble effect on the specified controller.

	@param	controller	The controller index.
	@param	strength  	The strength of the rumble. Between 0-1.
	@param	lengthMS  	The length in milliseconds.
	*/
	static void playControllerRumble(int controller, float strength, uint32_t lengthMS);

	/**
	@brief	Stops a currently active rumble effect on the specified controller.

	@param	controller	The controller index.
	*/
	static void stopControllerRumble(int controller);

	/**
	@brief	Gets number of controllers detected.

	@return	The number of controllers.
	*/
	static uint8_t getNumControllers();


	static bool isControllerAxisInUse(int controller, Controller::Axis2D axis);

	static void setDeadZone(float newDeadzone);
	

	//Process Events

	/**
	@brief	Process SDL Keyboard event into internal arrays.

	@param [in,out]	e	The keyboard SDL_Event to process.
	*/
	static void processKeyEvent(SDL_Event& e);

	/**
	@brief	Process SDL Mouse event into internal arrays.

	@param [in,out]	e	The mouse SDL_Event to process.
	*/
	static void processMouseEvent(SDL_Event& e);

	/**
	@brief	Process SDL Game Controller event into internal arrays.

	@param [in,out]	e	The Game Controller SDL_Event to process.
	*/
	static void processGameControllerEvent(SDL_Event& e);

	/** @brief	Updates the internal variables. */
	static void update();


	static void printDebugInfo();

private:

	/** @brief	Values that represent all the possible button states for a controller. */
	enum ButtonState {
		Pressed,  ///< Button was pressed this frame
		Held,     ///< Button is held
		Released, ///< Button was release this frame
		None,     ///< Button has no active state
		Unknown   ///< An unknown button that can't be used
	};

	/** @brief	Map of keys and their states. */
	static std::unordered_map<SDL_Keycode, ButtonState> keys;

	/** @brief	Map of mouse buttons and their states. */
	static std::unordered_map<uint8_t, ButtonState> mouseButtons;

	/**
	@brief Vector containing all initialised controllers

	@todo Possibly switch to standard array.
	*/
	static std::vector<Controller*> gamepads;

	/** @brief	The maximum number of gamepads that can be handled. */
	static const unsigned int MAX_GAMEPADS = 4;

	static float deadzone;

	InputManager();


	//Mouse Vectors

	/** @brief	The mouse's current position this frame. */
	static Vec2 mousePos;

	/**
	@brief	The mouse's current direction this frame.
	The mouse direction is its' movement vector since the last frame.
	*/
	static Vec2 mouseDirection;

	/**
	@brief	The mouse wheel's current direction this frame.
	The mouse wheel's direction is its' movement vector since the last frame.

	@todo Give more detail about the numbers returned
	*/
	static Vec2 mouseWheelDirection;


	/**
	@brief	Queries if a gamepad stored in the array is valid.

	@param	controller	The controller index.

	@return	true if the gamepad is valid, false if not.
	*/
	static bool isGamepadValid(int controller);

	/**
	@brief	Finds out which specific controller is responsible for the passed in event.

	@param [in,out]	e	The SDL_Event to process.

	@return	The joystick instance identifier responsible.
	*/
	static SDL_JoystickID getJoystickInstanceIDFromEvent(SDL_Event& e);

	/**
	@brief	Adds a controller when one is detected from the passed in event.

	@param [in,out]	e	The SDL_Event to process.
	*/
	static void addController(SDL_Event& e);

	/**
	@brief	Adds a new controller to the internal arrays.

	@param	joystickID	Identifier for the joystick.
	@param	arrayPos  	The array position.
	*/
	static void addNewController(int joystickID, int arrayPos);

	/**
	@brief	Called when a controller is no longer detected.
	Currently just cleans up its memory and deletes

	@param [in,out]	e	The SDL_Event to process.
	*/
	static void removeController(SDL_Event& e);
};