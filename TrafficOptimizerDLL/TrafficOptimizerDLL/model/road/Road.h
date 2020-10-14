#pragma once
#include "SpeedLimit.h"
#include "../junction/JunctionInput.h"
#include <list>
#include "../Car.h"

class RoadCreationResult;
class Car;
class JunctionInput;

// constants //////////////////////////////////////////////////////////////////////////////////////
// add car on start
#define ADD_CAR_SUCCESS 0
#define ADD_CAR_NOT_ENOUGH_SPACE 1
// put car in section
#define MIN_GAP_MARGIN 3 // minimum distance to the car before and the car after after
#define ADDITIONAL_GAP_MARGIN_SPACE 0.4 // it will be added to the required gap size, just to make
										// sure that double comparison won't fail car insertion
										// after rearranging

using namespace std;

/*
	Class for representing a road or a single lane of a multi-lane road.
*/
class Road
{
public:
	static RoadCreationResult getInstance(double length, JunctionInput* pJunctionInput, 
		vector<SpeedLimit> speedLimits);
	// methods ////////////////////////////////////////////////////////////////////////////////////
	void reset();
	/*
		Checks if the car will fit at the beginnig of the road, including it's size and braking 
		road to the current last car or to the end of the road if there is no other car.
	*/
	bool canAddCarOnStart(Car* pCar);
	bool canAddCarOnStart(Car* pCar, double delay);
	/*
		Checks if car can be added on start and if yes, then adds it to the cars list
	*/
	int addCarOnStart(Car* car);
	void removeCar(Car* car);
	bool removeFirstCar();
	/*
		returns last car in section (start, end>
	*/
	Car* getLastCarInSection(double start, double end);
	Car* getLastCarAfter(double start);
	Car* getFirstCar();
	/*
		Returns first car found starting at start, so the range is inclusive [start, end]
	*/
	Car* getFirstCarAfter(double start);
	Car* getFirstCarInRange(double start, double end);
	double speedLimitAt(double pos);
	double nextSpeedLimit(double pos, double* speedLimitStartPosition);
	bool isRoadAccessible(Road* pRoad);
	/*
		Checks if the road is part of a multilane road, that is if it is a lane. Road is considered
		a lane if it has at leas one non-null neighbour.

		Returns:
			true -> is lane
			false -> is not lane
	*/
	bool isLane();
	/*
		It is used for lanes. The function checks if the specfied car (pRequester) can enter this
		road, taking into consideration it's position, speed and speed of the cars on this road.
	*/
	bool canEnter(Car* pRequester, Car** ppCarInFront, Car** ppCarBehind);
	/*
		Returns first car behind pCar or nullptr if it is the last car.
	*/
	Car* getCarBehind(Car* pCar);
	/*
		Returns first car after pCar or nullptr if it is the first car.
	*/
	Car* getCarInFront(Car* pCar);
	/*
		Puts the car on this road if the road is a lane. IMPORTANT the method doesn't check if
		there is enough space, or if it is really a lane, so all the check ups must be performed
		before, explicitly.
	*/
	void enterLane(Car* pCar);
	/*
		Tries to find a gap big enough in the section to fit the car. If there is enough space
		then the car is put in the first gap found, if no, then other cars are arranged so that
		there is one gap big enough. The cars are arranged so that the MIN_GAP_MARGIN is always
		maintained between them. If it happens that it's not possible, then the car is not put
		on the road and false is returned. Speeds are not taken into account and after the car
		enters the road it has it's speed changed to speed of the car before or if there is no
		car before then the car after. If there aren't any other cars then speed of the car is
		set to the speed limit.
		
		Returns:
			true	- success
			false	- there were to many cars to put the car on the road
	*/
	bool putCarInSection(Car* pCar, double sectionStart, double sectionEnd);
	bool canPutCarInSection(Car* pCar, double sectionStart, double sectionEnd);
	/*
		Puts the car at the specified position and returns the car in front if any
	*/
	Car* putCarAtPosition(Car* pCar, double position);
	int getCarIdx(Car* pCar);
	bool isFree(double start, double end);
	// getters && stters //////////////////////////////////////////////////////////////////////////
	list<Car*>& getCars();
	double getLength();
	JunctionInput* getJunctionInput();
	void setLeftNeighbour(Road* pNeighbour);
	Road* getLeftNeighbour();
	void setRightNeighbour(Road* pNeighbour);
	Road* getRightNeighbour();
	bool isLaneAccessible(Road* pLane);
	double getOptimalTime();
	void setOptimalTime(double time);

private:
	// constructors ///////////////////////////////////////////////////////////////////////////////
	Road(double length, JunctionInput* pJunctionInput, vector<SpeedLimit> speedLimits);
	// fields /////////////////////////////////////////////////////////////////////////////////////
	double length;						// total length of the road
	JunctionInput* pJunctionInput;		// gate at the end of the road - connector to next road
	vector<SpeedLimit> speedLimits;		// speed limits on particular sections
	list<Car*> cars;					// cars are sorted by position (last element is the closest to the start)
	Road* pLeftNeighbour;				// if is part of a multi-lane road, then the lane to the left
	Road* pRightNeighbour;				// is is part of a mulit-lane road, then the lane to the right
	double optimalTime;

	// methods ////////////////////////////////////////////////////////////////////////////////////
	int idxOfSpeedLimitAt(double pos);
	bool canEnterConsideringCarInFront(Car** ppCarInFront, Car* pRequester);
	bool canEnterConsideringCarBehind(Car** ppCarBehind, Car* pRequester);
	bool isRequiredDistanceMaintained(Car* pCarBehind, Car* pCarInFront);
	/*
		Returns all cars which are at least partly in the section and also the first car before the
		section and the first car after. The cars are ordered - [0] is the car which is the closest
		to the beginning of the road.
	*/
	vector<Car*> getCarsInSectionWithSurroundings(double start, double end);
	/*
		Returns:
			-1 - the car is before the section
			0 - the car is in the section
			1 - the car is after the section
	*/
	int isCarInSection(Car* pCar, double sectionStart, double sectionEnd);
	/*
		Checks if it is possible to arrange the cars so that the pCar can fit between.
	*/
	bool canPutCarAmongCars(Car* pCar, vector<Car*>& cars, double sectionLen);
	bool putCarInFirstGap(Car* pCar, vector<Car*>& relevantCars, double start, double end);
	double getBigEnoughGapStart(vector<Car*>& carsInSection, double start, double end, 
		double minSpace);
	void rearrangeToMakeGap(vector<Car*>& relevantCars, double gapPosition, double gapSize);
	void squeezeCarForward(Car* pCar, double distance);
	void initOptimalTime();
};



class RoadCreationResult
{
public:
	// Error constants /////////////////////////////////////////////////////////
	static const int NOT_CONTINUOUS_SPEED_LIMITS = 1;
	static const int NOT_ENOUGH_SPEED_LIMITS = 2;
	static const int TOO_SHORT_LENGTH = 3;
	static const int NULL_JUNCTION_INPUT = 4;
	static const int NOT_WHOLE_ROAD_COVERED = 5;

	// Constructors ////////////////////////////////////////////////////////////
	RoadCreationResult(Road* pRoad, bool ok = true, int error = 0) 
		: pRoad(pRoad), ok(ok), error(error) {};

	// methods /////////////////////////////////////////////////////////////////
	Road* getRoad()
	{
		return pRoad;
	}

	bool isOk()
	{
		return ok;
	}

	int getError()
	{
		return error;
	}

private:
	Road* pRoad;
	bool ok;
	int error;
};

