#pragma once
#include "Gate.h"
#include <vector>
#include "../../Car.h"
#include "../../road/Road.h"


class Conflict;


// const //////////////////////////////////////////////////////////////////////////////////////////
#define ADD_CONFLICT_SUCCESS 0
#define ADD_CONFLICT_NEGATIVE_START_POSITION 2
#define ADD_CONFLICT_NEGATIVE_REQUIRED_TIME 3
#define ADD_CONFLICT_NEGATIVE_CONFLICT_POSITION 4
#define ADD_CONFLICT_EMPTY_CONFLICT_ROADS 5



/*
	Gate which guards entrance to a road which is not accessible directly - the car must cross
	other roads and it can be done only if it won't cause an accident
*/
class ConflictGate : public Gate
{
public:
	ConflictGate(Road* pRoad = nullptr) : Gate(pRoad) {};
	// methods ////////////////////////////////////////////////////////////////////////////////////
	bool isGateOpen(Car* pCar) override;
	int addConflict(std::vector<Road*> roads, double startPosition, double requiredTime, 
		double conflictPosition);

private:
	// fields /////////////////////////////////////////////////////////////////////////////////////
	std::vector<Conflict> conflicts;
	// methods ////////////////////////////////////////////////////////////////////////////////////
	/*
		Returns the first (closest to the conflict area) car on the conflicting road/roads
	*/
	Car* firstCar(Conflict& conflict, double* pDistance);
};



class Conflict
{
public:
	Conflict(std::vector<Road*> roads, double startPosition, double requiredTime, 
		double conflictPosition) : 
		roads{ roads }, startPosition{ startPosition }, requiredTime{ requiredTime },
		conflictPosition{ conflictPosition } {};
	/*
	     road2                  road1
	 ______|______ _______________|______________
	|             |                              |
	                               startPosition
		|    |    |                      |
		|       X | <-- goal             v
	 ___|    |    |_______________________________
	               
	 _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ 

	 ___           _______________________________
	    |    | __ |							
		|     |  ||
		|    ||  || <-- car
		|     |__||

		Typically the conflicting road will be conflicting just at it's beginning, because it 
		starts on a junction. This makes it hard to check whether the car can go through the road
		because there is no way to check if some other car arrives from the other road. Solution
		for that is storing the roads which are in "vison range" of the car and search for the 
		first (the closest to the conflicting area) car on these roads. the first road is the
		closest to the conflicting area, they are ordered.
	*/
	std::vector<Road*> roads;
	/*
		The smallest position on the last road on which the car can be
	*/
	double startPosition;
	/*
		Time from now for which there can't be any car on the crossed road
	*/
	double requiredTime;
	/*
		Position on the first road at which the conflict occurs
	*/
	double conflictPosition;
};
