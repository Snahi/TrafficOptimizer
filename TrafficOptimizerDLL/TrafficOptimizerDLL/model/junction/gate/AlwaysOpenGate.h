#pragma once
#include "Gate.h"
#include "../../Car.h"



/*
	Gate which will always let a car through provided that the guarded road allows it.
*/
class AlwaysOpenGate :
	public Gate
{
public:
	AlwaysOpenGate() : Gate() {};
	AlwaysOpenGate(Road* pRoad) : Gate(pRoad) {};
	bool isGateOpen(Car* pCar)
	{
		return true;
	};
};

