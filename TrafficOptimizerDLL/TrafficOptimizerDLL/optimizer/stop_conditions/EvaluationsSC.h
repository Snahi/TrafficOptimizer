#pragma once
#include "StopCondition.h"
#include "../algorithms/Algorithm.h"



class EvaluationsSC : public StopCondition
{
public:
	EvaluationsSC(int numOfEvaluations, Algorithm* pAlgorithm) : StopCondition(pAlgorithm)
	{
		this->numOfEvaluations = numOfEvaluations;
	}

	virtual bool notMet() override
	{
		return pAlgorithm->getNumOfEvaluations() < numOfEvaluations;
	}

	virtual float progress() override
	{
		return (float)(pAlgorithm->getNumOfEvaluations()) / numOfEvaluations;
	}

private:
	int numOfEvaluations;
};
