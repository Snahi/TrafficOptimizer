#pragma once
#include "SimplifiedCar.h"
#include "Connector.h"
#include "SimplifiedLights.h"
#include <list>



class SimplifiedCar;
class Connector;



class SimplifiedRoad
{
public:
	// constructors ///////////////////////////////////////////////////////////////////////////////
	SimplifiedRoad(double transferTime, int maxCars);
	~SimplifiedRoad();

	// methods ////////////////////////////////////////////////////////////////////////////////////
	void tick(double currTime);
	bool putCar(SimplifiedCar* pCar, double currTime);
	/*
		Resets cars counter AND ADDITIONAL TIME (because they are strongly correlated, when the
		counter is reset then we are interested in the additional time only for these cars
		which crossed the road after reset).
	*/
	void resetCarsCounter();
	int getCarsCounter();
	int getNumOfCars();
	std::list<SimplifiedCar*>& getCars();
	void decrementNumOfCars();
	void addWaitingTime(double waitingTime);
	double getAdditionalTime();
	void reset();
	void setConnector(Connector* pConnector);
	void addRoadToConnector(SimplifiedRoad* pRoad, SimplifiedLights* pLights);
	bool isConnected(SimplifiedRoad* pRoad);

private:
	/*
		How long it takes for a time to drive through the entire road.
	*/
	double transferTime;
	/*
		Maximum number of cars which can be on this road at the same time.
	*/
	int maxCars;
	/*
		Number of cars on this road at the moment.
	*/
	int numOfCars;
	/*
		How many cars have eneterd this road since last reset of the counter.
	*/
	int carsCounter;
	/*
		Tototal time spend on road (including waiting in connector) minus transfer time. Counted
		for cars counted in carsCounter.
	*/
	double additionalTime;
	std::list<SimplifiedCar*> cars;
	/*
		Connector to the next roads.
	*/
	Connector* pConnector;
};
