#include "../../pch.h"
#include "Roundabout.h"
#include "gate/AlwaysOpenGate.h"
#include <algorithm>
#include "../MovementFunctions.h"



// constructors ///////////////////////////////////////////////////////////////////////////////////



Roundabout::Roundabout(double length, double speed)
{
	this->length = length;
	this->speed = speed;
	this->freeSpace = length;

	vector<SpeedLimit> speedLimits;
	SpeedLimit limit(0, MOCK_ROAD_LEN, MOCK_ROAD_SPEED_LIMIT);
	speedLimits.push_back(limit);
	pMockRoad = Road::getInstance(MOCK_ROAD_LEN, &mockJunctionInput, speedLimits).getRoad();
	pMockRoad->setOptimalTime(mov::time(speed, length, 0.0));
}



// destructor /////////////////////////////////////////////////////////////////////////////////////



Roundabout::~Roundabout()
{
	delete pMockRoad;

	for (RoundaboutOutput* pOutput : outputs)
		delete pOutput;
}



// methods ////////////////////////////////////////////////////////////////////////////////////////



void Roundabout::reset()
{
	cars.clear();
	pMockRoad->reset();
	freeSpace = length;
}



int Roundabout::tick(double tickDuration)
{
	int res = TICK_SUCCESS;
	Car* pFinishedCar = nullptr;

	CarOnRoundabout* pCarOnRoundabout = nullptr;
	for (int i = 0; i < (int) cars.size() && i >= 0; ++i)
	{
		pCarOnRoundabout = &cars[i];
		if (pCarOnRoundabout->tick(tickDuration)) // if true then the car finished (on this roundabout)
		{
			pFinishedCar = pCarOnRoundabout->getCar();
			Gate* outputGate = pCarOnRoundabout->getOutputGate();
			// set appropriate speed for the car, because on roundabout it is neglected
			Road* pNextRoad = pFinishedCar->getNextRoad();
			double nextRoadSpeed = pNextRoad->speedLimitAt(0.0);
			pFinishedCar->setSpeed(min(speed, nextRoadSpeed));

			if (outputGate->enter(pFinishedCar) == ENTER_SUCCESS)
			{
				if (pFinishedCar->moveOnNextRoad())
				{
					freeSpace += pFinishedCar->getLength() + MIN_GAP_ROUNDABOUT;
					cars.erase(cars.begin() + i);
					--i; // so that i points to the next car, otherwise it would skip one
				}
				else
					res = TICK_CAR_BEYOND_ROUTE;
			}
		}
	}

	return res;
}



Gate* Roundabout::getGateAt(double position)
{
	double outputStart, outputEnd;
	for (RoundaboutOutput* pOutput : outputs)
	{
		outputStart = pOutput->getPosition();
		outputEnd = outputStart + ROAD_WIDTH;

		if (outputStart <= position && position <= outputEnd)
			return pOutput->getGate();
	}

	return nullptr;
}



int Roundabout::addInput(double position, int* pId)
{
	int res = validatePosition(position);

	if (res == ADD_INPUT_SUCCESS)
	{
		inputs.push_back(position);
		if (pId != nullptr)
			*pId = inputs.size() - 1;
	}

	return res;
}



int Roundabout::addOutput(double position, Gate* pGate)
{
	int res = validatePosition(position);
	if (res == 0)
	{
		res = validateGate(pGate);
		if (res == 0)
		{
			RoundaboutOutput* pOutput = new RoundaboutOutput(position, pGate);
			outputs.push_back(pOutput);
		}
	}

	return res;
}



int Roundabout::validateGate(Gate* pGate)
{
	if (pGate == nullptr)
		return ADD_OUTPUT_NULL_GATE;

	// check if the gate is not already connected to the roundabout
	for (RoundaboutOutput* pOutput : outputs)
	{
		if (pOutput->getGate() == pGate)
			return ADD_OUTPUT_NON_UNQ_GATE;
	}

	return 0;
}



int Roundabout::validatePosition(double position)
{
	if (position < 0.0)
		return NEGATIVE_POSITION;
	else if (position > length - ROAD_WIDTH)
		return TOO_BIG_POSITION;

	// position is in a proper range, now check if doesn't collide
	// find collisions with inputs
	for (double inputPos : inputs)
	{
		if (overlaps(position, inputPos))
			return POSITION_NOT_AVAILABLE;
	}

	// find collisions with outputs
	for (RoundaboutOutput* pOutput : outputs)
	{
		if (overlaps(position, pOutput->getPosition()))
			return POSITION_NOT_AVAILABLE;
	}

	return ADD_INPUT_SUCCESS;
}



bool Roundabout::overlaps(double position1, double position2)
{
	double end1 = position1 + ROAD_WIDTH;
	double end2 = position2 + ROAD_WIDTH;

	if (end1 > position2 && position1 < end2)
		return true;

	return false;
}



bool Roundabout::canPutCar(Car* pCar)
{
	double requiredSpace = pCar->getLength() + MIN_GAP_ROUNDABOUT;

	return requiredSpace <= freeSpace;
}



bool Roundabout::putCar(CarOnRoundabout& car)
{
	double requiredSpace = car.getLength() + MIN_GAP_ROUNDABOUT;
	if (freeSpace >= requiredSpace)
	{
		freeSpace -= requiredSpace;
		cars.push_back(car);
		return true;
	}

	return false;
}



double Roundabout::getTimeToOutput(double fromPos, Road* pGoalRoad)
{
	for (RoundaboutOutput* pOutput : outputs)
	{
		if (pOutput->getRoad() == pGoalRoad)
		{
			double dist;
			double outputPos = pOutput->getPosition();
			if (outputPos > fromPos)
				dist = outputPos - fromPos;
			else
				dist = length - fromPos + outputPos;

			return dist / speed;
		}
	}

	return -1.0;
}



bool Roundabout::hasOutputs()
{
	return outputs.size() > 0;
}



// getters && setters /////////////////////////////////////////////////////////////////////////



Road* Roundabout::getMockRoad()
{
	return pMockRoad;
}



Gate* Roundabout::getOutputGate(Road* pRoad)
{
	for (RoundaboutOutput* pOutput : outputs)
	{
		if (pOutput->getRoad() == pRoad)
			return pOutput->getGate();
	}

	return nullptr;
}



double Roundabout::getSpeed()
{
	return speed;
}



vector<CarOnRoundabout> Roundabout::getCarsOnRoundabout()
{
	return cars;
}