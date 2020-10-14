#include "../../../pch.h"
#include "RoundaboutGate.h"



// constructors ///////////////////////////////////////////////////////////////////////////////////



RoundaboutGate::RoundaboutGate(Roundabout* pRoundabout, double position)
	: Gate(pRoundabout->getMockRoad(), pRoundabout->getSpeed())
{
	this->position = position;
	this->pRoundabout = pRoundabout;
	this->pConnectedRoundabout = nullptr;
}



// methods ////////////////////////////////////////////////////////////////////////////////////////



bool RoundaboutGate::connectToRoundabout()
{
	if (pRoundabout->addInput(position) == ADD_INPUT_SUCCESS)
	{
		pConnectedRoundabout = pRoundabout;
		return true;
	}

	return false;
}



bool RoundaboutGate::canEnter(Car* pCar)
{
	return pConnectedRoundabout != nullptr && pConnectedRoundabout->canPutCar(pCar) && 
		pCar->getSpeed() <= getSpeedLimit();
}



int RoundaboutGate::enter(Car* pCar)
{
	int res = ENTER_SUCCESS;

	if (pConnectedRoundabout != nullptr)
	{
		Road* pRoadAfterRoundabout = pCar->getRoadAtPosition(pCar->getRoadPos() + 2);
		CarOnRoundabout car(pCar, pConnectedRoundabout->getTimeToOutput(position, pRoadAfterRoundabout) + 
			ADDITIONAL_TIME_FOR_REACTIONS_FACTOR * pCar->getReactionTime(),
			pConnectedRoundabout->getOutputGate(pRoadAfterRoundabout));

		if (!pConnectedRoundabout->putCar(car)) // if didn't enter
			res = ENTER_CANNOT_ENTER_ROAD;
	}
	else
		res = ENTER_NOT_CONNECTED_TO_ROUNDABOUT;
	
	return res;
}



bool RoundaboutGate::isGateOpen(Car* pCar)
{
	return true;
}



// getters && setters /////////////////////////////////////////////////////////////////////////////



Roundabout* RoundaboutGate::getRoundabout()
{
	return pRoundabout;
}






