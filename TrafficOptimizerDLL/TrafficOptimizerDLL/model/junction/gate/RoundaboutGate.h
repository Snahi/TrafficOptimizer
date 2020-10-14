#pragma once
#include "Gate.h"
#include "../Roundabout.h"



// constants //////////////////////////////////////////////////////////////////////////////////////
#define ENTER_NOT_CONNECTED_TO_ROUNDABOUT 1123
#define ADDITIONAL_TIME_FOR_REACTIONS_FACTOR 6



/*
	Gate for entering a roundabout. After initializing it is required to call connectToRoundabout()
	before using it.
*/
class RoundaboutGate : public Gate
{
public:
	// constructors ///////////////////////////////////////////////////////////////////////////////
	RoundaboutGate(Roundabout* pRoundabout, double position);
	// methods ////////////////////////////////////////////////////////////////////////////////////
	virtual int enter(Car* pCar) override;
	virtual bool canEnter(Car* pCar) override;
	virtual bool isGateOpen(Car* pCar) override;
	/*
		Sets pRoundabout as pConnectedRoundabout if it was possible to add this gate to the 
		roundabout.
	*/
	bool connectToRoundabout();
	// getters && setters
	Roundabout* getRoundabout();

private:
	Roundabout* pRoundabout;
	Roundabout* pConnectedRoundabout;
	double position;
};
