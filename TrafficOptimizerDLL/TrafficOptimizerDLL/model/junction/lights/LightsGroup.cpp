#include "../../../pch.h"
#include "LightsGroup.h"

using namespace std;



// constructors /////////////////////////////////////////////////////////////////////////////////



LightsGroup::LightsGroup(double duration, double minDuration, double maxDuration)
{
	this->duration = duration;
	this->minDuration = minDuration;
	this->maxDuration = maxDuration;
}



LightsGroup::LightsGroup(double duration, vector<Lights*> greenLights, vector<Lights*> redLights, 
	double minDuration, double maxDuration)
	: LightsGroup(duration)
{
	this->greenLights = greenLights;
	this->redLights = redLights;
	this->minDuration = minDuration;
	this->maxDuration = maxDuration;
}



// methods /////////////////////////////////////////////////////////////////////////////////////



void LightsGroup::addGreenLights(Lights* pLights)
{
	allLights.push_back(pLights);
	greenLights.push_back(pLights);
}



void LightsGroup::addRedLights(Lights* pLights)
{
	allLights.push_back(pLights);
	redLights.push_back(pLights);
}



bool LightsGroup::hasGreenLights(Lights* pLights)
{
	for (Lights* pGreen : greenLights)
	{
		if (pGreen == pLights)
			return true;
	}

	return false;
}



bool LightsGroup::hasLights()
{
	return allLights.size() > 0;
}



// getters & setters ///////////////////////////////////////////////////////////////////////////



int LightsGroup::setDuration(double duration)
{
	if (duration < minDuration)
		return SET_DURATION_TOO_SHORT;
	if (duration > maxDuration)
		return SET_DURATION_TOO_LONG;
	else
	{
		this->duration = duration;
		return SET_DURATION_SUCCESS;
	}
}



double LightsGroup::getDuration()
{
	return duration;
}



vector<Lights*>& LightsGroup::getGreenLights()
{
	return greenLights;
}



vector<Lights*>& LightsGroup::getRedLights()
{
	return redLights;
}



vector<Lights*>& LightsGroup::getAllLights()
{
	return allLights;
}



double LightsGroup::getMinDuration()
{
	return minDuration;
}



void LightsGroup::setMinDuration(double minDuration)
{
	this->minDuration = minDuration;
}



double LightsGroup::getMaxDuration()
{
	return maxDuration;
}



void LightsGroup::setMaxDuration(double maxDuration)
{
	this->maxDuration = maxDuration;
}








