#pragma once
#include <unordered_map>
#include "SimplifiedRoad.h"
#include "SimplifiedLights.h"
#include "SimplifiedCar.h"
#include <list>



class SimplifiedRoad;
class SimplifiedCar;



/*
	Connects a single road to other roads (can be more). It is always combined with one road
	(pRoad) which is the road which ends in this connector. These two objects work closely 
	together. When a car crosses the entire road the road puts it into its associated connector and
	removes the car from itself BUT it still counts the car in it's counter! Only after the car
	leaves the associated connector the counter is decremented.
	If there are no other waiting cars in the connector and the light to the next road of the car
	is green then the car is simply put on the next road. If it was the last road of the car, then
	the car is put on the final road. In any other case (other cars waiting, red light) the car is
	added to a cars list which represents a traffic jam. Cars are taken from the cars list one by
	one, but with delays so that human reaction time can be simulated.
*/
class Connector
{
public:
	// constructors ///////////////////////////////////////////////////////////////////////////////
	Connector(double delay, std::unordered_map<SimplifiedRoad*, SimplifiedLights*>& roadLights,
		SimplifiedRoad* pFinalRoad);

	// methods ////////////////////////////////////////////////////////////////////////////////////
	void tick(double currTime);
	void putCar(SimplifiedCar* pCar, double currTime);
	/*
		Restore the initial state of this connector. As if it was newly created.
	*/
	void reset();
	/*
		Set the road which ends in this connector.
	*/
	void setRoad(SimplifiedRoad* pRoad);
	/*
		Add a road which can be reached from this connector.
	*/
	void addRoad(SimplifiedRoad* pRoad, SimplifiedLights* pLights);
	/*
		Checks if the specified road is accessible from this connector.
	*/
	bool isConnected(SimplifiedRoad* pRoad);

private:
	/*
		How much time must pass before a car starts driving when it's stopped. It is to simulate 
		human reaction -> when a light changes from red to green not all cars start at the same
		time.
	*/
	double delay;
	/*
		Time at which next car will be released on its next road
	*/
	double releaseTime;
	/*
		Connects a road with lights which must be green to put a car on the road.
	*/
	std::unordered_map<SimplifiedRoad*, SimplifiedLights*> roadLights;
	/*
		Cars waiting for entering their desired road, traffic jam
	*/
	std::list<SimplifiedCar*> cars;
	/*
		Road which ends in this connector
	*/
	SimplifiedRoad* pRoad;
	/*
		Final road of the owning model. If a car enters this connector and it has no more roads in
		its route then it is put on pFinalRoad.
	*/
	SimplifiedRoad* pFinalRoad;
};
