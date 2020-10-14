#pragma once
#include "Evaluator.h"



class AdditionalTimeEvaluator : public Evaluator 
{
public:
	// const //////////////////////////////////////////////////////////////////////////////////////
	static const double READY_CAR_PENALTY;

protected:
	// metods /////////////////////////////////////////////////////////////////////////////////////
	virtual int runModel() override;
	virtual double getFitness() override;
	virtual double inverse(double evaluatedFitness) override;
};
