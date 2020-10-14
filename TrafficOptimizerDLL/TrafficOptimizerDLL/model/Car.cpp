#include "../pch.h"
#include "Car.h"
#include "MovementFunctions.h"
#include "road/SpeedLimit.h"
#include <algorithm>
#include <stdlib.h>
#include <time.h>


using namespace std;



// init ///////////////////////////////////////////////////////////////////////////////////////////



Car::Car(vector<Road*>* p_route, double length, double speed, double avgGas, double maxGas, 
	double avgBrake, double strongBrake, double maxBrake, double minGap, double reactionTime)
{
	this->pRoute = p_route;
	this->length = length;
	this->speed = speed;
	this->isHandbrakeOn = false;
	this->avgGas = avgGas;
	this->maxGas = maxGas;
	this->avgBrake = avgBrake;
	this->strongBrake = strongBrake;
	this->maxBrake = maxBrake;
	this->minGap = minGap;
	this->reactionTime = reactionTime;
	this->timeGap = reactionTime * TIME_GAP_TO_REACTION_RATIO;
	this->timeSinceLastReaction = 0.0;
	this->finished = false;
	this->evaluated = false;
	this->pYieldRequester = nullptr;
	this->pYieldRequestRecipient = nullptr;

	this->acc = 0;
	this->position = -1;
	this->pRoad = nullptr;		// initially car is not on any road
	this->roadIdx = -1;			// initially car is not on any road

	this->hasHardPosition = false;

	pSourceGenerator = nullptr;

	// statistics
	this->totalTime = 0.0;
	this->totalWaitingTime = 0.0;
	this->speedSum = 0.0;
	this->speedMeasurements = 0;
}



// methods ////////////////////////////////////////////////////////////////////////////////////////



int Car::tick(double duration)
{
	if (pRoad == nullptr)
		return TICK_CAR_NOT_STARTED;

	if (timeSinceLastReaction >= reactionTime)
	{
		adjustAcc(changeLane());
		timeSinceLastReaction = 0.0;
	}

	if (!isHandbrakeOn)
	{
		speed = mov::speed(speed, acc, duration);
		position = position + mov::distance(speed, acc, duration);
		position = position <= pRoad->getLength() ? position : pRoad->getLength();

		if (position >= pRoad->getLength())
			nextRoad();
	}

	timeSinceLastReaction += duration;

	updateStatistics(duration);

	return TICK_CAR_SUCCESS;
}



int Car::putOnStart()
{
	if (pRoute != nullptr && pRoute->size() > 0)
	{
		if (pRoute->at(0)->addCarOnStart(this) == ADD_CAR_SUCCESS)
		{
			pRoad = pRoute->at(0);
			roadIdx = 0;
			position = 0.0;

			return PUT_ON_START_OK;
		}
		else
			return PUT_ON_START_CANT_PUT_ON_ROAD;
	}
	else
		return PUT_ON_START_NO_ROADS;
}



int Car::nextRoad()
{
	int res = NEXT_ROAD_OK;

	Road* pNextRoad = getNextRoad();

	JunctionInput* pJInput = pRoad->getJunctionInput();

	if (pJInput == nullptr)
		res = NEXT_ROAD_END_OF_ROUTE;
	else
	{
		Gate* pGate = pJInput->gateToRoad(pNextRoad);
		if (pGate != nullptr && pGate->enter(this) == ENTER_SUCCESS)
		{
			pRoad->removeFirstCar();
			++roadIdx;
			pRoad = pNextRoad;
			if (!hasHardPosition) // if the car didn't enter the road in the middle
			{
				position = 0.0;
				hasHardPosition = false;
			}
		}
		else
			res = NEXT_ROAD_CANT_ENTER_ROAD;
	}

	return res;
}



bool Car::moveOnNextRoad()
{
	if (roadIdx < (int) pRoute->size() - 1)
	{
		pRoad->removeCar(this);
		pRoad = getNextRoad();
		++roadIdx;
		if (!hasHardPosition) // if the car didn't enter the road in the middle
		{
			position = 0.0;
			hasHardPosition = false;
		}

		return true;
	}
	else
		return false;
}



Road* Car::getNextRoad()
{
	if (roadIdx < (int) pRoute->size() - 1)
	{
		return pRoute->at(roadIdx + 1);
	}

	return nullptr;
}



Road* Car::getRoadAtPosition(int position)
{
	if (position <= (int) pRoute->size() - 1)
		return pRoute->at(position);
	else
		return nullptr;
}



// speed adjustment ///////////////////////////////////////////////////////////////////////////////



void Car::adjustAcc(int changeLaneRes)
{
	double nextCarAcc = desiredAccToNextCar();		// next acc based on the car in front
	double speedLimitAcc = desiredAccSpeedLimit();	// next acc based on speed limit
	double junctionAcc = desiredAccJunction();
	double yieldRequesterAcc = desiredAccYieldRequest();
	double changeLaneAcc = changeLaneRes == CHANGE_LANE_MUST_BRAKE_A_LITTLE ? avgBrake : 0.0;

	if (nextCarAcc <= 0.0 || speedLimitAcc <= 0.0 || junctionAcc <= 0.0 || changeLaneAcc < 0.0 ||
		yieldRequesterAcc <= 0.0)
	{
		acc =
			min(nextCarAcc,
				min(yieldRequesterAcc,
					min(changeLaneAcc,
						min(speedLimitAcc, junctionAcc))));
	}
	else
		acc = speedLimitAcc;
}



/*
	o-o _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ o-o
	   <--------------------------------------------------------------------------------------->
		                                 distanceToNextCar
	   <-------------------------------><----------------------><---------->
		          brakingRoad                 safeDistance         minGap
	   <------------------------------------------------------------------->
	                            requiredDistance

	if there is no next car, then there is nothing to limit the car, so add gas
											situation 1

	o-o _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ 


	if ther is the next car

											situation 2
	o-o _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ o-o
	   <------------------------------------------------------------------------------------>
		                                 distanceToNextCar
	   <---------------------------------------------------><----------------------><---------->
		          brakingRoad                                     safeDistance         minGap
	   <--------------------------------------------------------------------------------------->
	                                       requiredDistance
	   <---------------------------------------------------------------------------------->
	                                       minRequiredDistance


											situation 3
	o-o _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ o-o
	   <--------------------------------------------------------------------------------------->
										 distanceToNextCar
	   <---------------------------------------------------><----------------------><---------->
				  brakingRoad                                     safeDistance         minGap
	   <--------------------------------------------------------------------------------------->
								             requiredDistance


											situation 4
	o-o _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ o-o
	   <--------------------------------------------------------------------------------------->
										 distanceToNextCar
	   <-------------------------------><----------------------><---------->
				  brakingRoad                 safeDistance         minGap
	   <------------------------------------------------------------------->
								requiredDistance


								            situation 5
	o-o _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ o-o
	   <--------------------------------------------------------------------->
										 distanceToNextCar
	   <---------------------------------------------------><----------------------><---------->
				  brakingRoad                                     safeDistance         minGap
	   <--------------------------------------------------------------------------------------->
										   requiredDistance
	   <---------------------------------------------------------------------------------->
										   minRequiredDistance


											situation 6
	o-o _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ o-o
	   <--------------------------------------------------------------------->
										 distanceToNextCar
	   <---------------------------------------------------><------------><-------------------->
				  brakingRoad                                   minGap         safeDistance
	   <--------------------------------------------------------------------------------------->
										   requiredDistance
	   <---------------------------------------------------------------------------------->
										   minRequiredDistance
*/
double Car::desiredAccToNextCar()
{
	// default value is accelerate, if it should change it will be changed
	double desiredAcc = avgGas;
	Car* pNextCar = pRoad->getLastCarAfter(position);

	// no car, skip the logic, just accelerate - situation 1
	if (pNextCar != nullptr)
	{
		// there is the next car, calculate required parameters to asses desiredAcc
		double brakingRoad = 0.0;	// should be different than 0 only if the next car drives slower
		if (speed - pNextCar->getSpeed() > 0)	// if the next car drives faster
			brakingRoad = mov::brakingDistance(speed, pNextCar->getSpeed(), avgBrake);	// braking road to the next car;
		double distanceToNextCar = pNextCar->getBack() - position;	
		// calculate a safe distance for human reaction that should be kept to the next car
		double safeDistance = mov::distance(speed, acc, timeGap);
		double requiredDistance = brakingRoad + safeDistance + minGap;	// +/- distance to keep to the next car
		double minRequiredDistance = requiredDistance * (1.0 - POS_RANGE_TO_NEXT_CAR_FACTOR);

		// pull handbrake if very close
		if (distanceToNextCar <= minGap)
			pullUpHandbrake();
		else
			pullDownHandbrake();

		// assign appropriate acceleration
		if (distanceToNextCar >= minRequiredDistance)	// situation 2
		{
			if (isInRangeFactor(distanceToNextCar, requiredDistance, POS_RANGE_TO_NEXT_CAR_FACTOR)) // is in desired distance - situation 3
				desiredAcc = 0.0;	// continue with the same speed
			else  // too far - situation 4
				desiredAcc = avgGas;	// add gas
		}
		else // brake is required - situation 5
		{
			if (brakingRoad < distanceToNextCar + minGap)	// if possible with avg brake - situation 6
				desiredAcc = avgBrake;
			else // imposible with avg brake
			{
				// recalculate the braking distance for the strong brake
				if (speed - pNextCar->getSpeed() > 0)
					brakingRoad = mov::brakingDistance(speed, pNextCar->getSpeed(), strongBrake);

				// if possible with strong brake
				if (brakingRoad < distanceToNextCar + minGap)
					desiredAcc = strongBrake;
				else // imposible even with strong brake
					desiredAcc = maxBrake;	// use the maximum possible brake
			}
		}
	}

	return desiredAcc;
}



double Car::minRequiredDistanceToObstacle(double brakingRoad)
{
	double safeDistance = mov::distance(speed, acc, timeGap);
	double requiredDistance = brakingRoad + safeDistance + minGap;	// +/- distance to keep to the next car

	return requiredDistance * (1.0 - POS_RANGE_TO_NEXT_CAR_FACTOR);
}



double Car::desiredAccSpeedLimit()
{
	double desiredAcc = avgGas;

	double currSpeedLimit = pRoad->speedLimitAt(position);
	double nextSpeedLimitStart;
	double nextSpeedLimit = pRoad->nextSpeedLimit(position, &nextSpeedLimitStart);

	if (nextSpeedLimitStart >= 0)
	{
		// consider only when the next speed limit is smaller than the current one. If the next
		// speed limit is greater, then start accelerating once it is reached
		if (nextSpeedLimit < currSpeedLimit && nextSpeedLimit < speed)
		{
			double brDist = mov::brakingDistance(speed, nextSpeedLimit, avgBrake);
			double distToNextSpeedLimit = nextSpeedLimitStart - position;

			if (distToNextSpeedLimit <= brDist + BRAKING_DIST_BEFORE_SPEED_LIMIT)	// just avg brake, it's not a problem if car
				return avgBrake;													// goes too fast for a moment
		}
	}

	// if arrived here it means that the next speed limit is not important at this time, focus 
	// only on the current speed limit
	if (isInRange(speed, currSpeedLimit, SPEED_LIMIT_RANGE))
		desiredAcc = 0.0;
	else if (speed < currSpeedLimit)	// too slow
		desiredAcc = avgGas;
	else   // too fast
		desiredAcc = avgBrake;

	return desiredAcc;
}



double Car::desiredAccJunction()
{
	double desiredAcc = avgGas;

	// if there is a next road check if it is necessary to break
	if (roadIdx < (int) pRoute->size() - 1)
	{
		Gate* pNextGate = pRoad->getJunctionInput()->gateToRoad(pRoute->at(roadIdx + 1));
		if (pNextGate != nullptr)
		{
			// in case gate is open and the handbrake was pulled up release it
			if (pNextGate->isGateOpen(this))
				pullDownHandbrake();

			// gateSpeedLimit includes also the speed of the guarded road, which will matter if for
			// some reason the speed allowed on the guarded road is lower than on the gate
			double gateSpeedLimit = pNextGate->canEnter(this) ? pNextGate->getSpeedLimit() : 0.0;
			double distToJunction = pRoad->getLength() - position;

			if (gateSpeedLimit < speed) // only if braking may be required
			{
				double brDist = mov::brakingDistance(speed, gateSpeedLimit, avgBrake);
				double safeDist = mov::distance(speed, acc, timeGap);
				double reqDist = brDist + safeDist + minGap;

				if (distToJunction < reqDist) // only if braking is required
				{
					if (distToJunction > brDist + minGap) // enough if use normal brake
						desiredAcc = avgBrake;
					else
					{
						brDist = mov::brakingDistance(speed, gateSpeedLimit, strongBrake);
						if (distToJunction > brDist) // if strong brake is enough
							desiredAcc = strongBrake;
						else
							desiredAcc = maxBrake;
					}
				}
			}

			// it's after the previous if, because otherwise it wouldn't pull the brake if car is already stopped
			if (!pNextGate->isGateOpen(this) && distToJunction <= minGap) // to prevent moving on closed gate
			{
				pullUpHandbrake();
			}
		}
		else // null gate -> next road is lane
		{
			// if end of road is close enugh start braking
			double brDist = mov::brakingDistance(speed, 0, avgBrake);
			double safeDist = mov::distance(speed, acc, timeGap);
			double reqDist = brDist + safeDist + minGap;

			double distToJunction = distanceToEndOfRoad();
			if (distToJunction < reqDist) // only if braking is required
			{
				if (distToJunction > brDist + minGap) // enough if use normal brake
					desiredAcc = avgBrake;
				else
				{
					brDist = mov::brakingDistance(speed, 0, strongBrake);
					if (distToJunction > brDist) // if strong brake is enough
						desiredAcc = strongBrake;
					else
						desiredAcc = maxBrake;
				}
			}
		}
	}

	return desiredAcc;
}



double Car::desiredAccYieldRequest()
{
	return pYieldRequester != nullptr ? avgBrake : avgGas;
}



bool Car::isInRangeFactor(double valueToCheck, double exactValue, double rangeFactor)
{
	double max = exactValue + (exactValue * rangeFactor);
	double min = exactValue - (exactValue * rangeFactor);

	return valueToCheck >= min && valueToCheck <= max;
}



bool Car::isInRange(double valueToCheck, double exactValue, double delta)
{
	return (valueToCheck >= (exactValue - delta)) && (valueToCheck <= (exactValue + delta));
}



bool Car::pullUpHandbrake()
{
	if (speed <= HANDBRAKE_THRESHOLD)
	{
		isHandbrakeOn = true;
		acc = 0.0;
		speed = 0.0;
		return true;
	}
	return false;
}



void Car::pullDownHandbrake()
{
	isHandbrakeOn = false;
}



double Car::distanceToEndOfRoad()
{
	double roadLength = pRoad->getLength();
	return pRoad != nullptr && position < roadLength ? roadLength - position : 0.0;
}



Car* Car::clone()
{
	Car* pClone = new Car(pRoute, length, speed, avgGas, maxGas, avgBrake, strongBrake, maxBrake, minGap,
		reactionTime);
	pClone->timeGap = this->timeGap;
	pClone->acc = this->acc;
	pClone->position = this->position;
	pClone->isHandbrakeOn = this->isHandbrakeOn;
	pClone->pRoad = this->pRoad;
	pClone->roadIdx = this->roadIdx;
	pClone->timeSinceLastReaction = this->timeSinceLastReaction;
	pClone->evaluated = this->evaluated;
	pClone->finished = this->finished;
	pClone->hasHardPosition = this->hasHardPosition;
	pClone->pYieldRequester = this->pYieldRequester;
	pClone->pYieldRequestRecipient = this->pYieldRequestRecipient;
	pClone->totalTime = this->totalTime;
	pClone->totalWaitingTime = this->totalWaitingTime;
	pClone->speedSum = this->speedSum;
	pClone->speedMeasurements = this->speedMeasurements;
	pClone->pSourceGenerator = this->pSourceGenerator;

	return pClone;
}



void Car::removeFromRoad()
{
	if (pRoad != nullptr)
	{
		pRoad->removeCar(this);
		pRoad = nullptr;
		roadIdx = -1;
	}
}



void Car::finish()
{
	this->finished = true;
}



bool Car::isFinished()
{
	return finished;
}



/*
	1. if the next road is lane
		 True -> if the lane is accessible
			True
				if can enter the lane
					True -> enter the lane
					False
						if the blocking car is at the same position or further and has smaller or 
						equal speed

							|  _  |     |
		   blocking car --> | | |    _  |
							| |_| | | | | <-- this car
							|       |_| |

							True -> brake a little

						if the blocking car has smaller position

							|     |  _  |
							|  _    | | | <-- this car
		   blocking car --> | | | | |_| |
							| |_|       |

							True -> Send request to the car in order to enter the lane (in real world
								it would mean using an indicator, but sending the request is more efficient
								way to simulate it, because otherwise all cars would need some more 
								complicated logic for detecting indicators)
				False -> Return CHANGE_LANE_NOT_ACCESSIBLE
			False -> Return CHANGE_LANE_NOT_LANE
*/
int Car::changeLane()
{
	int res = CHANGE_LANE_SUCCESS;

	Road* pNextRoad = getNextRoad();
	if (pNextRoad != nullptr)
	{
		if (pNextRoad->isLane())
		{
			if (pRoad->isLaneAccessible(pNextRoad))
			{
				// randomize the moment of joining the other lane, but not further than half of the road
				srand((unsigned int) time(NULL));
				int randPos = rand() % ((int) pRoad->getLength() / 2);
				if (position >= randPos)
				{
					Car* pCarInFront = nullptr; // if there is a blocking car in front it will be set 
					Car* pCarBehind = nullptr; // if there is a bolocking car behind it will be set
					bool canEnterRes = pNextRoad->canEnter(this, &pCarInFront, &pCarBehind);
					res = reactToCanEnterResult(canEnterRes, pCarBehind, pCarInFront);
				}
				else
					res = CHANGE_LANE_TOO_EARLY;
			}
			else
				res = CHANGE_LANE_NOT_ACCESSIBLE;
		}
		else
			res = CHANGE_LANE_NOT_LANE;
	}
	else
		res = CHANGE_LANE_NO_NEXT_ROAD;

	return res;
}



int Car::reactToCanEnterResult(bool result, Car* pCarBehind, Car* pCarInFront)
{
	int res = CHANGE_LANE_SUCCESS;

	if (result)
	{
		// change lane
		pRoad->removeCar(this);			// first remove car from the current road
		++roadIdx;						// then increment current road index
		pRoad = pRoute->at(roadIdx);	// set the next lane as the current road
		pRoad->enterLane(this); 		// enter the lane

		// release yield request recipient
		if (pYieldRequestRecipient != nullptr)
		{
			pYieldRequestRecipient->removeYieldRequester(this);
			pYieldRequestRecipient = nullptr;
		}
	}
	else // not enough space on the road
	{
		// check if there is a car behind in a close distance
		if (pCarBehind != nullptr)
		{
			res = CHANGE_LANE_MUST_WAIT; // inform that just needs to wait
			if (pCarBehind->sendEnterLaneRequest(this)) // request was accepted
			{
				// if the recipient changed, release the previous one
				if (pYieldRequestRecipient != pCarBehind && pYieldRequestRecipient != nullptr)
					pYieldRequestRecipient->removeYieldRequester(this);

				pYieldRequestRecipient = pCarBehind; // store info about car which is giving way
			}
			else // request was rejected
			{
				// send a request to further cars. Ignore result of the method because it doesn't
				// change anything, if no car accepted then this car must proceed and just wait
				sendEnterLaneRequestToFurtherCars(pCarBehind);
			}
		}

		// check if there is a car in front. it is important that it is done after processing the
		// car behind because it may require acc change, so a proper result code must be returned
		if (pCarInFront != nullptr)
			res = CHANGE_LANE_MUST_BRAKE_A_LITTLE;
	}

	return res;
}



bool Car::sendEnterLaneRequest(Car* pRequester)
{
	int res = true;

	if (pYieldRequester == nullptr) // if there is no pending request
	{
		if (canTakeYieldRequest(pRequester))
			pYieldRequester = pRequester;
		else
			res = false;
	}
	else if (pYieldRequester != pRequester) // if there is a different pending request
		res = false;

	return res;
}



/*
	Checks if it is possible so let the car in safely before the end of the road.
*/
bool Car::canTakeYieldRequest(Car* pRequester)
{
	if (finished)
		return false;

	int res = true;

	if (pRequester != nullptr)
	{
		double distToCar = pRequester->getBack() - position; // can be negative

		if (distToCar > minGap + MIN_DISTANCE_FOR_ACCEPTING_REQUEST)
		{
			double brakingRoad = 0.0; // should be different than 0 only if the requester drives slower
			double requesterSpeed = pRequester->getSpeed();
			if (speed - requesterSpeed > 0) // if the requester goes slower
				brakingRoad = mov::brakingDistance(speed, requesterSpeed, avgBrake); // br. road to requester

			double minDist = minRequiredDistanceToObstacle(brakingRoad);

			res = minDist < distanceToEndOfRoad();
		}
		else
			res = false;
	}
	else
		res = false;

	return res;
}



bool Car::removeYieldRequester(Car* pRequester)
{
	if (pRequester == pYieldRequester)
	{
		pYieldRequester = nullptr;
		return true;
	}

	return false;
}



bool Car::sendEnterLaneRequestToFurtherCars(Car* pRejecter)
{
	bool res = true;

	Car* pCarBehind = pRejecter->getCarBehind();
	
	if (pCarBehind != nullptr) // if there is a car behind
	{
		if (pCarBehind->sendEnterLaneRequest(this)) // request accepted
		{
			// if the recipient changed, release the previous one
			if (pYieldRequestRecipient != pCarBehind && pYieldRequestRecipient != nullptr)
				pYieldRequestRecipient->removeYieldRequester(this);

			pYieldRequestRecipient = pCarBehind; // store the recipient to release it later
		}
		else // request rejected
		{
			// send request to the next car
			sendEnterLaneRequestToFurtherCars(pCarBehind);
		}
	}
	else // no more cars, no car accepted the request
		res = false;

	return res;
}



Car* Car::getCarBehind()
{
	if (pRoad != nullptr)
		return pRoad->getCarBehind(this);
	else
		return nullptr;
}



void Car::setSpeed(double speed)
{
	this->speed = speed;
}



void Car::moveToNextRoad()
{
	pRoad->removeCar(this);
	++roadIdx;
	pRoad = pRoute->at(roadIdx);
}



// statistics /////////////////////////////////////////////////////////////////////////////////////



void Car::updateStatistics(double timePassed)
{
	totalTime += timePassed;
	if (speed == 0.0)
		totalWaitingTime += timePassed;

	speedSum += speed;
	speedMeasurements++;
}



double Car::getTotalTime()
{
	return totalTime;
}



double Car::getTotalTimeWaiting()
{
	return totalWaitingTime;
}



double Car::getAvgSpeed()
{
	return speedSum / speedMeasurements;
}



// getters & setters //////////////////////////////////////////////////////////////////////////////



double Car::getPosition()
{
	return position;
}



void Car::setPosition(double position)
{
	this->position = position;
}



double Car::getLength()
{
	return length;
}



double Car::getBack()
{
	return position - length;
}



int Car::getRoadPos()
{
	return roadIdx;
}



void Car::setPositionHard(double position)
{
	this->position = position;
	this->hasHardPosition = true;
}



vector<Road*>* Car::getRoute()
{
	return pRoute;
}



double Car::getSpeed()
{
	return speed;
}



double Car::getAcc()
{
	return acc;
}



double Car::getMinGap()
{
	return minGap;
}



double Car::getTimeGap()
{
	return timeGap;
}



void Car::setLength(double length)
{
	this->length = length;
}



double Car::getAvgBrake()
{
	return avgBrake;
}



double Car::getStrongBrake()
{
	return strongBrake;
}



Road* Car::getFirstRoad()
{
	if (pRoute->size() > 0)
		return pRoute->front();
	else
		return nullptr;
}



Road* Car::getCurrRoad()
{
	return pRoad;
}



void Car::setEvaluated(bool evaluated)
{
	this->evaluated = evaluated;
}



bool Car::isEvaluated()
{
	return evaluated;
}



double Car::getReactionTime()
{
	return reactionTime;
}



void Car::setSourceGenerator(Generator* pGenerator)
{
	this->pSourceGenerator = pGenerator;
}



Generator* Car::getSourceGenerator()
{
	return pSourceGenerator;
}


