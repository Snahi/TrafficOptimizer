#include "../../pch.h"
#include "JunctionInput.h"



// destructor /////////////////////////////////////////////////////////////////////////////////////



JunctionInput::~JunctionInput()
{
	int numOfGates = gates.size();
	for (int i = 0; i < numOfGates; i++)
		delete gates[i];
}



// methods ////////////////////////////////////////////////////////////////////////////////////////



void JunctionInput::addGate(Gate* pGate)
{
	gates.push_back(pGate);
}



Gate* JunctionInput::gateToRoad(Road* pRoad)
{
	int size = gates.size();
	for (int i = 0; i < size; i++)
	{
		if (gates[i]->getRoad() == pRoad)
		{
			return gates[i];
		}
	}

	return nullptr;
}



// getters && setters /////////////////////////////////////////////////////////////////////////////



void JunctionInput::setInputRoad(Road* pInputRoad)
{
	this->pInputRoad = pInputRoad;
}



Road* JunctionInput::getInputRoad()
{
	return pInputRoad;
}



std::vector<Gate*>& JunctionInput::getAllGates()
{
	return gates;
}



bool JunctionInput::hasGates()
{
	return gates.size() > 0;
}