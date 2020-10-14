#pragma once
#include "Lights.h"
#include <vector>



// const //////////////////////////////////////////////////////////////////////////////////////////
#define DEFAULT_MIN_LIGHTS_GROUP_DURATION 5.0
#define DEFAULT_MAX_LIGHTS_GROUP_DURATION 120.0
// set duration
#define SET_DURATION_SUCCESS 0
#define SET_DURATION_TOO_SHORT -1
#define SET_DURATION_TOO_LONG 1


/*
	The class solves the problem of dependencies between lights, e.g. some lights must be red when
	others are green. This class groups the dependent lights together and assigns them to the 
	desired group (green/red) during the period specified. Then it is used during creation of a
	lights system which based on the added LightsGroup classes modifies states and their durations
	of all the lights in the LightsSystem.
*/
class LightsGroup
{
public:
	// constructors ///////////////////////////////////////////////////////////////////////////////
	LightsGroup(double duration, double minDuration = DEFAULT_MIN_LIGHTS_GROUP_DURATION, 
		double maxDuration = DEFAULT_MAX_LIGHTS_GROUP_DURATION);
	LightsGroup(double duration, std::vector<Lights*> greenLights, std::vector<Lights*> redLights, 
		double minDuration = DEFAULT_MIN_LIGHTS_GROUP_DURATION,
		double maxDuration = DEFAULT_MAX_LIGHTS_GROUP_DURATION);
	// methods ////////////////////////////////////////////////////////////////////////////////////
	void addGreenLights(Lights* pLights);
	void addRedLights(Lights* pLights);
	bool hasGreenLights(Lights* pLights);
	bool hasLights();
	// getters && setters /////////////////////////////////////////////////////////////////////////
	/*
		Returns:
			SET_DURATION_SUCCESS	- success
			SET_DURATION_TOO_SHORT	- duration is smaller than the minimum allowed duration
			SET_DURATION_TOO_LONG	- duration is greater than the maximum allowed duration
	*/
	int setDuration(double duration);
	double getDuration();
	std::vector<Lights*>& getGreenLights();
	std::vector<Lights*>& getRedLights();
	std::vector<Lights*>& getAllLights();
	double getMinDuration();
	void setMinDuration(double minDuration);
	double getMaxDuration();
	void setMaxDuration(double maxDuration);

private:
	double duration;
	double minDuration;
	double maxDuration;
	std::vector<Lights*> allLights;
	std::vector<Lights*> greenLights;
	std::vector<Lights*> redLights;
};
