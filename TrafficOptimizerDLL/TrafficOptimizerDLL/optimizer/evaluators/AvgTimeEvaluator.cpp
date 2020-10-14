#include "../../pch.h"
#include "AvgTimeEvaluator.h"



// const //////////////////////////////////////////////////////////////////////////////////////////
const double AvgTimeEvaluator::READY_CAR_PENALTY = 0.1;



int AvgTimeEvaluator::runModel()
{
	return pModel->run(tickDuration, timeoutSeconds);
}



double AvgTimeEvaluator::getFitness()
{
	double avg = pModel->getAvgTime();
	// add a penalty for cars which were unable to enter a road
	int numOfReadyCars = pModel->getNumOfReadyCars();
	double penaltyAvg = numOfReadyCars * avg * AvgTimeEvaluator::READY_CAR_PENALTY;
		

	return -(avg + penaltyAvg);
}



double AvgTimeEvaluator::inverse(double evaluatedFitness)
{
	return -evaluatedFitness;
}