#pragma once
#include "SimplifiedLights.h"
#include "SimplifiedLightsGroup.h"
#include <vector>



/*
	Gathers lights groups and manages them. Lights are stored only for further deletion (called
	from outside, deleteLightsAndLightsGroups())
*/
class SimplifiedLightsSystem
{
public:
	// constructors ///////////////////////////////////////////////////////////////////////////////
	SimplifiedLightsSystem(int id, std::vector<SimplifiedLightsGroup*>& lightsGroups,
		std::vector<SimplifiedLights*>& lights);

	// methods ////////////////////////////////////////////////////////////////////////////////////
	void tick(double currTime);
	void setPhase(double phase);
	void reset();
	double getMaxTotalDuration();
	void deleteLightsAndLightsGroups();
	int getId();

private:
	int id;
	std::vector<SimplifiedLightsGroup*> lightsGroups;
	std::vector<SimplifiedLights*> lights;
	int currGroupIdx;
};
