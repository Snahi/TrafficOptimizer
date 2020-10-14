#pragma once
#include <vector>
#include "../junction/Junction.h"
#include "../road/Road.h"
#include "lights/Lights.h"



class RoundaboutOutput;
class CarOnRoundabout;



// CONSTANTS //////////////////////////////////////////////////////////////////////////////////////
#define ROAD_WIDTH 3.2	// width of a road, used to prevent inputs from being too close
#define MAX_ROUNDABOUT_SPEED 12.0
#define MIN_ROUNDABOUT_LENGTH 15.0
#define MOCK_ROAD_LEN 1000000	// mock road is the roundabout road, just to keep cars
#define MOCK_ROAD_SPEED_LIMIT 1.0
#define MIN_GAP_ROUNDABOUT 2.5	// minimum gap between cars on a roundabout, speed doesn't matter here
// tick
#define TICK_SUCCESS 0
#define TICK_CAR_BEYOND_ROUTE 1
// common for add input and add output
#define NEGATIVE_POSITION 1000
#define TOO_BIG_POSITION 1001
#define POSITION_NOT_AVAILABLE 1003
// add input
#define ADD_INPUT_SUCCESS 0
// add output
#define ADD_OUTPUT_SUCCESS 0
#define ADD_OUTPUT_NON_UNQ_GATE 1
#define ADD_OUTPUT_NULL_GATE 2




/*
	Class for representing roundabouts with a single lane. More complex and bigger ones can be 
	combined from other elements. Theoretically, simple roundabouts could also be combined from
	other parts, but becuase the distances are small it could not work that well. This class brakes
	rules a little, because it takes control from cars. Instead of leting them drive on the 
	roundabout road (mock road) and taking decisions when to leave by themselves it just puts them 
	there and then it calculates how much time the car must spend on the roundabout. During every 
	iteration the roundabout subtracts that time and once it reaches 0 the car leaves the 
	roundabout if output gate allows it. To get onto a roundabout it is necessary to use a
	RoundaboutGate. Outputs can be any gates.
*/
class Roundabout : public Junction
{
public:
	// constructors ///////////////////////////////////////////////////////////////////////////////
	Roundabout(double length, double speed);
	~Roundabout();
	// methods ////////////////////////////////////////////////////////////////////////////////////
	void reset();
	/*
		Counts down for every car on the roundabout when to leave and takes car of all the things
		required to leave the roundabout.
		Returns:
			TICK_SUCCESS - success
			TICK_CAR_BEYOND_ROUTE - serious error, car route is finished even though it shouldn't
	*/
	int tick(double tickDuration);
	/*
		Creates a new input and if the method succeeds then the id of the newly created input will
		be put into pId.

		Returns:
		ADD_INPUT_SUCCESS		- success
		NEGATIVE_POSITION		- the specified position was negative
		TOO_BIG_POSITION		- the specified position was bigger than the roundabout's length
		POSITION_NOT_AVAILABLE	- there is some other input or output which would collide with the 
								  new input
	*/
	int addInput(double position, int* pId = nullptr);
	/*
		Creates a new output.

		Returns:
		ADD_OUTPUT_SUCCESS		- success
		NEGATIVE_POSITION		- the specified position was negative
		TOO_BIG_POSITION		- the specified position was bigger than the roundabout's length
		POSITION_NOT_AVAILABLE	- there is some other input or output which would collide with the 
								  new input
		ADD_OUTPUT_NON_UNQ_GATE - the specified gate is already connected to the roundabout
		ADD_OUTPUT_NULL_GATE	- the specified gate was null
	*/
	int addOutput(double position, Gate* pGate);
	bool canPutCar(Car* pCar);
	bool putCar(CarOnRoundabout& car);
	/*
		Calculates how long it takes to move from the specified postion to the specified output 
		road with the roundabout speed. Returns -1 if the road is not in output roads.
	*/
	double getTimeToOutput(double fromPos, Road* pGoalRoad);
	bool hasOutputs();

	// getters && setters /////////////////////////////////////////////////////////////////////////
	Road* getMockRoad();
	Gate* getOutputGate(Road* pRoad);
	double getSpeed();
	vector<CarOnRoundabout> getCarsOnRoundabout();
	
private:
	double length;
	double speed;
	/*
		The only required thing for input is to know its id and the place where it meets the 
		roundabout road, because roundabouts don't need to know about the input roads. Index in
		vector corresponds to the id of the input and the actual value is the position.
	*/
	std::vector<double> inputs;
	/*
		gates to the roads through which cars will leave the roundabout
	*/
	std::vector<RoundaboutOutput*> outputs;
	std::vector<CarOnRoundabout> cars;
	Road* pMockRoad;	// artificial road for keeping the cars on roundabout
	JunctionInput mockJunctionInput;
	double freeSpace;	// how much free space is left for new cars

	// methods ////////////////////////////////////////////////////////////////////////////////////
	int validatePosition(double position);
	bool overlaps(double position1, double position2);
	int validateGate(Gate* pGate);
	Gate* getGateAt(double position);
};



class RoundaboutOutput
{
public:
	RoundaboutOutput(double position, Gate* pGate) : position{ position }, pGate{ pGate } {}
	~RoundaboutOutput() { if (pGate != nullptr) delete pGate; }
	double getPosition() { return position; }
	Gate* getGate() { return pGate; }
	Road* getRoad() { return pGate->getRoad(); }
private:
	double position;
	Gate* pGate;
};



/*
	Wrapper for cars which are currently on a roundabout, adds the possibility of storing required
	time to leave the roundabout.
*/
class CarOnRoundabout
{
public:
	CarOnRoundabout(Car* pCar, double timeLeft, Gate* pOutputGate) : pCar{ pCar }, 
		timeLeft{ timeLeft }, pOutputGate{ pOutputGate } {}
	/*
		Returns true if time left is 0 or less
	*/
	bool tick(double tickDuration) { timeLeft -= tickDuration; return timeLeft <= 0.0; }
	Car* getCar() { return pCar; }
	double getLength() { return pCar->getLength(); }
	Gate* getOutputGate() { return pOutputGate; }
	double getTimeLeft() { return timeLeft; }
private:
	Car* pCar;
	double timeLeft;
	Gate* pOutputGate;
};
