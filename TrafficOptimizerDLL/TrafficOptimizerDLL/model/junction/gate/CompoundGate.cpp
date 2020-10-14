#include "../../../pch.h"
#include "CompoundGate.h"



// destructor /////////////////////////////////////////////////////////////////////////////////////



CompoundGate::~CompoundGate()
{
	for (Gate* pComponentGate : componentGates)
		delete pComponentGate;
}



// methods ////////////////////////////////////////////////////////////////////////////////////////



bool CompoundGate::isGateOpen(Car* pCar)
{
	for (Gate* pComponentGate : componentGates)
	{
		if (!pComponentGate->isGateOpen(pCar))
			return false;
	}

	return true;
}



int CompoundGate::addGate(Gate* pComponentGate)
{
	if (pComponentGate->getRoad() == this->getRoad())
	{
		componentGates.push_back(pComponentGate);
		return CREATE_GATE_SUCCESS;
	}
	else
		return ADD_GATE_INCOMPATIBLE_ROAD;
}



// getters && setters /////////////////////////////////////////////////////////////////////////////



void CompoundGate::setRoad(Road* pRoad)
{
	for (Gate* pComponent : componentGates)
		pComponent->setRoad(pRoad);

	this->pRoad = pRoad;
}



vector<Gate*>& CompoundGate::getComponents()
{
	return componentGates;
}
