#pragma once
#include "../evaluators/Evaluator.h"
#include <vector>
#include <string>
#include <utility>
#include <unordered_map>



enum class InitializeResult
{
	SUCCESS,
	TOO_SMALL_PROBLEM,
	INVALID_PARAMETERS
};



enum class IterateResult
{
	SUCCESS,
	NOT_INITIALIZED,
	EVALUATE_ERROR
};



class Algorithm
{
public:
	// const //////////////////////////////////////////////////////////////////////////////////////
	static const std::string GENERATION_NUMBER_NAME;
	static const std::string NUM_OF_EVALUATIONS_NAME;
	static const std::string BEST_FITNESS_NAME;

	// constructors ///////////////////////////////////////////////////////////////////////////////
	Algorithm(Evaluator* pEvaluator);
	~Algorithm();

	// methods ////////////////////////////////////////////////////////////////////////////////////
	virtual InitializeResult initialize() = 0;
	IterateResult iterate();
	/*
		@pBest - pointer to a vector in which the best solution will be stored
		
		Returns fitness of the best solution.
	*/
	virtual double getBestSolution(double* pBest) = 0;
	virtual void getPopulation(double** pPopulation, int* pPopulationSize, 
		int* pIndividualSize) = 0;
	virtual void getState(std::unordered_map<std::string, double>& state);
	virtual void getParamsNames(std::vector<std::string>& names);
	int getGenerationNumber();
	int getIndividualSize();
	int getNumOfEvaluations();
	
protected:
	// fields /////////////////////////////////////////////////////////////////////////////////////
	Evaluator* pEvaluator;
	std::pair<double, double>* ranges;
	int individualSize;
	int generationNumber;
	bool isInitialized;
	int numOfEvaluations;

	// methods ////////////////////////////////////////////////////////////////////////////////////
	virtual IterateResult iterateInner() = 0;
	void obtainConstraints(double** pMinIndividual, double** pMaxIndividual);
};
