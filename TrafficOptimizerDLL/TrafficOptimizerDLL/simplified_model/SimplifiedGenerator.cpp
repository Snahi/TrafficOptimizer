#include "../pch.h"
#include "SimplifiedGenerator.h"



using namespace std;



// constructors ///////////////////////////////////////////////////////////////////////////////////



SimplifiedGenerator::SimplifiedGenerator(double period, SimplifiedCar* pTemplateCar, 
	SimplifiedRoad* pRoad, double phase)
{
	this->period = period;
	this->phase = phase;
	this->nextGenerationTime = period + phase;
	this->pTemplateCar = pTemplateCar;
	this->pRoad = pRoad;
	this->pModel = nullptr;
}



// destructor /////////////////////////////////////////////////////////////////////////////////////



SimplifiedGenerator::~SimplifiedGenerator()
{
	if (pTemplateCar != nullptr)
		delete pTemplateCar;
}



// methods ////////////////////////////////////////////////////////////////////////////////////////



void SimplifiedGenerator::tick(double currTime)
{
	if (!readyCars.empty()) // there is already a car waiting to be put on a road
	{
		if (pRoad->putCar(readyCars.front(), currTime)) // if was able to put the car on pRoad
		{
			readyCars.front()->setStartTime(currTime);
			readyCars.pop_front();
		}
	}

	if (nextGenerationTime <= currTime) // if it is time for car generation
	{
		SimplifiedCar* pCar = pModel->getCarInstance(pTemplateCar); // get car instance from a car pool
		if (pRoad->putCar(pCar, currTime))
			pCar->setStartTime(currTime);
		else // couldn't put car on pRoad
		{
			readyCars.push_back(pCar); // add the car to the queue
			pCar->setStartTime(currTime);
		}

		nextGenerationTime = currTime + period;
	}
}



void SimplifiedGenerator::reset()
{
	nextGenerationTime = period + phase;
	readyCars.clear();
}



void SimplifiedGenerator::setModel(SimplifiedModel* pModel)
{
	this->pModel = pModel;
}



list<SimplifiedCar*>* SimplifiedGenerator::getReadyCars()
{
	return &readyCars;
}



int SimplifiedGenerator::getNumOfReadyCars()
{
	return (int)readyCars.size();
}