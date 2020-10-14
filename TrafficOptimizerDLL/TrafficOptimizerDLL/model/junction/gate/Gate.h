#pragma once
#include "../../road/Road.h"
#include "../../Car.h"

// const //////////////////////////////////////////////////////////////////////////////////////////
#define ENTER_SUCCESS 0
#define ENTER_CANNOT_ENTER_ROAD 1
#define	ENTER_GATE_CLOSED 2
#define DEFAULT_SPEED_LIMIT 10

class Road;
class Car;

/*
	The gate class is responsible for guarding access to a road. It means that every car which 
	wants to enter a road should pass through a gate. Road can be guarded by multiple gates,
	but one gate can guard only one road.
	The class doesn't correspond to any real life object, but it facilitates coordination of
	entering a road by making it very flexible. For example gate can be always open or closed, but
	there may be other gates, which use traffic lights or check whether can enter the road based
	on other roads (there is no car coming), which can be useful when the car has to cross some
	roads in order to get to the guarded road.
*/
class Gate
{
public:
	// constructors ///////////////////////////////////////////////////////////////////////////////
	Gate();
	Gate(Road* pRoad);
	Gate(Road* pRoad, double speedLimit);
	// methods ////////////////////////////////////////////////////////////////////////////////////
	/*
		Checks if gate is open, if the car speed is not too big and if there is enough space on the
		guarded road and if yes, then puts the car on the road (but it doesn't update the state of
		the car, only the entered road is affected).
		Returns:
			ENTER_SUCCESS			- success
			ENTER_CANNOT_ENTER_ROAD - not enough space on the guarded road
			ENTER_GATE_CLOSED		- this gate is closed
	*/
	virtual int enter(Car* pCar);
	/*
		Checks if can enter through the gate. The check includes checking if this gate is open,
		checking if speed of the car is not too big and checking if there can enter the guarded
		road.
		Returns:
			true	- can enter
			false	- can't enter
	*/
	virtual bool canEnter(Car* pCar);
	// getters && setters
	virtual void setRoad(Road* pRoad);
	Road* getRoad();
	double getSpeedLimit();
	virtual bool isGateOpen(Car* pCar) = 0;

private:
	// fields /////////////////////////////////////////////////////////////////////////////////////
	double speedLimit;	// maximum speed allowed to cross the gate
protected:
	Road* pRoad;		// guarded road
};

