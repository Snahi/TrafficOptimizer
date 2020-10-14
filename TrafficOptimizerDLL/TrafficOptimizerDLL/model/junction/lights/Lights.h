#pragma once
#include <vector>
// const //////////////////////////////////////////////////////////////////////////////////////////
// tick
#define TICK_LIGHTS_SUCCESS 0


class StateDuration;


enum class LightsState 
{
	GREEN,
	RED
};



/* 
	Class that represents traffic lights. The lights have only two states - green and red, the 
	yellow (only yellow, not red and yellow) state is included in the red state and the yellow-red 
	state is included in the green state.
*/
class Lights
{
public:
	// constructors ///////////////////////////////////////////////////////////////////////////////
	Lights(LightsState initialState = LightsState::RED);
	// methods ////////////////////////////////////////////////////////////////////////////////////
	int tick(double duration);
	/*
		Adds the state at the end of the cycle. If the state is the same as the current last state
		then they are merged (their time is combined) if the state is different than the last
		state then it's just added as it is passed to the function.
		Returns:
			true	- success
			false	- fail -> duration is <= 0
	*/
	bool addState(LightsState state, double duration);
	bool isGreen();
	bool isRed();
	double getTimeLeftInState();
	void setToInitialState(LightsState initialState);
	/*
		Erases all ligths groups and sets the obeject to its initial state
	*/
	void reset();

private:
	// fields /////////////////////////////////////////////////////////////////////////////////////
	std::vector<StateDuration> statesCycle;	// ordered states of the lights
	int stateIdx;							// idx of the current state
	double timeLeftInState;					// time left in the current state
};



/*
	Class that represents the state internally (gathers the state and it's duration)
*/
class StateDuration
{
public:
	StateDuration(LightsState state, double duration) : state(state), duration(duration) {};
	LightsState state;
	double duration;
};

