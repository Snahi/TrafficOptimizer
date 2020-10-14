#pragma once
#include "Gate.h"



/*
	Gate which will never let any car through.
*/
class AlwaysClosedGate :
	public Gate
{
public:
	AlwaysClosedGate() : Gate() {};
	AlwaysClosedGate(Road* pRoad) : Gate(pRoad) {};
	bool isGateOpen(Car* pCar)
	{
		return false;
	};
};

