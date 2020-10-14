#include "../../pch.h"
#include "AdditionalTimeEvaluator.h"



// const //////////////////////////////////////////////////////////////////////////////////////////
const double AdditionalTimeEvaluator::READY_CAR_PENALTY = 10;



// methods ////////////////////////////////////////////////////////////////////////////////////////



int AdditionalTimeEvaluator::runModel()
{
	return pModel->run(tickDuration, timeoutSeconds);
}



double AdditionalTimeEvaluator::getFitness()
{
	return -(pModel->getAdditionalTime() + (pModel->getNumOfReadyCars() * READY_CAR_PENALTY));
}



double AdditionalTimeEvaluator::inverse(double evaluatedFitness)
{
	return -evaluatedFitness;
}