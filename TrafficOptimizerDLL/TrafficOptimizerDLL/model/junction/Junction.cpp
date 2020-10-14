#include "../../pch.h"
#include "Junction.h"



// constructor ////////////////////////////////////////////////////////////////////////////////////



Junction::Junction(LightsSystem* pLightsSystem)
{
	this->pLightsSystem = pLightsSystem;
}



// destructor /////////////////////////////////////////////////////////////////////////////////////



Junction::~Junction()
{
	int inputsSize = inputs.size();
	for (int i = 0; i < inputsSize; i++)
	{
		delete inputs[i];
	}

	if (pLightsSystem != nullptr)
		delete pLightsSystem;
}



// methods ////////////////////////////////////////////////////////////////////////////////////////



void Junction::addJunctionInput(JunctionInput* pInput)
{
	inputs.push_back(pInput);
}



// getters && setters /////////////////////////////////////////////////////////////////////////////



void Junction::setLightsSystem(LightsSystem* pLightsSystem)
{
	this->pLightsSystem = pLightsSystem;
}



bool Junction::hasInputs()
{
	return inputs.size() > 0;
}
