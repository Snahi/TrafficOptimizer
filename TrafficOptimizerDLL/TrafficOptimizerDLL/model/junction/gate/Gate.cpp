#include "../../../pch.h"
#include "Gate.h"
#include "../../MovementFunctions.h"


// init ////////////////////////////////////////////////////////////////////////////////////////////////



Gate::Gate()
{
	this->pRoad = nullptr;
	this->speedLimit = DEFAULT_SPEED_LIMIT;
}



Gate::Gate(Road* pRoad)
{
	this->pRoad = pRoad;
	if (pRoad != nullptr)
		this->speedLimit = pRoad->speedLimitAt(0.0);
	else
		this->speedLimit = DEFAULT_SPEED_LIMIT;
}



Gate::Gate(Road* pRoad, double speedLimit)
{
	this->pRoad = pRoad;
	this->speedLimit = speedLimit;
}



// methods ///////////////////////////////////////////////////////////////////////////////////////////////



bool Gate::canEnter(Car* pCar)
{
	if (pRoad != nullptr)
	{
		double delay = mov::time(pCar->getSpeed(), pCar->distanceToEndOfRoad(), pCar->getAcc());
		return isGateOpen(pCar) && (pCar->getSpeed() <= speedLimit) && pRoad->canAddCarOnStart(pCar, delay);
	}
	else  // no road, no enter
		return false;
}



int Gate::enter(Car* pCar)
{
	int res = ENTER_SUCCESS;

	if (canEnter(pCar))	// canEnter includes check for null pRoad
	{
		if (pRoad->addCarOnStart(pCar) != ADD_CAR_SUCCESS)
		{
			res = ENTER_CANNOT_ENTER_ROAD;
		}
	}
	else
	{
		res = ENTER_GATE_CLOSED;
	}

	return res;
}



// getters && setters //////////////////////////////////////////////////////////////////////////



void Gate::setRoad(Road* pRoad)
{
	this->pRoad = pRoad;
}



Road* Gate::getRoad()
{
	return this->pRoad;
}



double Gate::getSpeedLimit()
{
	return this->speedLimit;
}
