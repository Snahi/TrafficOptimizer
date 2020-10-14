#include "../../../pch.h"
#include "Lights.h"
#include <limits>



// init ///////////////////////////////////////////////////////////////////////////////////////////



Lights::Lights(LightsState initialState)
{
	setToInitialState(initialState);
}



// methods ////////////////////////////////////////////////////////////////////////////////////////



void Lights::setToInitialState(LightsState initialState)
{
	stateIdx = 0;
	timeLeftInState = DBL_MAX;
	StateDuration state(initialState, -1.0);
	statesCycle.push_back(state);
}



int Lights::tick(double duration)
{
	timeLeftInState -= duration;
	if (timeLeftInState <= 0)
	{
		stateIdx = (stateIdx + 1) % statesCycle.size();
		timeLeftInState = statesCycle[stateIdx].duration;
	}

	return TICK_LIGHTS_SUCCESS;
}



bool Lights::addState(LightsState state, double duration)
{
	if (duration < 0)
		return false;

	// if first user defined state is added delete the default one
	if (statesCycle.size() > 0 && statesCycle[0].duration < 0.0)
		statesCycle.erase(statesCycle.begin());

	if (statesCycle.size() > 0)
	{
		int lastStateIdx = statesCycle.size() - 1;
		StateDuration& lastState = statesCycle[lastStateIdx];
		
		if (lastState.state == state) // if the last state is the same as the new one
			lastState.duration += duration;	// just make it last longer
		else // the new state differs from the last state
		{
			StateDuration newState(state, duration); // create a new state
			statesCycle.push_back(newState);
		}
	}
	else // there are no states defined for the Lights
	{
		StateDuration sd(state, duration);
		statesCycle.push_back(sd);
		stateIdx = 0;
		timeLeftInState = duration;
	}

	return true;
}



void Lights::reset()
{
	statesCycle.clear();
	setToInitialState(LightsState::RED);
}



bool Lights::isGreen()
{
	return statesCycle[stateIdx].state == LightsState::GREEN;
}



bool Lights::isRed()
{
	return statesCycle[stateIdx].state == LightsState::RED;
}



double Lights::getTimeLeftInState()
{
	return timeLeftInState;
}
