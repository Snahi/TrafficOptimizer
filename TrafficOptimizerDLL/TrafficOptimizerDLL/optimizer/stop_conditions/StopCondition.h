#pragma once
#include "../algorithms/Algorithm.h"



class StopCondition
{
public:
	StopCondition(Algorithm* pAlgorithm)
	{
		this->pAlgorithm = pAlgorithm;
	}
	virtual bool notMet() = 0;
	virtual float progress() = 0;
	void setAlgorithm(Algorithm* pAlgorithm)
	{
		this->pAlgorithm = pAlgorithm;
	}
	Algorithm* getAlgorithm()
	{
		return pAlgorithm;
	}

protected:
	Algorithm* pAlgorithm;
};
