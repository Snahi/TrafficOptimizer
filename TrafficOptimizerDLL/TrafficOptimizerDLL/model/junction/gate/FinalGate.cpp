#include "../../../pch.h"
#include "FinalGate.h"
#include "FinalGate.h"



// constructors ///////////////////////////////////////////////////////////////////////////////////



FinalGate::FinalGate(FinishedCarListener* pListener)
{
	this->pListener = pListener;
}




// methods ////////////////////////////////////////////////////////////////////////////////////////



bool FinalGate::isGateOpen(Car* pCar)
{
	return true;
}



bool FinalGate::canEnter(Car* pCar)
{
	return true;
}



int FinalGate::enter(Car* pCar)
{
	if (pListener != nullptr)
		pListener->carFinished(pCar);

	return ENTER_SUCCESS;
}
