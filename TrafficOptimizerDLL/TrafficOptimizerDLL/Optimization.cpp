#include "pch.h"
#include "Optimization.h"
#include "optimizer/utils/ArrayUtils.h"


using namespace std;



// constructors ///////////////////////////////////////////////////////////////////////////////////



Optimization::Optimization(ModelInterface* pModel)
{
	this->pModel = pModel;
	pEvaluator = nullptr;
	pStopCondition = nullptr;
	pRecorder = nullptr;
	pAlgorithm = nullptr;
	pRunThread = nullptr;
	pOptimizer = nullptr;
	progress = 0.0;
	errCode = ERR_CODE_NOT_SET;
	finished = false;
	bestFitness = DBL_MAX;
	bestSolution = nullptr;
	solutionSize = 0;
	bestFitnessKnown = false;
}



// destructor /////////////////////////////////////////////////////////////////////////////////////



Optimization::~Optimization()
{
	if (pRunThread != nullptr)
	{
		if (pRunThread->joinable())
		{
			try
			{
				pRunThread->join();
			}
			catch (system_error)
			{
				return;
			}
		}

		delete pRunThread;
	}

	if (pStopCondition != nullptr)
		delete pStopCondition;

	if (pAlgorithm != nullptr)
		delete pAlgorithm;

	if (pRecorder != nullptr)
		delete pRecorder;

	if (bestSolution != nullptr)
		delete[] bestSolution;

	// evaluator is deleted in algorithm, model is deleted in evaluator
}



// methods ////////////////////////////////////////////////////////////////////////////////////////



bool Optimization::run()
{
	if (pAlgorithm == nullptr)
		return false;

	// obtain variable params names & initialize variableParams with empty lists
	vector<string> paramsNames;
	pAlgorithm->getParamsNames(paramsNames);
	for (string& name : paramsNames)
	{
		variableParamsNames.push_back(name.c_str());
		variableParams[name] = vector<double>();
	}

	// start the optimization
	pRunThread = new thread(&Optimization::runThreadFun, this);

	return true;
}



bool Optimization::stop()
{
	if (pOptimizer == nullptr)
		return false;

	try
	{
		mutOptiFinished.lock();
	}
	catch (system_error)
	{
		return false;
	}

	bool res;
	if (finished)
	{
		res = true;
		mutOptiFinished.unlock();
	}
	else
	{
		if (pOptimizer != nullptr)
		{
			res = pOptimizer->stop();
			mutOptiFinished.unlock();
			if (res)
			{
				if (pRunThread != nullptr && pRunThread->joinable())
				{
					try
					{
						pRunThread->join();
					}
					catch (system_error)
					{
						res = false;
					}
				}
			}
		}
		else
		{
			mutOptiFinished.unlock();
			res = true;
		}
	}

	return res;
}



bool Optimization::saveCurrentResult(const char* path)
{
	if (pRecorder == nullptr)
		return false;

	try
	{
		mutOptiFinished.lock();
	}
	catch (system_error)
	{
		return false;
	}

	if (pRecorder != nullptr) // in case the optimization was already finished
	{
		try
		{
			mutVariableParams.lock();
		}
		catch (system_error)
		{
			mutOptiFinished.unlock();
			return false;
		}

		bool res = pRecorder->saveResult(path, bestFitness, bestSolution);
		
		mutVariableParams.unlock();
		mutOptiFinished.unlock();
		return res;
	}
	else
	{
		mutOptiFinished.unlock();
		return false;
	}
}



int convertStartOptimizerResultToErrCode(StartOptimizerResult res)
{
	switch (res)
	{
	case StartOptimizerResult::SUCCESS: return Optimization::ERR_CODE_SUCCESS;
	case StartOptimizerResult::RECORDER_COULD_NOT_SAVE: return Optimization::ERR_CODE_SAVE_ERR;
	default:
		return Optimization::ERR_CODE_UNEXPECTED_ERR;
	}
}



void Optimization::runThreadFun()
{
	pOptimizer = new Optimizer(pAlgorithm, pRecorder);
	StartOptimizerResult res = pOptimizer->start(pStopCondition, this, this);
	try
	{
		mutOptiFinished.lock();
	}
	catch (system_error)
	{
		delete pOptimizer;
		return;
	}

	finished = true;
	errCode = convertStartOptimizerResultToErrCode(res);

	// get result
	if (errCode == Optimization::ERR_CODE_SUCCESS)
	{
		double* bestIndividual = new double[pModel->getProblemSize()];
		pAlgorithm->getBestSolution(bestIndividual);
		pModel->translate(bestIndividual, resultDurations, resultOffsets);
		delete[] bestIndividual;
	}

	delete pOptimizer;

	// mark them so that Optimization knows that they have been already deleted (in pOptimizer)
	pRecorder = nullptr;
	pAlgorithm = nullptr;

	mutOptiFinished.unlock();
}



void Optimization::progressChanged(double progress)
{
	try
	{
		mutProgress.lock();
	}
	catch (system_error e)
	{
		return;
	}

	this->progress = progress <= 1.0 ? progress : 1.0;

	mutProgress.unlock();
}



void Optimization::stateChanged(std::unordered_map<string, double>& state, double* bestSolution)
{
	try
	{
		mutVariableParams.lock();
	}
	catch (system_error e)
	{
		return;
	}

	for (auto& pairState : state)
	{
		variableParams[pairState.first].push_back(pairState.second);
	}

	this->bestFitness = state[Algorithm::BEST_FITNESS_NAME];
	bestFitnessKnown = true;
	EvoArray::copy(bestSolution, this->bestSolution, solutionSize);

	mutVariableParams.unlock();
}



bool Optimization::isFinished()
{
	try
	{
		mutOptiFinished.lock();
	}
	catch (system_error)
	{
		return false;
	}

	bool isFin = finished;
	mutOptiFinished.unlock();

	return isFin;
}



// getters && setters /////////////////////////////////////////////////////////////////////////////



void Optimization::setEvaluator(Evaluator* pEvaluator)
{
	this->pEvaluator = pEvaluator;
}



Evaluator* Optimization::getEvaluator()
{
	return pEvaluator;
}



void Optimization::setStopCondition(StopCondition* pStopCondition)
{
	this->pStopCondition = pStopCondition;
}



void Optimization::setRecorder(Recorder* pRecorder)
{
	this->pRecorder = pRecorder;
}



bool Optimization::setAlgorithm(Algorithm* pAlgorithm)
{
	if (pStopCondition == nullptr || pRecorder == nullptr || pAlgorithm == nullptr)
		return false;

	this->pAlgorithm = pAlgorithm;
	this->pStopCondition->setAlgorithm(pAlgorithm);
	this->pRecorder->setAlgorithm(pAlgorithm);
	this->solutionSize = pAlgorithm->getIndividualSize();
	this->bestSolution = new double[solutionSize];

	return true;
}



Algorithm* Optimization::getAlgorithm()
{
	return pAlgorithm;
}



ModelInterface* Optimization::getModel()
{
	return pModel;
}



double Optimization::getProgress(bool* pSuccess)
{
	*pSuccess = true;
	try
	{
		mutProgress.lock();
	}
	catch (system_error e)
	{
		*pSuccess = false;
		return 0;
	}

	double ret = progress;

	mutProgress.unlock();

	return ret;
}



bool Optimization::getVariableParamValues(string name, vector<double>& values)
{
	try
	{
		mutVariableParams.lock();
	}
	catch (system_error e)
	{
		return false;
	}

	if (variableParams.find(name) == variableParams.end())
	{
		mutVariableParams.unlock();
		return false;
	}

	vector<double>& valuesTmp = variableParams[name];
	for (double val : valuesTmp)
	{
		values.push_back(val);
	}

	valuesTmp.clear();

	mutVariableParams.unlock();
	return true;
}



vector<std::string>& Optimization::getVariableParamsNames()
{
	return variableParamsNames;
}



double Optimization::getBestFitness(bool* pSuccess)
{
	try
	{
		mutVariableParams.lock();
	}
	catch (system_error)
	{
		return DBL_MAX;
	}

	double best = bestFitness;
	*pSuccess = bestFitnessKnown; // so that user doesn't get DBL_MAX if the optimization didn't finish initialization yet

	mutVariableParams.unlock();

	return best;
}



int Optimization::getErrCode()
{
	try
	{
		mutOptiFinished.lock();
	}
	catch (system_error)
	{
		return ERR_CODE_NOT_SET;
	}

	int code = errCode;
	mutOptiFinished.unlock();

	return code;
}



void Optimization::getDurations(std::unordered_map<int, double>& durations)
{
	try
	{
		mutOptiFinished.lock();
	}
	catch (system_error)
	{
		return;
	}

	for (auto& pairDuration : resultDurations)
		durations[pairDuration.first] = pairDuration.second;

	mutOptiFinished.unlock();
}



void Optimization::getOffsetes(std::unordered_map<int, double>& offsets)
{
	try
	{
		mutOptiFinished.lock();
	}
	catch (system_error)
	{
		return;
	}

	for (auto& pairOffset : resultOffsets)
		offsets[pairOffset.first] = pairOffset.second;

	mutOptiFinished.unlock();
}
