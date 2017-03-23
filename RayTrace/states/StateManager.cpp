#include "StateManager.h"

#include "State.h"
#include "../misc/Utility.h"
#include "../misc/Log.h"

StateManager::StateManager(int windowWidth, int windowHeight)
	: WINDOW_WIDTH(windowWidth), WINDOW_HEIGHT(windowHeight)
{

}

StateManager::~StateManager()
{
	clearStates();
}

void StateManager::clearStates()
{
	for (auto s : states)
	{
		delete s;
	}
	states.clear();
}

void StateManager::addState(State* state)
{
	states.push_back(state);
	Log::logI("StateManager: Switched to the " + state->getStateName());
}

void StateManager::prepareToChangeState()
{
	Utility::Timer::cleanup();
}

void StateManager::changeState(State* state)
{
	clearStates();
	addState(state);
}

void StateManager::popLastState()
{
	delete states.back();
	states.pop_back();
}

bool StateManager::eventHandler()
{
	return states.back()->eventHandler();
}

void StateManager::update()
{
	states.back()->update();
}

void StateManager::render()
{
	for (int i = 0; i < (int)states.size(); i++)
	{
		states[i]->render();
	}
}