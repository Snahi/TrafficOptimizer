#include "../../../pch.h"
#include "LightsSystem.h"


using namespace std;



// constructors ///////////////////////////////////////////////////////////////////////////////////



LightsSystem::LightsSystem()
{
	
}



// destructor /////////////////////////////////////////////////////////////////////////////////////



LightsSystem::~LightsSystem()
{
	int numOfLights = lights.size();
	for (int i = 0; i < numOfLights; i++)
		delete lights[i];

	int numOfLightsGroups = lightsGroups.size();
	for (int i = 0; i < numOfLightsGroups; i++)
		delete lightsGroups[i];
}



// methods ////////////////////////////////////////////////////////////////////////////////////////



int LightsSystem::tick(double duration)
{
	int numOfLights = lights.size();

	for (int i = 0; i < numOfLights; i++)
		lights[i]->tick(duration);

	return TICK_LIGHTS_SYSTEM_SUCCESS;
}



void LightsSystem::addLights(Lights* pLights)
{
	lights.push_back(pLights);
}



bool LightsSystem::addLightsGroup(LightsGroup* pGroup)
{
	if (lightsGroupCoversAllLights(pGroup))
	{
		lightsGroups.push_back(pGroup);
		
		updateLightsStates(pGroup);

		return true;
	}
	else
		return false;
}



void LightsSystem::updateLightsStates(LightsGroup* pGroup)
{
	double groupDuration = pGroup->getDuration();

	// update green
	vector<Lights*>& green = pGroup->getGreenLights();
	int numOfGreen = green.size();

	for (int i = 0; i < numOfGreen; i++)
		green[i]->addState(LightsState::GREEN, groupDuration);

	// update red
	vector<Lights*>& red = pGroup->getRedLights();
	int numOfRed = red.size();

	for (int i = 0; i < numOfRed; i++)
		red[i]->addState(LightsState::RED, groupDuration);
}



bool LightsSystem::lightsGroupCoversAllLights(LightsGroup* pGroup)
{
	vector<Lights*>& lightsInGroup = pGroup->getAllLights();
	int numOfLights = lights.size();

	for (int i = 0; i < numOfLights; i++)
	{
		if (find(lightsInGroup.begin(), lightsInGroup.end(), lights[i]) == lightsInGroup.end()) // check if ligthsGroup contains ligths[i]
			return false;
	}

	return true;
}



void LightsSystem::reset()
{
	for (Lights* pLights : lights)
		pLights->reset();

	for (LightsGroup* pGroup : lightsGroups)
		updateLightsStates(pGroup);
}



bool LightsSystem::hasLights(Lights* pLights)
{
	for (Lights* l : lights)
	{
		if (pLights == l)
			return true;
	}

	return false;
}



bool LightsSystem::hasLights()
{
	return lights.size() > 0;
}



bool LightsSystem::hasLightsGroups()
{
	return lightsGroups.size() > 0;
}



void LightsSystem::setOffset(double offset)
{
	double maxOffset = getTotalMaxLightsGroupsDuration();
	int maxOffset100 = (int) (maxOffset * 100);
	int offset100 = (int) (offset * 100);
	int offsetNormalized100 = offset100 % maxOffset100;
	double offsetNormalized = (double)offsetNormalized100 / 100;

	double time = 0.0;
	while (time <= offsetNormalized)
	{
		tick(0.1);
		++time;
	}
}



double LightsSystem::getTotalMaxLightsGroupsDuration()
{
	double totalDuration = 0.0;
	for (LightsGroup* pGroup : lightsGroups)
	{
		totalDuration += pGroup->getMaxDuration();
	}

	return totalDuration;
}

