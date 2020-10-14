#pragma once
#include "road/Road.h"
#include "junction/gate/Gate.h"
#include "generator/Generator.h"
#include <vector>

using namespace std;
class Gate;
class Generator;

// const //////////////////////////////////////////////////////////////////////////////////////////////
// default values
#define DEFAULT_GAP 2.0					// default minimum distance that the car will try to maintain to an obstacle
#define DEFAULT_LENGTH 3.5				// default length of the car
#define DEFAULT_SPEED 13.89
#define DEFAULT_AVG_GAS 2.0
#define DEFAULT_MAX_GAS 4.0
#define DEFAULT_AVG_BRAKE -1.0
#define DEFAULT_STRONG_BRAKE -4
#define DEFAULT_MAX_BRAKE -6.0
#define DEFAULT_REACTION_TIME 0.3
#define TIME_GAP_TO_REACTION_RATIO 3.0		// how many times time gap maintained to obstacles is bigger than reaction time
#define POS_RANGE_TO_NEXT_CAR_FACTOR 0.1	// car tries to maintain a specific distance to
											// the preceeding car, but this distance does not have to
											// be perfect, it can vary be the factor (depending on
											// the exact required distance)
#define SPEED_LIMIT_RANGE 0.2				// cars will try to keep their speed in speedLimit +/- SPEED_LIMIT_RANGE 
#define BRAKING_DIST_BEFORE_SPEED_LIMIT	5	// distance at which car wants to have low enough limit for the next speed limit
#define HANDBRAKE_THRESHOLD 0.3				// minimum speed at which the car can use hand brake (immediate stop)
#define MIN_DISTANCE_FOR_ACCEPTING_REQUEST 2.0
// next road
#define NEXT_ROAD_OK 0
#define NEXT_ROAD_CANT_ENTER_ROAD 1
#define NEXT_ROAD_END_OF_ROUTE 2
// put on start
#define PUT_ON_START_OK 0
#define PUT_ON_START_NO_ROADS 1
#define PUT_ON_START_CANT_PUT_ON_ROAD 2
// tick
#define TICK_CAR_SUCCESS 0
#define TICK_CAR_NOT_STARTED 1
// change lane
#define CHANGE_LANE_SUCCESS 0
#define CHANGE_LANE_NO_NEXT_ROAD 1
#define CHANGE_LANE_NOT_LANE 2
#define CHANGE_LANE_NOT_ACCESSIBLE 3
#define CHANGE_LANE_MUST_WAIT 4
#define CHANGE_LANE_MUST_BRAKE_A_LITTLE 5
#define CHANGE_LANE_TOO_EARLY 6


class Road;


class Car
{
public:
	// constructors ///////////////////////////////////////////////////////////////////////////////////
	Car(vector<Road*>* pRoute, 
		double length = DEFAULT_LENGTH, 
		double speed = DEFAULT_SPEED, 
		double avgGas = DEFAULT_AVG_GAS, 
		double maxGas = DEFAULT_MAX_GAS,
		double avgBrake = DEFAULT_AVG_BRAKE,
		double strongBrake = DEFAULT_STRONG_BRAKE,
		double maxBrake = DEFAULT_MAX_BRAKE,
		double minGap = DEFAULT_GAP,
		double reactionTime = DEFAULT_REACTION_TIME);
	// methods ////////////////////////////////////////////////////////////////////////////////////////
	int tick(double duration);
	/*
		Puts the car at the beginning of it's route.
		Returns:
			PUT_ON_START_OK					- success
			PUT_ON_START_NO_ROADS			- the car reached the end of the road
			PUT_ON_START_CANT_PUT_ON_ROAD	- the car cannot enter the next road e.g. because drives too fast
	*/
	int putOnStart();
	double distanceToEndOfRoad();
	Car* clone();
	void removeFromRoad();
	void finish();
	double minRequiredDistanceToObstacle(double brakingRoad);
	Car* getCarBehind();
	void setSpeed(double speed);
	void moveToNextRoad();
	Road* getNextRoad();
	Road* getRoadAtPosition(int position);
	/*
		Moves the car on the next road, ommiting the gate which guards the road
	*/
	bool moveOnNextRoad();
	// statistics
	double getTotalTime();
	double getTotalTimeWaiting();
	double getAvgSpeed();
	// getters & setters
	double getLength();
	double getPosition();
	double getBack();
	void setPosition(double position);
	int getRoadPos();
	/*
		If position is set with this method, then after enetering a next road the position
		won't be set to 0 but will be kept as it is. It is used when joining a road in 
		the middle
	*/
	void setPositionHard(double position);
	vector<Road*>* getRoute();
	double getSpeed();
	double getAcc();
	double getMinGap();
	double getTimeGap();
	void setLength(double length);
	double getAvgBrake();
	double getStrongBrake();
	Road* getFirstRoad();
	Road* getCurrRoad();
	void setEvaluated(bool evaluated);
	bool isEvaluated();
	bool isFinished();
	double getReactionTime();
	void setSourceGenerator(Generator* pGenerator);
	Generator* getSourceGenerator();

private:
	// fields /////////////////////////////////////////////////////////////////////////////////////
	// characteristics
	double length;			// length of the car
	double avgGas;			// normal acceleration
	double maxGas;			// maximum acceleration
	double avgBrake;		// normal brake
	double strongBrake;		// brake to be applied when avgBrake is not enough
	double maxBrake;		// maximum brake
	vector<Road*>* pRoute;	// roads through which the car will go. first road is at index 0.
	double minGap;			// minimum distance the car will keep to the next obstacle
	double reactionTime;
	double timeGap;			// time which the car will try to remain to the current position of the next car
	Generator* pSourceGenerator; // generator which generated this car
	// state
	double speed;			// current speed
	double acc;				// current acceleration
	double position;		// position of the front
	bool isHandbrakeOn;		// true if the handbrake is on -> the car should not change it's speed
	Road* pRoad;			// road on which the car is currently placed
	int roadIdx;			// position of current road in the route
	double timeSinceLastReaction;
	bool evaluated;			// if true it means that statiscitcs will be calculated for that car
	bool finished;			// true if is finished and shouldn't be ticked anymore
	bool hasHardPosition;	// to let know the car to not set it's position when entering a road
	// requests
	Car* pYieldRequester;
	Car* pYieldRequestRecipient;
	// statistics
	double totalTime;			// time since generation until finishing
	double totalWaitingTime;	// time spent with 0 speed
	double speedSum;			// for calculating avg speed, speed is summed there at every iter.
	int speedMeasurements;		// for calculating avg speed, how many measurements were taken
	// methods ////////////////////////////////////////////////////////////////////////////////////
	void adjustAcc(int changeLaneRes);
	double desiredAccToNextCar();
	double desiredAccSpeedLimit();
	double desiredAccJunction();
	double desiredAccYieldRequest();
	bool isInRangeFactor(double valueToCheck, double exactValue, double rangeFactor);
	bool isInRange(double valueToCheck, double exactValue, double delta);
	bool pullUpHandbrake();
	void pullDownHandbrake();
	/*
		If the next road in route is a lane, then try to enter that lane (at some point the car
		will be able to enter it, but not necessarily during the first call).

		Returns:
		CHANGE_LANE_SUCCESS				- success
		CHANGE_LANE_NO_NEXT_ROAD		- there are no more roads on this car's route
		CHANGE_LANE_NOT_LANE			- the next road is not a lane
		CHANGE_LANE_NOT_ACCESSIBLE		- the lane is not accessible from the current lane
		CHANGE_LANE_MUST_WAIT			- there is a blocking car behind, request was sent, just wait
		CHANGE_LANE_MUST_BRAKE_A_LITTLE - there is a blocking car in front, car should brake
	*/
	int changeLane();
	/*
		Moves the car on the next road if the car haven't reached the end of its route yet.
		Returns:
			NEXT_ROAD_OK				- success
			NEXT_ROAD_END_OF_ROUTE		- end of the route
			NEXT_ROAD_CANT_ENTER_ROAD	- can't put car on the next road
	*/
	int nextRoad();
	bool sendEnterLaneRequest(Car* pRequester);
	bool sendEnterLaneRequestToFurtherCars(Car* pRejecter);
	bool removeYieldRequester(Car* pRequester);
	int reactToCanEnterResult(bool result, Car* pCarBehind, Car* pCarInFront);
	/*
		Evaluates whether it is possible to yield the law of road to the sender of the specified
		request
	*/
	bool canTakeYieldRequest(Car* pRequester);
	void updateStatistics(double timePassed);
};

