#pragma once
#include "SimplifiedLights.h"
#include <vector>



/*
	Because lights can't work independently, e.g. some lights must be red when others are green,
	this class is introduced to solve that issue. Group gathers lights which dependencies must
	be preserved. In a group single lights can be either green or red. Group has duration and
	during that period all lights in that groups are either red or green (as specified during
	creation or updated later). This way lights work in a way which assures that not conflicting
	lights will be green at the same time. Actually, optimizer optimizes durations of lights 
	groups, not lights directly.
*/
class SimplifiedLightsGroup
{
public:
	// constructors ///////////////////////////////////////////////////////////////////////////////
	SimplifiedLightsGroup(int id, std::vector<SimplifiedLights*>& green, 
		std::vector<SimplifiedLights*>& red, double minDuration, double maxDuration);

	// methods ////////////////////////////////////////////////////////////////////////////////////
	void start(double currTime);
	bool finished(double currTime);
	void setDuration(double duration);
	double getDuration();
	double getMinDuration();
	double getMaxDuration();
	int getId();

private:
	// fields /////////////////////////////////////////////////////////////////////////////////////
	int id;
	std::vector<SimplifiedLights*> green;
	std::vector<SimplifiedLights*> red;
	double duration;
	double minDuration;
	double maxDuration;
	/*
		Time at which the group will be finished. It is set after start() is called.
	*/
	double finishTime;
};
