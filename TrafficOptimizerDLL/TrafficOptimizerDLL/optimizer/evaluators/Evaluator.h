#pragma once
#include <string>
#include <vector>
#include "../ModelInterface.h"




enum class InitEvaluatorResult
{
	SUCCESS,
	NULL_MODEL,
	NEGATIVE_TICK_DURATION
};



enum class EvaluateResult 
{
	SUCCESS,
	NULL_MODEL,
	RUN_MODEL_UNKNOWN_ERROR,
	COULD_NOT_UPDATE_MODEL
};



class Evaluator
{
public:
	// const //////////////////////////////////////////////////////////////////////////////////////
	static const int TIMEOUT_NOT_SET = -1;

	// constructors ///////////////////////////////////////////////////////////////////////////////
	Evaluator();
	~Evaluator();

	// methods ////////////////////////////////////////////////////////////////////////////////////
	InitEvaluatorResult init(ModelInterface* pModel, double timeoutSeconds = TIMEOUT_NOT_SET,
		double tickDuration = DEFAULT_TICK_DURATION);
	virtual EvaluateResult evaluate(double* individuals, int numOfIndividuals, int individualSize, 
		double* fitnessArr);
	EvaluateResult evaluate(double* individual, int individualSize, double* pFitness);
	void getRanges(std::pair<double, double>* ranges);
	int getProblemSize();
	virtual double inverse(double evaluatedFitness) = 0;
protected:
	// fields /////////////////////////////////////////////////////////////////////////////////////
	ModelInterface* pModel;
	double tickDuration;
	double timeoutSeconds;

	// methods ////////////////////////////////////////////////////////////////////////////////////
	virtual int runModel() = 0;
	virtual double getFitness() = 0;

};
