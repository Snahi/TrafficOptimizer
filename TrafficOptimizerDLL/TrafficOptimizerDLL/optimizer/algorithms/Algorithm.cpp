#include "../../pch.h"
#include "Algorithm.h"



using namespace std;



// constants //////////////////////////////////////////////////////////////////////////////////////
const string Algorithm::GENERATION_NUMBER_NAME = "generation_number";
const string Algorithm::NUM_OF_EVALUATIONS_NAME = "num_of_evaluations";
const string Algorithm::BEST_FITNESS_NAME = "best_fitness";



// constructors ///////////////////////////////////////////////////////////////////////////////////



Algorithm::Algorithm(Evaluator* pEvaluator)
{
	generationNumber = 0;
	this->pEvaluator = pEvaluator;
	isInitialized = false;
	numOfEvaluations = 0;

	if (pEvaluator != nullptr)
	{
		individualSize = pEvaluator->getProblemSize();
		ranges = new pair<double, double>[individualSize];
		pEvaluator->getRanges(ranges);
		isInitialized = true;
	}
	else
	{
		individualSize = -1;
		ranges = nullptr;
	}
}



// destructor /////////////////////////////////////////////////////////////////////////////////////



Algorithm::~Algorithm()
{
	if (ranges != nullptr)
		delete[] ranges; // err

	if (pEvaluator != nullptr)
		delete pEvaluator;
}



// methods ////////////////////////////////////////////////////////////////////////////////////////



IterateResult Algorithm::iterate()
{
	if (pEvaluator != nullptr && isInitialized)
	{
		++generationNumber;
		return iterateInner();
	}
	else
		return IterateResult::NOT_INITIALIZED;
}



void Algorithm::getState(std::unordered_map<std::string, double>& state)
{
	state[Algorithm::GENERATION_NUMBER_NAME] = generationNumber;
	state[Algorithm::NUM_OF_EVALUATIONS_NAME] = numOfEvaluations;
	state[Algorithm::BEST_FITNESS_NAME] = getBestSolution(nullptr);
}



void Algorithm::getParamsNames(std::vector<std::string>& names)
{
	names.push_back(Algorithm::GENERATION_NUMBER_NAME);
	names.push_back(Algorithm::NUM_OF_EVALUATIONS_NAME);
	names.push_back(Algorithm::BEST_FITNESS_NAME);
}



int Algorithm::getGenerationNumber()
{
	return generationNumber;
}



int Algorithm::getIndividualSize()
{
	return individualSize;
}



int Algorithm::getNumOfEvaluations()
{
	return numOfEvaluations;
}



void Algorithm::obtainConstraints(double** pMinIndividual, double** pMaxIndividual)
{
	double* minIndividual = new double[individualSize];
	double* maxIndividual = new double[individualSize];
	for (int i = 0; i < individualSize; ++i)
	{
		minIndividual[i] = ranges[i].first;
		maxIndividual[i] = ranges[i].second;
	}

	*pMinIndividual = minIndividual;
	*pMaxIndividual = maxIndividual;
}