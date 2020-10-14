#include "../pch.h"
#include "SimplifiedLightsGroup.h"



using namespace std;



// constructors ///////////////////////////////////////////////////////////////////////////////////



SimplifiedLightsGroup::SimplifiedLightsGroup(int id, vector<SimplifiedLights*>& green, vector<SimplifiedLights*>& red,
	double minDuration,	double maxDuration)
{
	this->id = id;
	this->green = green;
	this->red = red;
	this->duration = (minDuration + maxDuration) / 2;
	this->finishTime = duration;
	this->minDuration = minDuration;
	this->maxDuration = maxDuration;
}



// methods ////////////////////////////////////////////////////////////////////////////////////////



void SimplifiedLightsGroup::start(double currTime)
{
	// set all green lights to green state
	int greenSize = green.size();
	for (int i = 0; i < greenSize; ++i)
		green[i]->setGreen();

	// set all red lights to red state
	int redSize = red.size();
	for (int i = 0; i < redSize; ++i)
		red[i]->setRed();

	finishTime = currTime + duration;
}



bool SimplifiedLightsGroup::finished(double currTime)
{
	return finishTime <= currTime;
}



void SimplifiedLightsGroup::setDuration(double duration)
{
	this->duration = duration;
}



double SimplifiedLightsGroup::getDuration()
{
	return duration;
}



double SimplifiedLightsGroup::getMinDuration()
{
	return minDuration;
}



double SimplifiedLightsGroup::getMaxDuration()
{
	return maxDuration;
}



int SimplifiedLightsGroup::getId()
{
	return id;
}


