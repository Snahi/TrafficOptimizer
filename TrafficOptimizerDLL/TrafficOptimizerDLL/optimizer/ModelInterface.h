#pragma once
#include <string>
#include <unordered_map>



// constants //////////////////////////////////////////////////////////////////////////////////////
#define DEFAULT_TICK_DURATION 0.5
// run
#define RUN_SUCCESS 0
#define RUN_TIME_OUT 1



class ModelInterface
{
public:
	// methods ////////////////////////////////////////////////////////////////////////////////////
	virtual int run(double tickDuration, double timeoutSeconds) = 0;
	virtual double getAvgTime() = 0;
	/*
		Total time spend by each finished car in the simulation minus the time which these cars
		would spend in the simulation with not a single obstacle (perfect drive, no red lights,
		no traffic jams etc.)
	*/
	virtual double getAdditionalTime() = 0;
	virtual int getNumOfFinishedCars() = 0;
	/*
		How many cars are ready to be put on a road, but they can't be put on the road for some
		reason (e.g. traffic jam cover the entire road)
	*/
	virtual int getNumOfReadyCars() = 0;
	/*
		Change the model parameters (lights groups durations and lights systems phases) according
		to the given individual/solution
	*/
	virtual bool update(double* individual) = 0;
	/*
		Returns vector of pairs where the first element is the minimum value and the second is
		the maximum value of the corresponding gene (ranges[i] corresponds to individual[i])
	*/
	virtual void getRanges(std::pair<double, double>* ranges) = 0;
	virtual int getProblemSize() = 0;
	/*
		Converts individual/solution to a form in which groups ids and lights systems ids are 
		connected with their values.
	*/
	virtual void translate(double* individual, std::unordered_map<int, double>& groups, 
		std::unordered_map<int, double>& systems) = 0;
};
