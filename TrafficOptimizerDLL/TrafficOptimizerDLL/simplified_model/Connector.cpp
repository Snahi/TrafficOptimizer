#include "../pch.h"
#include "Connector.h"



// constructors ///////////////////////////////////////////////////////////////////////////////////



Connector::Connector(double delay, 
	std::unordered_map<SimplifiedRoad*, SimplifiedLights*>& roadLights, SimplifiedRoad* pFinalRoad)
{
	this->delay = delay;
	this->releaseTime = delay;
	this->roadLights = roadLights;
	this->pFinalRoad = pFinalRoad;
}

// methods ////////////////////////////////////////////////////////////////////////////////////////



void Connector::tick(double currTime)
{
	if (releaseTime <= currTime && cars.size() > 0)
	{
		SimplifiedRoad* pNextRoad = cars.front()->getNextRoad(); // get the next road of the first car
		// pNextRoad can't be nullptr here, if it was then it was detected in putCar()
		if (roadLights[pNextRoad]->isGreen()) // if lights to pNextRoad are green
		{
			/*
				Calculate the time which the car spent in this connector. It must be done before 
				putting the car on the road, because road also uses the same timer, so it would
				cause errors. The timer was set to currentTime at the moment when it entered this 
				connector, so the waiting time is curretnTime - timer.
			*/
			double waitingTime = currTime - cars.front()->getTimer();
			if (pNextRoad->putCar(cars.front(), currTime)) // if car was successfully put on its next road
			{
				pRoad->decrementNumOfCars(); // mark that the car left the road previous road
				pRoad->addWaitingTime(waitingTime); // add additional time equal to the time that the car spent here
				cars.pop_front();
				releaseTime = currTime + delay; // set the new release time including delay
			}
		}
	}
}



void Connector::putCar(SimplifiedCar* pCar, double currTime)
{
	SimplifiedRoad* pNextRoad = pCar->getNextRoad();
	if (pNextRoad == nullptr) // it was the last road of the car -> finish the car
	{
		pFinalRoad->putCar(pCar, currTime); // always success
		pCar->setEndTime(currTime);
		pRoad->decrementNumOfCars();
	}
	else if (!cars.empty()) // if there are waiting cars, then the car must wait no matter if its light is green
	{
		pCar->setTimer(currTime); // set timer to measure how much time the car spends in this connector
		cars.push_back(pCar);
	}
	else if (roadLights[pNextRoad]->isGreen()) // cars are empty and light is green
	{
		if (pNextRoad->putCar(pCar, currTime))
			pRoad->decrementNumOfCars(); // don't add additional time, the car didn't spend any time waiting (there was no jam)
		else
		{
			pCar->setTimer(currTime); // set timer to measure how much time the car spent in this connector
			cars.push_back(pCar);
		}
	}
	else // red lights
	{
		pCar->setTimer(currTime); // set timer to measure how much time the car spent in this connector
		cars.push_back(pCar);
	}
}



void Connector::reset()
{
	releaseTime = delay;
	cars.clear();
}



void Connector::setRoad(SimplifiedRoad* pRoad)
{
	this->pRoad = pRoad;
}



void Connector::addRoad(SimplifiedRoad* pRoad, SimplifiedLights* pLights)
{
	roadLights[pRoad] = pLights;
}



bool Connector::isConnected(SimplifiedRoad* pRoad)
{
	return roadLights.find(pRoad) != roadLights.end();
}