#pragma once
#include "Lights.h"
#include "LightsGroup.h"
#include <vector>
// const //////////////////////////////////////////////////////////////////////////////////////////
// lighsts system
#define TICK_LIGHTS_SYSTEM_SUCCESS 0

/*
	Class that represents all lights at on a single junction. How to create the class:
	1. add all lights
	2. add lights groups in the order they occur
*/
class LightsSystem
{
public:
	// constructors ///////////////////////////////////////////////////////////////////////////////
	LightsSystem();
	~LightsSystem();
	// methods ////////////////////////////////////////////////////////////////////////////////////
	int tick(double duration);
	void addLights(Lights* pLights);
	/*
		Adds the new lights group at the end of the lightsGroups vector and updates states
		of all lights accordingly.
		Returns:
			true	- success
			false	- the group does not cover all lights
	*/
	bool addLightsGroup(LightsGroup* pGroup);
	void updateLightsStates(LightsGroup* pGroup);
	/*
		Checks whether every light in this lights system is covered by the pGroup
	*/
	bool lightsGroupCoversAllLights(LightsGroup* pGroup);
	/*
		Resets all lights and reloads their lightsGroups
	*/
	void reset();
	bool hasLights(Lights* pLights);
	bool hasLights();
	bool hasLightsGroups();
	/*
		Ticks the lights until offset is reached. It is not a permanent offset, it must be called
		after adding or changing lights groups whenever it happens, because only then it will
		have any effect.
	*/
	void setOffset(double offset);
	double getTotalMaxLightsGroupsDuration();

private:
	// fields /////////////////////////////////////////////////////////////////////////////////////
	std::vector<Lights*> lights;
	std::vector<LightsGroup*> lightsGroups;
};
