#include "../pch.h"
#include "SimplifiedCar.h"



using namespace std;



// constructors ///////////////////////////////////////////////////////////////////////////////



SimplifiedCar::SimplifiedCar(vector<SimplifiedRoad*>* pRoute)
{
	this->startTime = DBL_MAX;
	this->endTime = DBL_MAX;
	this->timer = DBL_MAX;
	this->roadIdx = -1;
	this->pRoute = pRoute;
}

// methods ////////////////////////////////////////////////////////////////////////////////////



bool SimplifiedCar::timerFinished(double currTime)
{
	return currTime >= timer;
}



void SimplifiedCar::setTimer(double time)
{
	this->timer = time;
}



SimplifiedRoad* SimplifiedCar::getNextRoad()
{
	int idx = roadIdx + 1;
	return idx < (int) pRoute->size() ? pRoute->at(idx) : nullptr;
}



void SimplifiedCar::incRoadIdx()
{
	++roadIdx;
}



void SimplifiedCar::setStartTime(double time)
{
	this->startTime = time;
}



void SimplifiedCar::setEndTime(double time)
{
	this->endTime = time;
}



double SimplifiedCar::getEndTime()
{
	return endTime;
}



double SimplifiedCar::getTotalTime()
{
	return endTime - startTime;
}



void SimplifiedCar::reset()
{
	this->startTime = DBL_MAX;
	this->endTime = DBL_MAX;
	this->timer = DBL_MAX;
	roadIdx = -1;
}



void SimplifiedCar::setRoute(vector<SimplifiedRoad*>* pRoute)
{
	this->pRoute = pRoute;
}



vector<SimplifiedRoad*>* SimplifiedCar::getRoute()
{
	return pRoute;
}



double SimplifiedCar::getTimer()
{
	return timer;
}


