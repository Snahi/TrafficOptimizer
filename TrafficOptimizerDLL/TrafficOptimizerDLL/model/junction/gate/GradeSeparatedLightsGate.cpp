#include "../../../pch.h"
#include "GradeSeparatedLightsGate.h"
#include "../../MovementFunctions.h"



// constructors ///////////////////////////////////////////////////////////////////////////////////



GradeSeparatedLightsGate::GradeSeparatedLightsGate(Lights* pLights) : Gate()
{
	this->pLights = pLights;
}



GradeSeparatedLightsGate::GradeSeparatedLightsGate(Road* pRoad, Lights* pLights) : Gate(pRoad)
{
	this->pLights = pLights;
}



// methods ////////////////////////////////////////////////////////////////////////////////////////



bool GradeSeparatedLightsGate::isGateOpen(Car* pCar)
{
	double carDist = pCar->distanceToEndOfRoad();
	double timeOfCarArrival = mov::time(pCar->getSpeed(), carDist, pCar->getAcc());
	double requiredTime = timeOfCarArrival * SAFE_CROSSING_TIME_MULTIPLIER;

	if (pLights->isGreen())
	{
		if (requiredTime < pLights->getTimeLeftInState())
			return true;
	}

	return false;
}



// getters && setters /////////////////////////////////////////////////////////////////////////////



void GradeSeparatedLightsGate::setLights(Lights* pLights)
{
	this->pLights = pLights;
}