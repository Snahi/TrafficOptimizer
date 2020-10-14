#include "../pch.h"
#include "SimplifiedLightsSystem.h"



using namespace std;



// constructors ///////////////////////////////////////////////////////////////////////////////////



SimplifiedLightsSystem::SimplifiedLightsSystem(int id, 
	vector<SimplifiedLightsGroup*>& lightsGroups, vector<SimplifiedLights*>& lights)
{
	this->id = id;
	this->lightsGroups = lightsGroups;
	this->lights = lights;

	this->lightsGroups.front()->start(0);
	currGroupIdx = 0;
}



// methods ////////////////////////////////////////////////////////////////////////////////////////



void SimplifiedLightsSystem::deleteLightsAndLightsGroups()
{
	for (SimplifiedLightsGroup* pGroup : lightsGroups)
		delete pGroup;

	for (SimplifiedLights* pLights : lights)
		delete pLights;
}



void SimplifiedLightsSystem::tick(double currTime)
{
	if (lightsGroups[currGroupIdx]->finished(currTime)) // if current group finished
	{
		currGroupIdx = (currGroupIdx + 1) % lightsGroups.size(); // increment current group idx
		lightsGroups[currGroupIdx]->start(currTime); // start the new current group
	}
}



void SimplifiedLightsSystem::setPhase(double phase)
{
	// obtain total groups duration
	double totalGroupsDuration = 0.0;
	int numOfGroups = lightsGroups.size();
	for (int i = 0; i < numOfGroups; ++i)
		totalGroupsDuration += lightsGroups[i]->getDuration();

	if (totalGroupsDuration > 0.0)
	{
		int fullCycles = (int)(phase / totalGroupsDuration);
		double phaseNorm = phase - (fullCycles * totalGroupsDuration);

		double timeLeft = phaseNorm;
		int groupIdx = 0;
		while (timeLeft >= 0.0)
		{
			timeLeft -= lightsGroups[groupIdx]->getDuration();
			if (timeLeft >= 0.0)
				++groupIdx;
		}

		double timeInGroup = timeLeft + lightsGroups[groupIdx]->getDuration();

		this->currGroupIdx = groupIdx;

		lightsGroups[groupIdx]->start(0.0 - timeInGroup);
	}
}



void SimplifiedLightsSystem::reset()
{
	currGroupIdx = 0;
	lightsGroups[currGroupIdx]->start(0.0);
}



double SimplifiedLightsSystem::getMaxTotalDuration()
{
	double totMaxDuration = 0.0;
	int numOfLightsGroups = lightsGroups.size();
	for (int i = 0; i < numOfLightsGroups; ++i)
		totMaxDuration += lightsGroups[i]->getMaxDuration();

	return totMaxDuration;
}



int SimplifiedLightsSystem::getId()
{
	return id;
}