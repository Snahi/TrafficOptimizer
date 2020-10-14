#pragma once
#include "algorithms/Algorithm.h"
#include "recorders/Recorder.h"
#include "stop_conditions/StopCondition.h"
#include <unordered_map>
#include <string>
#include <mutex>



class ProgressListener;
class StateListener;



enum class StartOptimizerResult
{
	SUCCESS,
	NOT_INITIALIZED_PROPERLY, // pAlgorithm or pRecorder are null
	UNKNOWN_ERROR,
	NULL_ALGORITHM_IN_RECORDER,
	ALGORITHM_NOT_INITIALIZED,
	RECORDER_COULD_NOT_SAVE,
	NO_POPULATION,
	NULL_EVALUATOR,
	LOCK_MUTEX
};



/*
	Class which manages run of a specific optimization algorithm.
*/
class Optimizer
{
public:
	Optimizer(Algorithm* pAlgorithm, Recorder* pRecorder);
	~Optimizer();
	StartOptimizerResult start(StopCondition* pStopCondition, 
		ProgressListener* pProgressListener = nullptr,
		StateListener* pStateListener = nullptr);
	/*
		Stops the optimisation and stores the results permanently using pRecorder.
	*/
	bool stop();
private:
	// fields /////////////////////////////////////////////////////////////////////////////////////
	Algorithm* pAlgorithm;		// the algorithm to be run
	Recorder* pRecorder;		// stores the optimization state permanently
	/* 
		The optimization will stop if this variable is set to false. mutIsRunning bust be acquired 
		before accessing it. 
	*/
	bool isRunning;				
	std::mutex mutIsRunning;

	// methods ////////////////////////////////////////////////////////////////////////////////////
	StartOptimizerResult processAlgorithmError(IterateResult err);
	StartOptimizerResult processStartRecordingError(StartRecordingResult err);
	StartOptimizerResult processRecordError(RecordResult err);
	StartOptimizerResult processStopRecordingError(StopRecordingResult err);
	StartOptimizerResult processInitializeError(InitializeResult err);
	bool isInitializedProperly();
};



class ProgressListener
{
public:
	virtual void progressChanged(double progress) = 0;
};



class StateListener
{
public:
	virtual void stateChanged(std::unordered_map<std::string, double>& state, 
		double* bestSolution) = 0;
};
