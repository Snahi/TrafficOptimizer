#include "../../pch.h"
#include "FinalJunctionInput.h"



// constructors ///////////////////////////////////////////////////////////////////////////////



FinalJunctionInput::FinalJunctionInput(Gate* pFinalGate)
{
	this->pFinalGate = pFinalGate;
}



FinalJunctionInput::~FinalJunctionInput()
{
	if (pFinalGate != nullptr)
		delete pFinalGate;
}



// methods ////////////////////////////////////////////////////////////////////////////////////
Gate* FinalJunctionInput::gateToRoad(Road* pRoad)
{
	return pFinalGate;
}