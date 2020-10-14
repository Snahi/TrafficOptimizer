#include "../../pch.h"
#include "LSHADEWGD.h"
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
const int LSHADEWGD::DEFAULT_R_N_INIT = 18;
const double LSHADEWGD::DEFAULT_R_ARC = 2.6;
const double LSHADEWGD::DEFAULT_P = 0.11;
const int LSHADEWGD::DEFAULT_H = 6;
const double LSHADEWGD::NORM_VARIANCE = 0.25;
const double LSHADEWGD::CAUCHY_SCALE = 0.1;
// initial evolving parameters values
const double LSHADEWGD::INITIAL_CR = 0.5;
const double LSHADEWGD::INITIAL_F = 0.5;
// constraints
const int LSHADEWGD::MIN_POPULATION_SIZE = 4;
const int LSHADEWGD::MIN_PROBLEM_SIZE = 2;
// other
const int LSHADEWGD::MAX_NUM_OF_EVALUATIONS_NOT_SET = -1;
const int LSHADEWGD::DEFAULT_GENE_HISTORY_SIZE = 12;
const double LSHADEWGD::DEFAULT_GQ_IMPORTANCE = 0.1;
const int LSHADEWGD::DEFAULT_EVALUATIONS_MULTIPLIER = 10000;
const double LSHADEWGD::TERMINAL_CR_VALUE = -1.0;
const double LSHADEWGD::MIN_FITNESS_DIFF_FOR_SUCCESS = 0.000000001;
// state variables names
const string LSHADEWGD::NUM_OF_INDIVIDUALS_NAME = LSHADE::NUM_OF_INDIVIDUALS_NAME;
const string LSHADEWGD::NUM_OF_INDIVIDUALS_IN_ARCHIVE_NAME = LSHADE::NUM_OF_INDIVIDUALS_IN_ARCHIVE_NAME;
const string LSHADEWGD::NUM_OF_SUCCESSFUL_GENERATIONS_NAME = LSHADE::NUM_OF_SUCCESSFUL_GENERATIONS_NAME;
const string LSHADEWGD::CR_NAME = LSHADE::CR_NAME;
const string LSHADEWGD::F_NAME = LSHADE::F_NAME;



// constructors ///////////////////////////////////////////////////////////////////////////////////



LSHADEWGD::LSHADEWGD(Evaluator* pEvaluator, int maxNumOfEvaluations, double rNInit, double rArc, 
	double p, int h, int geneHistorySize, double gqImportance) : Algorithm(pEvaluator)
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
	
	// LSHADEWGD
	this->modifiedGenes = nullptr;
	this->weakGenesHistory = nullptr;
	this->weakGenesHistInsIdxes = nullptr;
	this->geneHistorySize = geneHistorySize;
	this->gqImportance = gqImportance;
}



// destructor /////////////////////////////////////////////////////////////////////////////////////



LSHADEWGD::~LSHADEWGD()
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

	if (modifiedGenes != nullptr)
		delete[] modifiedGenes;

	if (weakGenesHistory != nullptr)
		delete[] weakGenesHistory;

	if (weakGenesHistInsIdxes != nullptr)
		delete[] weakGenesHistInsIdxes;
}



// methods ////////////////////////////////////////////////////////////////////////////////////////
// initialization /////////////////////////////////////////////////////////////////////////////////



InitializeResult LSHADEWGD::initialize()
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



void LSHADEWGD::initFields()
{
	initNumOfIndividuals = numOfIndividuals = (int) round(rNInit * individualSize);
	/*
		population is physically stored in 1D array, so the actual size must be
		multiplied by individual size.
	*/
	int populationSize = numOfIndividuals * individualSize;
	population = new double[populationSize];

	fitness = new double[numOfIndividuals];

	maxNumOfIndividualsInArchive = (int) round(rArc * individualSize);
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

	// LSHADEWGD
	modifiedGenes = new bool[populationSize];
	int weakGenesHistorySize = populationSize * geneHistorySize;
	weakGenesHistory = new int[weakGenesHistorySize];
	// init weakGenesHistory with 0s
	for (int i = 0; i < weakGenesHistorySize; ++i)
		weakGenesHistory[i] = 0;
	weakGenesHistInsIdxes = new int[populationSize];
	// init weakGenesHistInsIdxes with 0
	for (int i = 0; i < populationSize; ++i)
		weakGenesHistInsIdxes[i] = 0;
}



void LSHADEWGD::initPopulation()
{
	obtainConstraints(&minIndividual, &maxIndividual);

	// generate a random population
	EvoRand::randomPopulationConstrained(population, minIndividual, maxIndividual, numOfIndividuals,
		individualSize);

	// evaluate
	pEvaluator->evaluate(population, numOfIndividuals, individualSize, fitness);
}



bool LSHADEWGD::areStaticParametersValid()
{
	if (rNInit <= 0)
		return false;

	if (rArc <= 0)
		return false;

	if (p <= 0.0 || p > 1.0)
		return false;

	if (h <= 0)
		return false;

	if (geneHistorySize < 1)
		return false;

	if (gqImportance < 0.0 || gqImportance > 1.0)
		return false;

	return true;
}



// iterate ////////////////////////////////////////////////////////////////////////////////////////



IterateResult LSHADEWGD::iterateInner()
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



void LSHADEWGD::assignEvolvedParameters()
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



void LSHADEWGD::mutate()
{
	Mutation::DECurrentToPBestArch1(population, fitness, mutants, fs, p, numOfIndividuals, individualSize,
		archive, numOfIndividualsInArchive, minIndividual, maxIndividual);
}



void LSHADEWGD::crossover()
{
	Crossover::DEBinomialWGD(population, mutants, offspring, numOfIndividuals, individualSize, crs, 
		weakGenesHistory, geneHistorySize, gqImportance, modifiedGenes);
}



bool LSHADEWGD::select()
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

			updateWeakGenesHistory(i, 1);
		}
		else
			updateWeakGenesHistory(i, 0);
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



void LSHADEWGD::updateWeakGenesHistory(int indivIdx, int historyInsert)
{
	int realIndivIdx = indivIdx * individualSize;
	int realIndivGeneHistoryIdx = realIndivIdx * geneHistorySize;
	int insIdx;
	for (int geneIdx = 0; geneIdx < individualSize; ++geneIdx)
	{
		if (modifiedGenes[realIndivIdx + geneIdx])
		{
			insIdx = weakGenesHistInsIdxes[realIndivIdx + geneIdx];
			weakGenesHistory[realIndivGeneHistoryIdx + (geneIdx * geneHistorySize) + // first idx of history
				insIdx] = historyInsert;
			// modulo increase history insert idx
			weakGenesHistInsIdxes[realIndivIdx + geneIdx] = (insIdx + 1) % geneHistorySize;
		}
	}
}



void LSHADEWGD::evolveParameters()
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



void LSHADEWGD::resize()
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



double LSHADEWGD::getBestSolution(double* pBest)
{
	double fitnessValue;
	int bestIdx = EvoArray::idxOfBest(fitness, numOfIndividuals, &fitnessValue);

	if (pBest != nullptr)
		EvoArray::copy(population + (bestIdx * individualSize), pBest, individualSize);

	return pEvaluator->inverse(fitnessValue);
}



void LSHADEWGD::getPopulation(double** pPopulation, int* pPopulationSize,
	int* pIndividualSize)
{
	*pPopulation = population;
	*pPopulationSize = numOfIndividuals;
	*pIndividualSize = individualSize;
}



void LSHADEWGD::getState(std::unordered_map<std::string, double>& state)
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



void LSHADEWGD::getParamsNames(std::vector<std::string>& names)
{
	Algorithm::getParamsNames(names);

	names.push_back(LSHADEWGD::NUM_OF_INDIVIDUALS_NAME);
	names.push_back(LSHADEWGD::NUM_OF_INDIVIDUALS_IN_ARCHIVE_NAME);
	names.push_back(LSHADEWGD::NUM_OF_SUCCESSFUL_GENERATIONS_NAME);
	for (int i = 0; i < h; ++i)
	{
		names.push_back(LSHADEWGD::CR_NAME + to_string(i));
	}

	for (int i = 0; i < h; ++i)
	{
		names.push_back(LSHADEWGD::F_NAME + to_string(i));
	}
}