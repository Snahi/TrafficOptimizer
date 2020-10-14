#pragma once
#include "Algorithm.h"
#include <string>



class LSHADE : public Algorithm
{
public:
	// constants //////////////////////////////////////////////////////////////////////////////////
	// tunded parameters according to L-SHADE authors
	static const int MAX_NUM_OF_EVALUATIONS_NOT_SET;
	static const int DEFAULT_EVALUATIONS_MULTIPLIER;
	static const int DEFAULT_R_N_INIT;
	static const double DEFAULT_R_ARC;
	static const double DEFAULT_P;
	static const int DEFAULT_H;
	static const double NORM_VARIANCE;
	static const double CAUCHY_SCALE;
	// initial evolving parameters values
	static const double INITIAL_CR;
	static const double INITIAL_F;
	// constraints
	static const int MIN_POPULATION_SIZE;
	static const int MIN_PROBLEM_SIZE;
	// other
	static const double TERMINAL_CR_VALUE;
	static const double MIN_FITNESS_DIFF_FOR_SUCCESS;
	// state variables names
	static const std::string NUM_OF_INDIVIDUALS_NAME;
	static const std::string NUM_OF_INDIVIDUALS_IN_ARCHIVE_NAME;
	static const std::string NUM_OF_SUCCESSFUL_GENERATIONS_NAME;
	static const std::string CR_NAME;
	static const std::string F_NAME;

	// constructors ///////////////////////////////////////////////////////////////////////////////
	LSHADE(Evaluator* pEvaluator, int maxNumOfEvaluations = MAX_NUM_OF_EVALUATIONS_NOT_SET,
		double rNInit = DEFAULT_R_N_INIT, double rArc = DEFAULT_R_ARC, double p = DEFAULT_P, 
		int h = DEFAULT_H);
	~LSHADE();

	// methods ////////////////////////////////////////////////////////////////////////////////////
	/*
		Checks if static paremeters passed during object creation and if the problem size is not
		too small. If everything is valid, then initializes fields and creates the initial 
		population.
	*/
	virtual InitializeResult initialize() override;
	/*
		@pBest - pointer to a vector in which the best solution will be stored

		Returns fitness of the best solution.
	*/
	virtual double getBestSolution(double* pBest) override;
	virtual void getPopulation(double** pPopulation, int* pPopulationSize,
		int* pIndividualSize) override;
	virtual void getState(std::unordered_map<std::string, double>& state) override;
	virtual void getParamsNames(std::vector<std::string>& names) override;

protected:
	// methods ////////////////////////////////////////////////////////////////////////////////////
	virtual IterateResult iterateInner();

private:
	// fields /////////////////////////////////////////////////////////////////////////////////////
	/*
		Array which stores the current population. it is 1D, so in order to access i-th individual
		the index must be calculated as i x individualSize. 
		Its size is numOfIndividuals x individualSize.
	*/
	double* population;
	/*
		Array in which i-th element is i-th's individual's fitness.
	*/
	double* fitness;
	/*
		Number of individuals in the initial population. It is calculated as 
		rNInit * individualSize. It is used to calculate how many individuals should be left in 
		the next generation.
	*/
	int initNumOfIndividuals;
	/*
		Initially it is equal to initNumOfIndividuals but then it decreases. It shows the actual
		number of individuals in the current generation. numOfIndividuals <= initNumOfIndividuals
	*/
	int numOfIndividuals;
	/*
		Array which stores individuals which were parents, but their offspring was better. As in 
		the	case of population it is 1D, so in order to access i-th archived individual one must
		calculate the index as i x individualSize. It is initialized with size equal to 
		maxNumOfIndividualsInArchive x individualSize, but its real size is equal to 
		numOfIndividualsInArchive x individualSize, where 
		numOfIndividualsInArchive <= maxNumOfIndividualsInArchive.
	*/
	double* archive;
	/*
		Maximum number of individuals which can be currently stored in archive. It decreases as
		population decreases. Initially it's value is rArc x individualSize.
	*/
	int maxNumOfIndividualsInArchive;
	int numOfIndividualsInArchive;
	/*
		Array for storing historical values of the global cr parameter. Its size is h. Initially
		every element is equal to INITIAL_CR.
	*/
	double* crHistory;
	/*
		Array for storing historical valus of the global f parameter. Its size is h. Initially
		every element is equalt to INITIAL_F.
	*/
	double* fHistory;
	/*
		idx in crHistory and fHistory at which a new value will be inserted. After inserting
		it is modulo increased.
	*/
	int idxForHistoryInsert;
	/*
		When offspring is better than its parent then the parent's cr parameter is inserted here.
		After all offspring is generated the new global cr parameter is calculated based on this 
		array. It's maximum size is equal to numOfIndividuals, but its real size is equal to 
		numOfSuccessfulGenerations which is equal to the number of offspring which were better
		than their parents in the current generation.
	*/
	double* successfulCrs;
	/*
		When offspring is better than its parent then the parent's f parameter is inserted here.
		After all offspring is generated the new global f parameter is calculated based on this 
		array. It's maximum size is equal to numOfIndividuals, but its real size is equal to
		numOfSuccessfulGenerations which is equal to the number of offspring which were better
		than their parents in the current generation.
	*/
	double* successfulFs;
	/*
		How many children were better than parents during the previous crossover.
	*/
	int numOfSuccessfulGenerations;
	/*
		Array in which i-th element is the minimal acceptable value for i-th gene. It's size is
		individualSize.
	*/
	double* minIndividual;
	/*
		Array in which i-th element is the maximal acceptable value for i-th gene. It's size is
		individualSize.
	*/
	double* maxIndividual;
	/*
		Array of mutated individuals. It's size is equal to the population array's size.
	*/
	double* mutants;
	/*
		Array of offspring generated from crossover between mutants and population. It's size is
		equalt to the population array's size.
	*/
	double* offspring;
	/*
		Array in which i-th element corresponds to i-th offspring's fitness. It's size is 
		numOfIndividuals.
	*/
	double* offspringFitness;
	/*
		Array in which i-th element corresponds to i-th individual's f parameter. It's size is
		numOfIndividuals.
	*/
	double* fs;
	/*
		Array in which i-th element corresponds to i-th individual's cr parameters. It's size is
		numOfIndividuals.
	*/
	double* crs;
	/*
		Array in which i-th element corresponds to i-th successful cr/f's weight during lehmerMean
		calculation. It's maximal size is numOfIndividuals but the actual size is equal to
		numOfSuccessfulGenerations.
	*/
	double* lehmerMeanWeights;

	// static parameters //////////////////////////////////////////////////////////////////////////
	int maxNumOfEvaluations;
	double rNInit;			 // initial population size = problem dimension * rNInit
	double rArc;			 // max archive size = problem dimension * rArc
	double p;				 // p * population size -> how many top individuals are considered for pBest
	int h;					 // number of records in history of Cr and F

	// methods ////////////////////////////////////////////////////////////////////////////////////
	// iteration steps
	void assignEvolvedParameters();
	void mutate();
	void crossover();
	bool select();
	void evolveParameters();
	void resize();
	
	// other
	void initFields();
	bool areStaticParametersValid();
	void initPopulation();
};
