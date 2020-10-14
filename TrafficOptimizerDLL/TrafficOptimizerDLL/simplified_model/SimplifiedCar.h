#pragma once
#include "SimplifiedRoad.h"
#include <vector>



class SimplifiedRoad;



class SimplifiedCar
{
public:
	// constructors ///////////////////////////////////////////////////////////////////////////////
	SimplifiedCar(std::vector<SimplifiedRoad*>* pRoute);

	// methods ////////////////////////////////////////////////////////////////////////////////////
	bool timerFinished(double currTime);
	void setTimer(double time);
	SimplifiedRoad* getNextRoad();
	void incRoadIdx();
	void setStartTime(double time);
	void setEndTime(double time);
	double getEndTime();
	double getTotalTime();
	void reset();
	void setRoute(std::vector<SimplifiedRoad*>* pRoute);
	std::vector<SimplifiedRoad*>* getRoute();
	double getTimer();

private:
	double startTime;
	double endTime;
	/*
		Used by other elements of a model to measure time for this car, e.g. it is used to detect
		when the car reached the end of its current road. It is never changed automatically, only
		external objects influence it.
	*/
	double timer;
	int roadIdx;
	std::vector<SimplifiedRoad*>* pRoute;

};
