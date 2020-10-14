#pragma once
#include "EvaluationsSC.h"
#include "../algorithms/Algorithm.h"



class EvaluationsOptSC : public EvaluationsSC
{
public:
	EvaluationsOptSC(int numOfEvaluations, Algorithm* pAlgorithm, double optimum) 
		: EvaluationsSC(numOfEvaluations, pAlgorithm)
	{
		this->optimum = optimum;
	}

	virtual bool notMet() override
	{
		return EvaluationsSC::notMet() && 
			pAlgorithm->getBestSolution(nullptr) > optimum;
	}

private:
	double optimum;
};
