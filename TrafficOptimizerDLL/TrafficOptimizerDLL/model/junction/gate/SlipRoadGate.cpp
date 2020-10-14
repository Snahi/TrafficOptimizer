#include "../../../pch.h"
#include "SlipRoadGate.h"



// constructors ///////////////////////////////////////////////////////////////////////////////////



SlipRoadGate::SlipRoadGate(Road* pRoad, double joinPos, double slipRoadLen) 
	: Gate(pRoad, pRoad->speedLimitAt(joinPos))
{
	this->joinPos = joinPos;
	this->joinEnd = joinPos + slipRoadLen;
}



// methods ////////////////////////////////////////////////////////////////////////////////////////



bool SlipRoadGate::canEnter(Car* pCar)
{
	return pCar->getSpeed() <= getSpeedLimit() && getRoad()->canPutCarInSection(pCar, joinPos, joinEnd);
}



int SlipRoadGate::enter(Car* pCar)
{
	return getRoad()->putCarInSection(pCar, joinPos, joinEnd) == true ? 
		ENTER_SUCCESS : ENTER_CANNOT_ENTER_ROAD;
}



bool SlipRoadGate::isGateOpen(Car* pCar)
{
	return getRoad()->canPutCarInSection(pCar, joinPos, joinEnd);
}