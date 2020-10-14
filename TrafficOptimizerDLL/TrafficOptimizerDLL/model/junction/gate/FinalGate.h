#pragma once
#include "Gate.h"


class FinishedCarListener;

class FinalGate : public Gate
{
public:
	// constructors ///////////////////////////////////////////////////////////////////////////////
	FinalGate(FinishedCarListener* pListener);

	// methods ////////////////////////////////////////////////////////////////////////////////////
	bool isGateOpen(Car* pCar) override;
	bool canEnter(Car* pCar) override;
	int enter(Car* pCar) override;
	
private:
	FinishedCarListener* pListener;
};



class FinishedCarListener {
public:
	virtual void carFinished(Car* pCar) = 0;
};
