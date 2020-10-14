#pragma once
#include "Gate.h"
#include "../lights/Lights.h"
#include "../../Car.h"


// constants
#define SAFE_CROSSING_TIME_MULTIPLIER 2



/*
	Gate which is open when the corresponding traffic lights are green and will be green
	at the time when a car arrives. In other cases the gate is closed.
*/
class GradeSeparatedLightsGate :
	public Gate
{
public:
	GradeSeparatedLightsGate(Lights* pLights);
	GradeSeparatedLightsGate(Road* pRoad, Lights* pLights);
	bool isGateOpen(Car* pCar);
	// getters && setters
	void setLights(Lights* pLights);

private:
	Lights* pLights;
};

