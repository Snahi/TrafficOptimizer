#include "../../pch.h"
#include "LSHADE.h"
#include "../utils/RandomUtils.h"
#include "../utils/MutationUtils.h"
#include "../utils/CrossoverUtils.h"
#include "../utils/ArrayUtils.h"
#include "../utils/MathUtils.h"
#include <time.h>
#include <random>
#include <math.h>



using namespace std;



// constants initialization ///////////////////////////////////////////////////////////////////////
// tunded parameters according to L-SHADE authors
const int LSHADE::MAX_NUM_OF_EVALUATIONS_NOT_SET = -1; // value to distinguish if evaluations were not set
const int LSHADE::DEFAULT_EVALUATIONS_MULTIPLIER = 10000; // if evaluations were not set they will be set to this * problem size
const int LSHADE::DEFAULT_R_N_INIT = 18;
const double LSHADE::DEFAULT_R_ARC = 2.6;
const double LSHADE::DEFAULT_P = 0.11;
const int LSHADE::DEFAULT_H = 6;
const double LSHADE::NORM_VARIANCE = 0.1;
const double LSHADE::CAUCHY_SCALE = 0.1;
// initial evolving parameters values
const double LSHADE::INITIAL_CR = 0.5;
const double LSHADE::INITIAL_F = 0.5;
// constraints
const int LSHADE::MIN_POPULATION_SIZE = 4;
const int LSHADE::MIN_PROBLEM_SIZE = 2;
// other
const double LSHADE::TERMINAL_CR_VALUE = -1.0;
const double LSHADE::MIN_FITNESS_DIFF_FOR_SUCCESS = 0.000000001;
// state variables names
const string LSHADE::NUM_OF_INDIVIDUALS_NAME = "num_of_individuals";
const string LSHADE::NUM_OF_INDIVIDUALS_IN_ARCHIVE_NAME = "num_of_individuals_in_archive";
const string LSHADE::NUM_OF_SUCCESSFUL_GENERATIONS_NAME = "num_of_successful_generations";
const string LSHADE::CR_NAME = "Cr_";
const string LSHADE::F_NAME = "F_";



// constructors ///////////////////////////////////////////////////////////////////////////////////



LSHADE::LSHADE(Evaluator* pEvaluator, int maxNumOfEvaluations, double rNInit, double rArc, 
	double p, int h) : Algorithm(pEvaluator)
{
	// static parameters
	this->maxNumOfEvaluations = maxNumOfEvaluations > 0 ? maxNumOfEvaluations : 
		pEvaluator->getProblemSize() * DEFAULT_EVALUATIONS_MULTIPLIER;
	this->rNInit = rNInit;
	this->rArc = rArc;
	this->p = p;
	this->h = h;

	// other fields
	this->population = nullptr;
	this->archive = nullptr;
	this->initNumOfIndividuals = 0;
	this->numOfIndividuals = 0;
	this->maxNumOfIndividualsInArchive = 0;
	this->numOfIndividualsInArchive = 0;
	this->crHistory = nullptr;
	this->fHistory = nullptr;
	this->idxForHistoryInsert = 0;
	this->successfulCrs = nullptr;
	this->successfulFs = nullptr;
	this->numOfSuccessfulGenerations = 0;
	this->fitness = nullptr;
	this->minIndividual = nullptr;
	this->maxIndividual = nullptr;
	this->mutants = nullptr;
	this->offspring = nullptr;
	this->offspringFitness = nullptr;
	this->fs = nullptr;
	this->crs = nullptr;
	this->lehmerMeanWeights = nullptr;
}



// destructor /////////////////////////////////////////////////////////////////////////////////////



LSHADE::~LSHADE()
{
	if (population != nullptr)
		delete[] population;

	if (archive != nullptr)
		delete[] archive;

	if (crHistory != nullptr)
		delete[] crHistory;

	if (fHistory != nullptr)
		delete[] fHistory;

	if (successfulCrs != nullptr)
		delete[] successfulCrs;

	if (successfulFs != nullptr)
		delete[] successfulFs;

	if (fitness != nullptr)
		delete[] fitness;

	if (minIndividual != nullptr)
		delete[] minIndividual;

	if (maxIndividual != nullptr)
		delete[] maxIndividual;

	if (mutants != nullptr)
		delete[] mutants;

	if (fs != nullptr)
		delete[] fs;

	if (crs != nullptr)
		delete[] crs;
	
	if (offspring != nullptr)
		delete[] offspring;

	if (offspringFitness != nullptr)
		delete[] offspringFitness;

	if (lehmerMeanWeights != nullptr)
		delete[] lehmerMeanWeights;
}



// methods ////////////////////////////////////////////////////////////////////////////////////////
// initialization /////////////////////////////////////////////////////////////////////////////////



InitializeResult LSHADE::initialize()
{
	if (!areStaticParametersValid())
		return InitializeResult::INVALID_PARAMETERS;

	if (individualSize < MIN_PROBLEM_SIZE)
		return InitializeResult::TOO_SMALL_PROBLEM;

	initFields();
	initPopulation();

	isInitialized = true;

	return InitializeResult::SUCCESS;
}



void LSHADE::initFields()
{
	initNumOfIndividuals = (int) round(rNInit * individualSize);
	initNumOfIndividuals = initNumOfIndividuals < MIN_POPULATION_SIZE ? MIN_POPULATION_SIZE : 
		initNumOfIndividuals;
	numOfIndividuals = initNumOfIndividuals;
	/*
		population is physically stored in 1D array, so the actual size must be
		multiplied by individual size.
	*/
	int populationSize = numOfIndividuals * individualSize;
	population = new double[populationSize];

	fitness = new double[numOfIndividuals];

	maxNumOfIndividualsInArchive = (int) round(rArc * individualSize);
	if (maxNumOfIndividualsInArchive < 1)
		maxNumOfIndividualsInArchive = 1;
	int maxArchiveSize = maxNumOfIndividualsInArchive * individualSize;
	archive = new double[maxArchiveSize];

	crHistory = new double[h];
	fHistory = new double[h];
	for (int i = 0; i < h; ++i)
	{
		crHistory[i] = INITIAL_CR;
		fHistory[i] = INITIAL_F;
	}

	// max #successful crs/fs is the population size (in case all crossovers were successful)
	successfulCrs = new double[numOfIndividuals];
	successfulFs = new double[numOfIndividuals];

	mutants = new double[populationSize];

	offspring = new double[populationSize];
	offspringFitness = new double[numOfIndividuals];

	fs = new double[numOfIndividuals];
	crs = new double[numOfIndividuals];
	lehmerMeanWeights = new double[numOfIndividuals];
}



void LSHADE::initPopulation()
{
	obtainConstraints(&minIndividual, &maxIndividual);

	// generate a random population
	EvoRand::randomPopulationConstrained(population, minIndividual, maxIndividual, numOfIndividuals,
		individualSize);

	// evaluate
	pEvaluator->evaluate(population, numOfIndividuals, individualSize, fitness);
}



bool LSHADE::areStaticParametersValid()
{
	if (rNInit <= 0)
		return false;

	if (rArc <= 0)
		return false;

	if (p <= 0.0 || p > 1.0)
		return false;

	if (h <= 0)
		return false;

	return true;
}



// iterate ////////////////////////////////////////////////////////////////////////////////////////



IterateResult LSHADE::iterateInner()
{
	assignEvolvedParameters();
	mutate();
	crossover();
	if (!select())
		return IterateResult::EVALUATE_ERROR;
	evolveParameters();
	resize();

	return IterateResult::SUCCESS;
}



void LSHADE::assignEvolvedParameters()
{
	srand((unsigned int)time(NULL));
	double cr;
	double f;
	default_random_engine generator((unsigned int)time(NULL));
	for (int i = 0; i < numOfIndividuals; ++i)
	{
		cr = crHistory[rand() % h];
		f = fHistory[rand() % h];
		// normal distribution for the cr parameter
		normal_distribution<double> crDist(cr, NORM_VARIANCE);
		// cauchy distribution for the f parameter
		cauchy_distribution<double> fDist(f, CAUCHY_SCALE);

		crs[i] = cr != TERMINAL_CR_VALUE ? crDist(generator) : 0.0;
		if (crs[i] < 0.0)
			crs[i] = 0.0;
		else if (crs[i] > 1)
			crs[i] = 1.0;

		fs[i] = fDist(generator);
		if (fs[i] < 0.0)
			fs[i] = 0.0;
		else if (fs[i] > 1.0)
			fs[i] = 1.0;
	}
}



void LSHADE::mutate()
{
	Mutation::DECurrentToPBestArch1(population, fitness, mutants, fs, p, numOfIndividuals, individualSize,
		archive, numOfIndividualsInArchive, minIndividual, maxIndividual);
}



void LSHADE::crossover()
{
	Crossover::DEBinomial(population, mutants, offspring, numOfIndividuals, individualSize, crs);
}



bool LSHADE::select()
{
	// evaluate offspring
	EvaluateResult evaluateRes = pEvaluator->evaluate(offspring, numOfIndividuals, individualSize, 
		offspringFitness);
	if (evaluateRes != EvaluateResult::SUCCESS)
		return false;

	numOfEvaluations += numOfIndividuals;

	numOfSuccessfulGenerations = 0;
	double* pArchivedParent;

	srand((unsigned int)time(NULL));
	for (int i = 0; i < numOfIndividuals; ++i)
	{
		if (offspringFitness[i] > fitness[i] + MIN_FITNESS_DIFF_FOR_SUCCESS)
		{
			// move the parent to archive
			if (numOfIndividualsInArchive < maxNumOfIndividualsInArchive)
			{
				pArchivedParent = archive + (numOfIndividualsInArchive * individualSize);
				++numOfIndividualsInArchive;
			}
			else
			{
				pArchivedParent = archive + ((rand() % maxNumOfIndividualsInArchive) * 
					individualSize);
			}
			EvoArray::copy(population + (i * individualSize), pArchivedParent, individualSize);

			// add to successful crs and successful fs
			successfulCrs[numOfSuccessfulGenerations] = crs[i];
			successfulFs[numOfSuccessfulGenerations] = fs[i];
			lehmerMeanWeights[numOfSuccessfulGenerations] = offspringFitness[i] - fitness[i]; // for now without scale
			
			++numOfSuccessfulGenerations;
		}

		if (offspringFitness[i] >= fitness[i])
		{
			fitness[i] = offspringFitness[i];
			EvoArray::copy(offspring + (i * individualSize), population + (i * individualSize), 
				individualSize);
		}
	}

	// scale leherman weights
	if (numOfSuccessfulGenerations > 0)
	{
		double scaleDenominator = EvoArray::sum(lehmerMeanWeights, numOfSuccessfulGenerations);
		double scale = 1.0 / scaleDenominator;
		EvoArray::multiply(lehmerMeanWeights, scale, lehmerMeanWeights, 
			numOfSuccessfulGenerations);
	}

	return true;
}



void LSHADE::evolveParameters()
{
	if (numOfSuccessfulGenerations > 0)
	{
		if (crHistory[idxForHistoryInsert] == TERMINAL_CR_VALUE ||
			EvoArray::maxValue(successfulCrs, numOfSuccessfulGenerations, nullptr) == 0.0)
		{
			crHistory[idxForHistoryInsert] = TERMINAL_CR_VALUE;
		}
		else
		{
			crHistory[idxForHistoryInsert] = EvoMath::weightedLehmerMean(lehmerMeanWeights, successfulCrs,
				numOfSuccessfulGenerations);
		}
		fHistory[idxForHistoryInsert] = EvoMath::weightedLehmerMean(lehmerMeanWeights, successfulFs,
			numOfSuccessfulGenerations);

		idxForHistoryInsert = (idxForHistoryInsert + 1) % h;
	}
}



void LSHADE::resize()
{
	int newNumOfIndiv = (int)round(
		(((double)MIN_POPULATION_SIZE - initNumOfIndividuals) / (double)maxNumOfEvaluations) *
		numOfEvaluations + initNumOfIndividuals);

	if (newNumOfIndiv >= MIN_POPULATION_SIZE)
	{
		if (newNumOfIndiv < numOfIndividuals) // population shrinks
		{
			int diff = numOfIndividuals - newNumOfIndiv;
			int* worstIdxes = new int[diff];
			EvoArray::worstNIdxes(fitness, numOfIndividuals, worstIdxes, diff);
			// erase from population
			EvoArray::eraseRows(population, numOfIndividuals, individualSize, worstIdxes, diff);
			// erase from fitness
			EvoArray::eraseRows(fitness, numOfIndividuals, 1, worstIdxes, diff);

			numOfIndividuals = newNumOfIndiv;
			maxNumOfIndividualsInArchive = (int)round((rArc / rNInit) * numOfIndividuals);
			if (maxNumOfIndividualsInArchive < 1)
				maxNumOfIndividualsInArchive = 1;
			if (numOfIndividualsInArchive > maxNumOfIndividualsInArchive)
				numOfIndividualsInArchive = maxNumOfIndividualsInArchive;

			delete[] worstIdxes;
		}
	}
}



// other //////////////////////////////////////////////////////////////////////////////////////////



double LSHADE::getBestSolution(double* pBest)
{
	double fitnessValue;
	int bestIdx = EvoArray::idxOfBest(fitness, numOfIndividuals, &fitnessValue);

	if (pBest != nullptr)
		EvoArray::copy(population + (bestIdx * individualSize), pBest, individualSize);

	return pEvaluator->inverse(fitnessValue);
}



void LSHADE::getPopulation(double** pPopulation, int* pPopulationSize,
	int* pIndividualSize)
{
	*pPopulation = population;
	*pPopulationSize = numOfIndividuals;
	*pIndividualSize = individualSize;
}



void LSHADE::getState(std::unordered_map<std::string, double>& state)
{
	Algorithm::getState(state);

	state[LSHADE::NUM_OF_INDIVIDUALS_NAME] = numOfIndividuals;
	state[LSHADE::NUM_OF_INDIVIDUALS_IN_ARCHIVE_NAME] = numOfIndividualsInArchive;
	state[LSHADE::NUM_OF_SUCCESSFUL_GENERATIONS_NAME] = numOfSuccessfulGenerations;
	for (int i = 0; i < h; ++i)
	{
		state[LSHADE::CR_NAME + to_string(i)] = crHistory[i];
		state[LSHADE::F_NAME + to_string(i)] = fHistory[i];
	}
}



void LSHADE::getParamsNames(std::vector<std::string>& names)
{
	Algorithm::getParamsNames(names);

	names.push_back(LSHADE::NUM_OF_INDIVIDUALS_NAME);
	names.push_back(LSHADE::NUM_OF_INDIVIDUALS_IN_ARCHIVE_NAME);
	names.push_back(LSHADE::NUM_OF_SUCCESSFUL_GENERATIONS_NAME);
	for (int i = 0; i < h; ++i)
	{
		names.push_back(LSHADE::CR_NAME + to_string(i));
	}

	for (int i = 0; i < h; ++i)
	{
		names.push_back(LSHADE::F_NAME + to_string(i));
	}
}