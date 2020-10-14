#include "../pch.h"
#include "Optimizer.h"



using namespace std;



// constructors ///////////////////////////////////////////////////////////////////////////////////



Optimizer::Optimizer(Algorithm* pAlgorithm, Recorder* pRecorder)
{
	this->pAlgorithm = pAlgorithm;
	this->pRecorder = pRecorder;
	this->isRunning = true;
}



// destructor /////////////////////////////////////////////////////////////////////////////////////



Optimizer::~Optimizer()
{
	if (pAlgorithm != nullptr)
		delete pAlgorithm;

	if (pRecorder != nullptr)
		delete pRecorder;
}



// methods ////////////////////////////////////////////////////////////////////////////////////////



StartOptimizerResult Optimizer::start(StopCondition* pStopCondition,
	ProgressListener* pProgressListener, StateListener* pStateListener)
{
	if (!isInitializedProperly())
		return StartOptimizerResult::NOT_INITIALIZED_PROPERLY;

	// start the recorder
	StartRecordingResult startRecRes = pRecorder->startRecording();
	if (startRecRes != StartRecordingResult::SUCCESS)
		return processStartRecordingError(startRecRes);

	InitializeResult initRes = pAlgorithm->initialize();
	if (initRes != InitializeResult::SUCCESS)
		return processInitializeError(initRes);

	unordered_map<string, double> algState;
	double* bestSolution = new double[pAlgorithm->getIndividualSize()];
	IterateResult iterateRes;
	RecordResult recRes;
	bool isRunningLocal = true;
	bool runAtLeastOnce = false; // so that best fitness can be updated if stop condition was finished during initialization
	while (isRunningLocal && pStopCondition->notMet()) // in case pTime is specified check if it didn't pass
	{
		runAtLeastOnce = true;

		// perform iteration of the algorithm
		iterateRes = pAlgorithm->iterate();
		if (iterateRes != IterateResult::SUCCESS)
			return processAlgorithmError(iterateRes);

		// store algorithm's state
		recRes = pRecorder->record();
		if (recRes != RecordResult::SUCCESS)
			return processRecordError(recRes);

		// notify progress listener
		if (pProgressListener != nullptr)
			pProgressListener->progressChanged(pStopCondition->progress());

		// notify state listener
		if (pStateListener != nullptr)
		{
			pAlgorithm->getState(algState);
			pAlgorithm->getBestSolution(bestSolution);
			pStateListener->stateChanged(algState, bestSolution);
		}

		// check if stop wasn't ordered
		try
		{
			mutIsRunning.lock();
		}
		catch (system_error)
		{
			return StartOptimizerResult::LOCK_MUTEX;
		}
		isRunningLocal = isRunning;
		mutIsRunning.unlock();
	}

	if (!runAtLeastOnce)
	{
		// notify state listener
		if (pStateListener != nullptr)
		{
			pAlgorithm->getState(algState);
			pAlgorithm->getBestSolution(bestSolution);
			pStateListener->stateChanged(algState, bestSolution);
		}
	}

	delete[] bestSolution;

	// stop the recorder
	StopRecordingResult stopRecRes = pRecorder->stopRecording();
	if (stopRecRes != StopRecordingResult::SUCCESS)
		return processStopRecordingError(stopRecRes);

	return StartOptimizerResult::SUCCESS;
}



bool Optimizer::isInitializedProperly()
{
	return pAlgorithm != nullptr && pRecorder != nullptr;
}



StartOptimizerResult Optimizer::processStartRecordingError(StartRecordingResult err)
{
	switch (err)
	{
	case StartRecordingResult::NULL_ALGORITHM:
		return StartOptimizerResult::NULL_ALGORITHM_IN_RECORDER;
	default:
		return StartOptimizerResult::UNKNOWN_ERROR;
	}
}



StartOptimizerResult Optimizer::processAlgorithmError(IterateResult err)
{
	switch (err)
	{
	case IterateResult::NOT_INITIALIZED:
		return StartOptimizerResult::ALGORITHM_NOT_INITIALIZED;
	default:
		return StartOptimizerResult::UNKNOWN_ERROR;
	}
}



StartOptimizerResult Optimizer::processRecordError(RecordResult err)
{
	return StartOptimizerResult::UNKNOWN_ERROR;
}



StartOptimizerResult Optimizer::processStopRecordingError(StopRecordingResult err)
{
	switch (err)
	{
	case StopRecordingResult::COULD_NOT_WRITE_TO_FILE:
		return StartOptimizerResult::RECORDER_COULD_NOT_SAVE;
	case StopRecordingResult::NULL_POPULATION:
		return StartOptimizerResult::NO_POPULATION;
	default:
		return StartOptimizerResult::UNKNOWN_ERROR;
	}
}



StartOptimizerResult Optimizer::processInitializeError(InitializeResult err)
{
	return StartOptimizerResult::UNKNOWN_ERROR;
}



bool Optimizer::stop()
{
	try
	{
		mutIsRunning.lock();
	}
	catch (system_error)
	{
		return false;
	}

	isRunning = false;

	mutIsRunning.unlock();

	return true;
}