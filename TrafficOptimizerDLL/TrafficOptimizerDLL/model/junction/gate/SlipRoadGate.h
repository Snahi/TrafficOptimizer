#pragma once
#include "Gate.h"



/*
						|            |\
						|     |        \
						|           |   \
						|     |          \  _
						|           |     |  |
						|     |           |  |
						|           |     |  |-- slipRoadLen
						|     |     |     |  |
		    joinPos --> |           |     | _|
						|     |     |\     \
						|           | \     \
						|     |     |  \     \

	Simulating it in a similar way to the real life behaviour would be very complex, so it is
	simplified. It will just try to fit the car anywhere on the length corresponding to the
	slipRoadLen and if there isn't any gap big enough (also restrictions will be losened a little),
	then the cars on the guarded road will be rearranged so that there is enough space. Also, time
	spent on the slip road is not counted, but it can be done by connecting it with an additional
	road before, e.g. of half the length of the slip road so that the time would be on average like
	that.
*/
class SlipRoadGate : public Gate
{
public:
	/*
		@pRoad			- guarded road (road on which cars will enter)
		@joinPos		- postion on GUARDED road where slip road joins the road for the first time
		@slipRoadLen	- length of the slip road
		@slipRoadSpeed	- spped which cars will try to maintain on the slip road
	*/
	SlipRoadGate(Road* pRoad, double joinPos, double slipRoadLen);
	bool canEnter(Car* pCar) override;
	int enter(Car* pCar) override;
	bool isGateOpen(Car* pCar) override;

private:
	// fields /////////////////////////////////////////////////////////////////////////////////////
	double joinPos;
	double joinEnd;
};