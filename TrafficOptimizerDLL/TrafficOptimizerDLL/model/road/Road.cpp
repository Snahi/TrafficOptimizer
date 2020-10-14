#include "../../pch.h"
#include "Road.h"
#include "../MovementFunctions.h"
#include <algorithm>

// function declarations ////////////////////////////////////////////////////////////////////////////////////////
bool areSpeedLimitsContinuous(vector<SpeedLimit>& speedLimits);
bool areSpeedLimitsCoveringWholeRoad(vector<SpeedLimit>& speedLimits, double roadLength);



// init /////////////////////////////////////////////////////////////////////////////////////////////////////////



Road::Road(double length, JunctionInput* pJunctionInput, vector<SpeedLimit> speedLimits)
	: length(length), pJunctionInput(pJunctionInput), speedLimits(speedLimits), pLeftNeighbour(nullptr),
	pRightNeighbour(nullptr) 
{
	initOptimalTime();
}



RoadCreationResult Road::getInstance(double length, JunctionInput* pJunctionInput, vector<SpeedLimit> speedLimits)
{	
	if (speedLimits.size() == 0)
	{
		return RoadCreationResult(nullptr, false, RoadCreationResult::NOT_ENOUGH_SPEED_LIMITS);
	}
	else if (!areSpeedLimitsContinuous(speedLimits))
	{
		return RoadCreationResult(nullptr, false, RoadCreationResult::NOT_CONTINUOUS_SPEED_LIMITS);
	}
	else if (length <= 0)
	{
		return RoadCreationResult(nullptr, false, RoadCreationResult::TOO_SHORT_LENGTH);
	}
	else if (pJunctionInput == nullptr)
	{
		return RoadCreationResult(nullptr, false, RoadCreationResult::NULL_JUNCTION_INPUT);
	}
	else if (!areSpeedLimitsCoveringWholeRoad(speedLimits, length))
	{
		return RoadCreationResult(nullptr, false, RoadCreationResult::NOT_WHOLE_ROAD_COVERED);
	}

	Road* pRoad = new Road(length, pJunctionInput, speedLimits);
	pJunctionInput->setInputRoad(pRoad);

	return RoadCreationResult(pRoad);
}



bool areSpeedLimitsContinuous(vector<SpeedLimit>& speedLimits)
{
	int size = speedLimits.size();
	if (size > 1)
	{
		SpeedLimit* prevLimit = nullptr;
		for (int i = 1; i < size; i++)
		{
			prevLimit = &speedLimits.at(i - 1);
			if (prevLimit->getEndPos() != (&speedLimits.at(i))->getStartPos())
			{
				return false;
			}
		}
	}

	return true;
}



bool areSpeedLimitsCoveringWholeRoad(vector<SpeedLimit>& speedLimits, double roadLength)
{
	if (speedLimits.at(0).getStartPos() != 0.0)
	{
		return false;
	}
	else if (speedLimits.at(speedLimits.size() - 1).getEndPos() != roadLength)
	{
		return false;
	}

	return true;
}



// methods /////////////////////////////////////////////////////////////////////////////////////////////////////////



void Road::reset()
{
	cars.clear();
}



int Road::addCarOnStart(Car* pCar)
{
	int res = ADD_CAR_SUCCESS;

	if (canAddCarOnStart(pCar))
	{
		cars.push_back(pCar);
	}
	else
	{
		res = ADD_CAR_NOT_ENOUGH_SPACE;
	}

	return res;
}



void Road::removeCar(Car* pCar)
{
	cars.remove(pCar);
}



bool Road::removeFirstCar()
{
	if (cars.size() > 0)
	{
		cars.erase(cars.begin());
		return true;
	}

	return false;
}



bool Road::canAddCarOnStart(Car* pCar)
{
	// if the car is going too fast prevent it from entering
	if (pCar->getSpeed() > speedLimits[0].getValue())
		return false;

	// check if there is enough space
	bool res = true;

	double carSpeed = pCar->getSpeed();
	double carStrongBrake = pCar->getStrongBrake();
	double brakingRoad = 0.0;
	Car* pLastCar = getLastCarInSection(0, length);
	
	if (pLastCar != nullptr) // if there is another car -> braking distance to that car
	{
		double lastCarSpeed = pLastCar->getSpeed();

		if (carSpeed - lastCarSpeed > 0) // if the next car drives slower
			brakingRoad = mov::brakingDistance(carSpeed, lastCarSpeed, carStrongBrake); // braking road to the next car;
		else // next car drives faster or with the same speed -> no need to brake
			brakingRoad = 0.0;
	}
	else // the road is empty
		brakingRoad = 0.0;

	double safeDistance = mov::distance(carSpeed, pCar->getAcc(), pCar->getTimeGap());
	double requiredSpace = pCar->getLength() + brakingRoad + safeDistance + pCar->getMinGap();

	if (pLastCar != nullptr)
	{
		double backOfLastCarPos = pLastCar->getBack();

		if (backOfLastCarPos <= requiredSpace)	// not enough space
			res = false;
	}
	else if (requiredSpace <= length)
		res = true;
	else // road is too short for the car to brake
		res = false;

	return res;
}



bool Road::canAddCarOnStart(Car* pCar, double delay)
{
	// if the car is going too fast prevent it from entering
	if (pCar->getSpeed() > speedLimits[0].getValue())
		return false;

	// check if there is enough space
	bool res = true;

	double carSpeed = pCar->getSpeed();
	double carStrongBrake = pCar->getStrongBrake();
	double brakingRoad = 0.0;
	Car* pLastCar = getLastCarInSection(0, length);

	if (pLastCar != nullptr) // if there is another car -> braking distance to that car
	{
		double lastCarSpeed = pLastCar->getSpeed();

		if (carSpeed - lastCarSpeed > 0) // if the next car drives slower
			brakingRoad = mov::brakingDistance(carSpeed, lastCarSpeed, carStrongBrake); // braking road to the next car;
		else // next car drives faster or with the same speed -> no need to brake
			brakingRoad = 0.0;
	}
	else // the road is empty
		brakingRoad = 0.0;

	double safeDistance = mov::distance(carSpeed, pCar->getAcc(), pCar->getTimeGap());
	double requiredSpace = pCar->getLength() + brakingRoad + safeDistance + pCar->getMinGap();

	if (pLastCar != nullptr)
	{
		double backOfLastCarPos = pLastCar->getBack();

		// calculate car position after the delay
		double backAfterDelay = backOfLastCarPos + (pLastCar->getSpeed() * delay);

		if (backAfterDelay <= requiredSpace)	// not enough space
			res = false;
	}
	else if (requiredSpace <= length)
		res = true;
	else // road is too short for the car to brake
		res = false;

	return res;
}



Car* Road::getLastCarInSection(double start, double end)
{
	Car* currCar = nullptr;
	double currCarBack = -1;
	double currCarFront = -1;
	int size = cars.size();

	for (list<Car*>::reverse_iterator rit = cars.rbegin(); rit != cars.rend(); ++rit)
	{
		currCar = *rit;
		currCarBack = currCar->getBack();
		currCarFront = currCar->getPosition();

		if (currCarFront >= start && currCarBack <= end && !currCar->isFinished())
		{
			return currCar;
		}
	}

	return nullptr;
}



Car* Road::getLastCarAfter(double start)
{
	Car* pCurrCar = nullptr;
	double currCarFront = -1;

	int size = cars.size();
	for (list<Car*>::reverse_iterator rit = cars.rbegin(); rit != cars.rend(); ++rit)
	{
		pCurrCar = *rit;
		currCarFront = pCurrCar->getPosition();

		if (currCarFront > start)
			return pCurrCar;
	}

	return nullptr;
}



Car* Road::getFirstCar()
{
	if (!cars.empty())
		return cars.front();
	else
		return nullptr;
}



Car* Road::getFirstCarAfter(double start)
{
	for (Car* car : cars)
	{
		if (car->getPosition() >= start)
			return car;
	}

	return nullptr;
}



Car* Road::getFirstCarInRange(double start, double end)
{
	for (Car* pCar : cars)
	{
		if (pCar->getPosition() >= start && pCar->getPosition() <= end)
			return pCar;
	}

	return nullptr;
}



int Road::idxOfSpeedLimitAt(double pos)
{
	int numOfSpeedLimits = speedLimits.size();
	for (int i = 0; i < numOfSpeedLimits; i++)
	{
		if (speedLimits[i].getStartPos() <= pos)
			return i;
	}

	return -1;
}



double Road::speedLimitAt(double pos)
{
	int numOfSpeedLimits = speedLimits.size();
	for (int i = 0; i < numOfSpeedLimits; i++)
	{
		if (speedLimits[i].getStartPos() <= pos)
			return speedLimits[i].getValue();
	}

	return -1;
}



double Road::nextSpeedLimit(double pos, double* speedLimitStartPosition)
{
	int idxOfSpeedLimit = idxOfSpeedLimitAt(pos);

	if ((idxOfSpeedLimit + 1) < (int)speedLimits.size())
	{
		if (speedLimitStartPosition != nullptr)
			*speedLimitStartPosition = speedLimits[idxOfSpeedLimit + 1].getStartPos();
		return speedLimits[idxOfSpeedLimit + 1].getValue();
	}
	else
	{
		if (speedLimitStartPosition != nullptr)
			*speedLimitStartPosition = -1;
		return -1;
	}
}



bool Road::isRoadAccessible(Road* pRoad)
{
	if (pJunctionInput != nullptr)
	{
		if (pJunctionInput->gateToRoad(pRoad) != nullptr)
			return true;
	}

	return isLaneAccessible(pRoad);
}



bool Road::isLane()
{
	return pLeftNeighbour != nullptr || pRightNeighbour != nullptr;
}



bool Road::canEnter(Car* pRequester, Car** ppCarInFront, Car** ppCarBehind)
{
	bool consideringCarInFront = canEnterConsideringCarInFront(ppCarInFront, pRequester);
	bool consideringCarBehind = canEnterConsideringCarBehind(ppCarBehind, pRequester);
	
	// in the very unlikely case when carInFront and carBehind are the same (it can happen when the
	// blocking car has the same position as the requesting car) treat the car as carInFront
	if (*ppCarInFront == *ppCarBehind)
		*ppCarBehind = nullptr;

	return consideringCarInFront && consideringCarBehind;
}



bool Road::canEnterConsideringCarInFront(Car** ppCarInFront, Car* pRequester)
{
	Car* pCarInFront = getFirstCarAfter(pRequester->getPosition());

	bool res = isRequiredDistanceMaintained(pRequester, *ppCarInFront);

	* ppCarInFront = res ? nullptr : pCarInFront;

	return res;
}



bool Road::canEnterConsideringCarBehind(Car** ppCarBehind, Car* pRequester)
{
	Car* pCarBehind = getLastCarInSection(0, pRequester->getPosition());

	bool res = isRequiredDistanceMaintained(pCarBehind, pRequester);

	*ppCarBehind = res ? nullptr : pCarBehind;

	return res;
}



bool Road::isRequiredDistanceMaintained(Car* pCarBehind, Car* pCarInFront)
{
	bool res = true;

	if (pCarInFront != nullptr && pCarBehind != nullptr)
	{
		double position = pCarBehind->getPosition();
		double distToFrontCar = pCarInFront->getBack() - position;
		if (distToFrontCar <= 0) // the front car is blocking entrance
			res = false;
		else
		{
			double brakingRoad = 0.0; // should be different than 0 only if the next car drives slower
			double carInFrontSpeed = pCarInFront->getSpeed();
			double carBehindSpeed = pCarBehind->getSpeed();
			if (carBehindSpeed - carInFrontSpeed > 0) // if the car behind goes faster
				brakingRoad = mov::brakingDistance(carBehindSpeed, carInFrontSpeed, pCarBehind->getAvgBrake());	// br. road to front car

			double requiredDistToFrontCar = pCarBehind->minRequiredDistanceToObstacle(brakingRoad);

			if (requiredDistToFrontCar > distToFrontCar)
				res = false;
		}
	}

	return res;
}



bool Road::isLaneAccessible(Road* pLane)
{
	return pLeftNeighbour == pLane || pRightNeighbour == pLane;
}



double Road::getOptimalTime()
{
	return optimalTime;
}



void Road::setOptimalTime(double time)
{
	this->optimalTime = time;
}



void Road::initOptimalTime()
{
	optimalTime = 0.0;
	double speed, limitLength;
	for (SpeedLimit& limit : speedLimits) // add optimal durations of all speed limit ranges
	{
		limitLength = limit.getEndPos() - limit.getStartPos();
		speed = limit.getValue();
		optimalTime += mov::time(speed, limitLength, 0.0);
	}
}



Car* Road::getCarBehind(Car* pCar)
{
	Car* pRes = nullptr;

	auto itMatchingCar = find(cars.begin(), cars.end(), pCar);
	
	if (itMatchingCar != cars.end()) // car found
	{
		int idx = distance(cars.begin(), itMatchingCar);
		
		if (idx > 0)
		{
			auto itCars = cars.begin();
			advance(itCars, idx - 1);
			pRes = *itCars;
		}
	}

	return pRes;
}



Car* Road::getCarInFront(Car* pCar)
{
	Car* pCarInFront = nullptr;
	bool found = false;
	auto itCars = cars.begin();

	while (itCars != cars.end() && !found)
	{
		if (*itCars == pCar)
		{
			if (itCars != cars.begin())
				pCarInFront = *(--itCars);

			found = true;
		}

		++itCars;
	}

	return pCarInFront;
}



void Road::enterLane(Car* pCar)
{
	if (cars.empty())
		cars.push_back(pCar);
	else
	{
		auto itCars = cars.begin();
		Car* pCurr = *itCars;
		double carPosition = pCar->getPosition();
		/*
			Cars are sorted by position and the first car is the furthest one. To get the
			right position to insert the car I must proceed as long as I find a car which
			is closer to the beginning of the road.
		*/
		while (itCars != cars.end() && pCurr->getPosition() > carPosition)
		{
			pCurr = *itCars;
			advance(itCars, 1);
		}

		cars.insert(itCars, pCar);
	}
}



bool Road::putCarInSection(Car* pCar, double sectionStart, double sectionEnd)
{
	bool res = false;
	vector<Car*> relevantCars = getCarsInSectionWithSurroundings(sectionStart, sectionEnd);
	bool isPossible = canPutCarAmongCars(pCar, relevantCars, sectionEnd - sectionStart);

	if (isPossible)
	{
		if (putCarInFirstGap(pCar, relevantCars, sectionStart, sectionEnd)) // if fits
			res = true;
		else // doesn't fit into any gap
		{
			rearrangeToMakeGap(relevantCars, sectionStart, pCar->getLength() + (2.0 * MIN_GAP_MARGIN));
			// at this point the insertion should never fail, but just in case
			res = putCarInFirstGap(pCar, relevantCars, sectionStart, sectionEnd);
		}
	}

	return res;
}



bool Road::canPutCarInSection(Car* pCar, double sectionStart, double sectionEnd)
{
	vector<Car*> relevantCars = getCarsInSectionWithSurroundings(sectionStart, sectionEnd);
	return canPutCarAmongCars(pCar, relevantCars, sectionEnd - sectionStart);
}



vector<Car*> Road::getCarsInSectionWithSurroundings(double start, double end)
{
	vector<Car*> res;
	auto itCars = cars.rbegin(); // iterate backwards (1st car -> the closest to the road start)
	Car* pFirstCarBeforeSection = nullptr;

	// move iterator to the first car in section
	while (itCars != cars.rend() && isCarInSection(*itCars, start, end) == -1)
	{
		pFirstCarBeforeSection = *itCars;
		++itCars;
	}

	// add the first car before section if any
	if (pFirstCarBeforeSection != nullptr)
		res.push_back(pFirstCarBeforeSection);

	// add the cars in section
	while (itCars != cars.rend() && isCarInSection(*itCars, start, end) == 0)
	{
		res.push_back(*itCars);
		++itCars;
	}

	// add the first car after section if any
	if (itCars != cars.rend())
		res.push_back(*itCars);

	return res;
}



bool Road::canPutCarAmongCars(Car* pCar, vector<Car*>& surroundingCars, double sectionLen)
{
	bool res = false;

	int numOfCars = surroundingCars.size();
	if (numOfCars < 2)
		res = true;
	else
	{
		// calculste space which the cars would take if they were compressed as much as possible
		double spaceCompressed = 0.0; 
		
		for (Car* pCar : surroundingCars)
			spaceCompressed += pCar->getLength() + MIN_GAP_MARGIN;

		// at this point there is one MIN_GAP_MARGIN too much, so just add the pCar length
		spaceCompressed += pCar->getLength();

		// add a bit of additional space, to prevent unexpected double comparison related issues
		spaceCompressed += ADDITIONAL_GAP_MARGIN_SPACE;

		// calculate the total space the cars take
		double start = surroundingCars.front()->getBack();
		double end = surroundingCars.back()->getPosition();
		double maxSpace = max(end - start, sectionLen);

		// if the compressed space is not bigger than the maxSpace, then it's possible to arrange
		// the cars so that the new one will fit between
		res = spaceCompressed <= maxSpace;
	}
	
	return res;
}



bool Road::putCarInFirstGap(Car* pCar, vector<Car*>& relevantCars, double start, double end)
{
	bool res = false;

	// drop first and last car if they are not in section
	vector<Car*> carsInSection = relevantCars;
	if (!relevantCars.empty())
	{
		if (isCarInSection(carsInSection.back(), start, end) == 1)
			carsInSection.erase(carsInSection.end() - 1);

		if (!carsInSection.empty())
		{
			if (isCarInSection(carsInSection.front(), start, end) == -1)
				carsInSection.erase(carsInSection.begin());
		}
	}

	double gapStart = getBigEnoughGapStart(carsInSection, start, end, pCar->getLength() + 
		(2.0 * MIN_GAP_MARGIN));

	if (gapStart >= 0) // found a gap which is big enough
	{
		double positionToPut = gapStart + MIN_GAP_MARGIN + pCar->getLength();
		pCar->setPositionHard(positionToPut);
		Car* pCarInFront = putCarAtPosition(pCar, positionToPut);
		if (pCarInFront == nullptr)
			pCar->setSpeed(speedLimitAt(positionToPut));
		else
			pCar->setSpeed(pCarInFront->getSpeed());

		res = true;
	}

	return res;
}



double Road::getBigEnoughGapStart(vector<Car*>& carsInSection, double start, double end,
	double minSpace)
{
	double gapStart = -1.0; // position at which big enough gap starts
	int numOfCars = carsInSection.size();

	if (end - start >= minSpace)
	{
		if (numOfCars == 0)
			gapStart = start;
		else
		{
			Car* pCurrCar = nullptr;
			double gap = -1;

			// first gap between the bginning and the first car
			gap = carsInSection[0]->getBack() - start;
			if (gap >= minSpace)
				gapStart = start;
			else
			{
				for (int i = 0; i < numOfCars && gapStart < 0.0; ++i)
				{
					pCurrCar = carsInSection[i];

					if (i < numOfCars - 1)
						gap = carsInSection[i + 1]->getBack() - pCurrCar->getPosition();
					else
						gap = end - pCurrCar->getPosition();

					if (gap >= minSpace)
						gapStart = pCurrCar->getPosition();
				}
			}
		}
	}

	return gapStart;
}



Car* Road::putCarAtPosition(Car* pCar, double position)
{
	if (cars.empty())
		cars.insert(cars.begin(), pCar);
	else
	{
		Car* carInFront = nullptr;

		list<Car*>::iterator itCars;
		for (itCars = cars.begin(); itCars != cars.end(); ++itCars)
		{
			if ((*itCars)->getPosition() < position)
			{
				cars.insert(itCars, pCar);
				pCar->setPosition(position);

				--itCars;

				if (itCars != cars.begin())
					carInFront = *(--itCars);

				return carInFront;
			}
		}

		// if is the last
		cars.insert(itCars, pCar);
		--itCars;

		carInFront = *(--itCars);
	}

	return nullptr;
}



void Road::rearrangeToMakeGap(vector<Car*>& relevantCars, double gapPosition, double gapSize)
{
	// check if the first car is in section
	Car* pFirstCarToSqueeze = nullptr;

	if (relevantCars[0]->getPosition() > gapPosition)
		pFirstCarToSqueeze = relevantCars[0];
	else
	{
		if (relevantCars.size() > 1)
			pFirstCarToSqueeze = relevantCars[1];
	}

	if (pFirstCarToSqueeze != nullptr)
	{
		squeezeCarForward(pFirstCarToSqueeze, gapSize + ADDITIONAL_GAP_MARGIN_SPACE);
	}
}



void Road::squeezeCarForward(Car* pCar, double distance)
{
	double newPosition = pCar->getPosition() + distance;
	pCar->setPosition(newPosition);

	Car* pCarInFront = getCarInFront(pCar);
	if (pCarInFront != nullptr)
	{
		double distToCarInFront = pCarInFront->getBack() - newPosition;
		if (distToCarInFront < MIN_GAP_MARGIN)
		{
			double requiredSqueeze = MIN_GAP_MARGIN - distToCarInFront;
			squeezeCarForward(pCarInFront, requiredSqueeze);
		}
	}
}



int Road::isCarInSection(Car* pCar, double sectionStart, double sectionEnd)
{
	double front = pCar->getPosition();
	double back = pCar->getBack();

	if (front < sectionStart)
		return -1;
	if (back > sectionEnd)
		return 1;

	return 0;
}



int Road::getCarIdx(Car* pCar)
{
	int idx = -1;

	auto itCar = find(cars.begin(), cars.end(), pCar);
	if (itCar != cars.end())
		idx = distance(cars.begin(), itCar);

	return idx;
}



bool Road::isFree(double start, double end)
{
	double front; 
	double back;

	for (Car* pCar : cars)
	{
		front = pCar->getPosition();
		if (front < start)
			return true;

		back = pCar->getBack();

		if (front >= start && back <= end)
			return false;
	}

	// no cars, so free
	return true;
}



// getters && setters /////////////////////////////////////////////////////////////////////////////////



list<Car*>& Road::getCars()
{
	return cars;
}



double Road::getLength()
{
	return length;
}



JunctionInput* Road::getJunctionInput()
{
	return pJunctionInput;
}



void Road::setLeftNeighbour(Road* pNeighbour)
{
	this->pLeftNeighbour = pNeighbour;
}



Road* Road::getLeftNeighbour()
{
	return this->pLeftNeighbour;
}



void Road::setRightNeighbour(Road* pNeighbour)
{
	this->pRightNeighbour = pNeighbour;
}



Road* Road::getRightNeighbour()
{
	return this->pRightNeighbour;
}
