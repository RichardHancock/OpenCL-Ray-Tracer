#pragma once

#include <SDL.h>
#include <string>

#include "../Platform.h"
#include "../input/InputManager.h"


class StateManager;

//Reference: Pete Allen's Game State Example
//Used in other assignments

/**
@class State
@brief A game state that contains one screen of the game such as a Menu or the game itself
*/
class State
{
public:

	/**
	@brief Create the State.
	
	@param [in,out] manager		    Pointer to the current state manager.
	@param [in,out] platform	    Pointer to platform.
	 */
	State(StateManager* manager, Platform* platform);

	virtual ~State() {}

	/**
	@brief Handles any events such as keyboard/mouse input
	@return Continue the game
	*/
	virtual bool eventHandler() = 0;

	/**
	@brief Update any internal values
	*/
	virtual void update() = 0;

	/**
	@brief Render any sprites relevant to the state
	*/
	virtual void render() = 0;

	/**
	@brief Get this states name
	@return std::string - Name
	*/
	std::string getStateName() { return stateName; }
protected:
	///Pointer to state manager
	StateManager* stateManager;

	///Platform
	Platform* platform;

	///This states name
	std::string stateName;
};