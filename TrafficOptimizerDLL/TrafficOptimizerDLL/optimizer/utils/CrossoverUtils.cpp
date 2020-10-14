#include "../../pch.h"
#include "CrossoverUtils.h"
#include <stdlib.h>     // srand, rand 
#include <time.h>       // time 
#include "ArrayUtils.h"



void Crossover::DEBinomial(double* population, double* mutants, double* offspring, 
	int populationSize,	int individualSize, double crossoverRate)
{
	srand((unsigned int)time(NULL));
	int k = rand() % individualSize; // to assure that mutant gives at least one gene

	int realIdx;
	for (int indivIdx = 0; indivIdx < populationSize; ++indivIdx)
	{
		for (int geneIdx = 0; geneIdx < individualSize; ++geneIdx)
		{
			realIdx = indivIdx * individualSize + geneIdx;
			offspring[realIdx] =
				geneIdx == k || ((double)rand() / RAND_MAX <= crossoverRate)
				?
				mutants[realIdx]
				:
				population[realIdx];
		}
	}
}



void Crossover::DEBinomial(double* population, double* mutants, double* offspring, int populationSize,
	int individualSize, double* crossoverRate)
{
	srand((unsigned int)time(NULL));
	int k = rand() % individualSize; // to assure that mutant gives at least one gene

	int realIdx;
	for (int indivIdx = 0; indivIdx < populationSize; ++indivIdx)
	{
		for (int geneIdx = 0; geneIdx < individualSize; ++geneIdx)
		{
			realIdx = indivIdx * individualSize + geneIdx;
			offspring[realIdx] =
				geneIdx == k || ((double)rand() / RAND_MAX <= crossoverRate[indivIdx])
				?
				mutants[realIdx]
				:
				population[realIdx];
		}
	}
}



void Crossover::DEBinomialWGD(double* population, double* mutants, 
	double* offspring, int populationSize, int individualSize, double* crossoverRate, 
	int* wgh, int geneHistorySize, double gqImportance, bool* modifiedGenes)
{
	srand((unsigned int)time(NULL));
	int k = rand() % individualSize; // to assure that mutant gives at least one gene

	/* real index of individual's gene */
	int realIdx;
	/* how many ones in history per current individual and current gene */
	int numOfSuccesses;
	/* idx at which history for a particular gene starts */
	int geneHistoryIdx;
	/* probablity that the current gene is weak */
	double weakGeneProb;
	/* crossover rate taking into accout both, crossoverRate and weakGeneProb */
	double finalCrossoverRate;
	/* how much will be added to crossoverRate due to weakGeneProb */
	double probAdd;
	/* if true -> gene will be taken from mutant, if false -> from population */
	bool fromMutant;
	for (int indivIdx = 0; indivIdx < populationSize; ++indivIdx)
	{
		for (int geneIdx = 0; geneIdx < individualSize; ++geneIdx)
		{
			// calculate weak gene probability, first calculate index of gene history
			geneHistoryIdx = (indivIdx * individualSize * geneHistorySize) + 
				(geneIdx * geneHistorySize);
			// how many ones are in genes history
			numOfSuccesses = EvoArray::sum((int*) (wgh + geneHistoryIdx), geneHistorySize);
			// calculate the actual probability
			weakGeneProb = (double)numOfSuccesses / geneHistorySize;
			// calculate the final crossover rate
			probAdd = (1.0 - crossoverRate[indivIdx]) * gqImportance * weakGeneProb;
			finalCrossoverRate = crossoverRate[indivIdx] + probAdd;

			realIdx = indivIdx * individualSize + geneIdx;
			fromMutant = geneIdx == k || ((double)rand() / RAND_MAX <= finalCrossoverRate);
			offspring[realIdx] = fromMutant	? mutants[realIdx] : population[realIdx];
			modifiedGenes[realIdx] = fromMutant;
		}
	}
}