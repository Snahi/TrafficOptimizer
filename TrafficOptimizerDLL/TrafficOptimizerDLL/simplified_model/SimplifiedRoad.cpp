#include "../pch.h"
#include "SimplifiedRoad.h"



using namespace std;



// constructors ///////////////////////////////////////////////////////////////////////////////
SimplifiedRoad::SimplifiedRoad(double transferTime, int maxCars)
{
	this->transferTime = transferTime;
	this->maxCars = maxCars;
	this->numOfCars = 0;
	this->carsCounter = 0;
	this->additionalTime = 0.0;
	this->pConnector = nullptr;
}



SimplifiedRoad::~SimplifiedRoad()
{
	if (pConnector != nullptr)
		delete pConnector;
}



// methods ////////////////////////////////////////////////////////////////////////////////////



void SimplifiedRoad::tick(double currTime)
{
	pConnector->tick(currTime);

	if (cars.size() > 0)
	{
		// check if the first car reached the end
		if (cars.front()->timerFinished(currTime)) // car reached the end of the road
		{
			pConnector->putCar(cars.front(), currTime); // add the car to connector
			/*
				Remove the car form the cars list, but do not decrease numOfCars. NumOfCars 
				will be decreased when the car leaves pConnector. It is like that so that
				no more than maxCars can be added. Once a car reaches the end there is no point
				in keeping it in cars and if we remove it we can check just if the first car
				reached the end, not the entire list.
			*/
			cars.pop_front();
		}
	}
}



bool SimplifiedRoad::putCar(SimplifiedCar* pCar, double currTime)
{
	if (numOfCars < maxCars)
	{
		pCar->incRoadIdx();
		pCar->setTimer(currTime + transferTime); // set car's timer to the time when it will reach the end of this road
		cars.push_back(pCar);
		++numOfCars;
		++carsCounter;

		return true;
	}
	else // road full
	{
		return false;
	}
}



void SimplifiedRoad::resetCarsCounter()
{
	carsCounter = 0;
	additionalTime = 0.0;
}



int SimplifiedRoad::getCarsCounter()
{
	return carsCounter;
}



int SimplifiedRoad::getNumOfCars()
{
	return numOfCars;
}



list<SimplifiedCar*>& SimplifiedRoad::getCars()
{
	return cars;
}



void SimplifiedRoad::decrementNumOfCars()
{
	--numOfCars;
}



void SimplifiedRoad::addWaitingTime(double waitingTime)
{
	additionalTime += waitingTime;
}



double SimplifiedRoad::getAdditionalTime()
{
	return additionalTime;
}



void SimplifiedRoad::reset()
{
	numOfCars = 0;
	carsCounter = 0;
	additionalTime = 0.0;
	cars.clear();

	if (pConnector != nullptr) // in case it is the final road
		pConnector->reset();
}



void SimplifiedRoad::setConnector(Connector* pConnector)
{
	this->pConnector = pConnector;
	pConnector->setRoad(this);
}



void SimplifiedRoad::addRoadToConnector(SimplifiedRoad* pRoad, SimplifiedLights* pLights)
{
	if (pConnector != nullptr)
		pConnector->addRoad(pRoad, pLights);
}



bool SimplifiedRoad::isConnected(SimplifiedRoad* pRoad)
{
	if (pConnector != nullptr)
		return pConnector->isConnected(pRoad);
	else
		return false;
}


