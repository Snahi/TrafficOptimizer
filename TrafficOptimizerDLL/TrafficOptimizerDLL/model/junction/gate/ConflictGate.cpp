#include "../../../pch.h"
#include "ConflictGate.h"
#include "../../MovementFunctions.h"



// methods ////////////////////////////////////////////////////////////////////////////////////////



bool ConflictGate::isGateOpen(Car* pCar)
{
	Car* pFirstCar = nullptr;
	double firstCarDist = 0.0;
	double carWithRightOfWayArrivalTime = 0.0;
	// when pCar will arrive at the gate
	double carArrivalTime = mov::time(pCar->getSpeed(), pCar->distanceToEndOfRoad(), 
		pCar->getAcc());

	int numOfConflicts = conflicts.size();
	for (int i = 0; i < numOfConflicts; i++)
	{
		pFirstCar = firstCar(conflicts[i], &firstCarDist);
		if (pFirstCar != nullptr)
		{
			carWithRightOfWayArrivalTime = mov::time(pFirstCar->getSpeed(), firstCarDist, 
				pFirstCar->getAcc());

			if (carWithRightOfWayArrivalTime <= conflicts[i].requiredTime + carArrivalTime)
				return false;
		}
	}

	return true;
}



Car* ConflictGate::firstCar(Conflict& conflict, double* pDistance)
{
	double distance = 0.0;
	vector<Road*>& roads = conflict.roads; // has at least 1 element
	Road* pRoad = nullptr;
	Car* pFirstCar = nullptr;

	int size = roads.size();
	double rangeStart = size == 1 ? conflict.startPosition : 0.0;
	pFirstCar = roads.front()->getFirstCarInRange(rangeStart, conflict.conflictPosition);
	if (pFirstCar == nullptr)
		distance += conflict.conflictPosition - rangeStart;
	else
		distance += conflict.conflictPosition - pFirstCar->getPosition();

	for (int i = 1; i < size && pFirstCar == nullptr; i++)
	{
		pRoad = roads[i];

		if (i != size - 1) // not last road
		{
			pFirstCar = pRoad->getFirstCar();
			if (pFirstCar != nullptr)
				distance += pRoad->getLength() - pFirstCar->getPosition();
			else
				distance += pRoad->getLength();
		}
		else // last road
		{
			pFirstCar = pRoad->getFirstCarAfter(conflict.startPosition);
			if (pFirstCar != nullptr)
				distance += pRoad->getLength() - pFirstCar->getPosition();
		}
	}

	*pDistance = distance;
	return pFirstCar;
}



int ConflictGate::addConflict(std::vector<Road*> roads, double startPosition, 
	double requiredTime, double conflictPosition)
{
	if (roads.empty()) return ADD_CONFLICT_EMPTY_CONFLICT_ROADS;
	if (startPosition < 0) return ADD_CONFLICT_NEGATIVE_START_POSITION;
	if (requiredTime < 0) return ADD_CONFLICT_NEGATIVE_REQUIRED_TIME;
	if (conflictPosition < 0) return ADD_CONFLICT_NEGATIVE_CONFLICT_POSITION;

	Conflict conflict(roads, startPosition, requiredTime, conflictPosition);
	conflicts.push_back(conflict);

	return ADD_CONFLICT_SUCCESS;
}
