#include "../../pch.h"
#include "Evaluator.h"
#include <limits>



using namespace std;



// constructors ///////////////////////////////////////////////////////////////////////////////////



Evaluator::Evaluator()
{
	pModel = nullptr;
	tickDuration = DEFAULT_TICK_DURATION;
	timeoutSeconds = TIMEOUT_NOT_SET;
}



// destructor /////////////////////////////////////////////////////////////////////////////////////



Evaluator::~Evaluator()
{
	if (pModel != nullptr)
		delete pModel;
}



// methods ////////////////////////////////////////////////////////////////////////////////////////



InitEvaluatorResult Evaluator::init(ModelInterface* pModel, double timeoutSeconds, 
	double tickDuration)
{
	if (pModel == nullptr)
		return InitEvaluatorResult::NULL_MODEL;

	if (this->pModel != nullptr)
		delete this->pModel;

	this->pModel = pModel;

	if (tickDuration <= 0.0)
		return InitEvaluatorResult::NEGATIVE_TICK_DURATION;

	this->tickDuration = tickDuration;
	this->timeoutSeconds = timeoutSeconds;

	return InitEvaluatorResult::SUCCESS;
}



EvaluateResult Evaluator::evaluate(double* individuals, int numOfIndividuals,
	int individualSize, double* fitnessArr)
{
	if (pModel != nullptr)
	{
		int runRes;
		for (int individualIdx = 0; individualIdx < numOfIndividuals; ++individualIdx)
		{
			if (pModel->update(individuals + (individualIdx * individualSize)))
			{
				runRes = runModel();
				if (runRes != RUN_SUCCESS && runRes != RUN_TIME_OUT)
					return EvaluateResult::RUN_MODEL_UNKNOWN_ERROR;

				if (runRes == RUN_TIME_OUT)
					fitnessArr[individualIdx] = -DBL_MAX; // timeout -> assign the worst possible fitness
				else
					fitnessArr[individualIdx] = getFitness();
			}
			else
				return EvaluateResult::COULD_NOT_UPDATE_MODEL;
		}

		return EvaluateResult::SUCCESS;
	}
	else
		return EvaluateResult::NULL_MODEL;
}



EvaluateResult Evaluator::evaluate(double* individual, int individualSize, double* pFitness)
{
	if (pModel != nullptr)
	{
		pModel->update(individual);

		int runRes = runModel();

		if (runRes != RUN_SUCCESS && runRes != RUN_TIME_OUT)
			return EvaluateResult::RUN_MODEL_UNKNOWN_ERROR;

		if (runRes == RUN_TIME_OUT)
			*pFitness = -DBL_MAX; // time out -> assign the worst possible fitness
		else
			*pFitness = getFitness();

		return EvaluateResult::SUCCESS;
	}
	else
		return EvaluateResult::NULL_MODEL;
}



void Evaluator::getRanges(pair<double, double>* ranges)
{
	if (pModel != nullptr)
		pModel->getRanges(ranges);
}



int Evaluator::getProblemSize()
{
	int size = 0;
	if (pModel != nullptr)
		size = pModel->getProblemSize();

	return size;
}