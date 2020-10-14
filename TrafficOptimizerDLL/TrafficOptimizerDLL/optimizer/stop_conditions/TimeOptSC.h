#pragma once
#include "TimeSC.h"
#include "../algorithms/Algorithm.h"



class TimeOptSC : public TimeSC
{
public:
	TimeOptSC(double durationSeconds, double optimum) : TimeSC(durationSeconds)
	{
		this->optimum = optimum;
	}

	bool notMet() override
	{
		return TimeSC::notMet() && pAlgorithm->getBestSolution(nullptr) > optimum;
	}

private:
	double optimum;
};