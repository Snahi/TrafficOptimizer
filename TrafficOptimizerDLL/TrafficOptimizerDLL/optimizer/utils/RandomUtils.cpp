#include "../../pch.h"
#include "RandomUtils.h"
#include <stdlib.h>     // srand, rand 
#include <time.h>       // time 



void EvoRand::randomIndividualConstrained(double* individual, double* mins, double* maxes, int size)
{
	srand((unsigned int) time(NULL));
	double factor;
	for (int i = 0; i < size; ++i)
	{
		factor = (double)rand() / RAND_MAX; // 0.0 - 1.0
		individual[i] = mins[i] + factor * (maxes[i] - mins[i]);
	}
}



void EvoRand::randomPopulationConstrained(double* population, double* mins, double* maxes,
	int populationSize, int individualSize)
{
	srand((unsigned int) time(NULL));
	double factor;
	int reps = populationSize - 2;
	for (int individualIdx = 0; individualIdx < reps; ++individualIdx)
	{
		for (int geneIdx = 0; geneIdx < individualSize; ++geneIdx)
		{
			factor = (double)rand() / RAND_MAX; // 0.0 - 1.0
			population[individualIdx * individualSize + geneIdx] = mins[geneIdx] + 
				factor * (maxes[geneIdx] - mins[geneIdx]);
		}
	}

	// add the minimal and maximal individual
	for (int geneIdx = 0; geneIdx < individualSize; ++geneIdx)
	{
		population[reps * individualSize + geneIdx] = mins[geneIdx];
		population[(reps + 1) * individualSize + geneIdx] = maxes[geneIdx];
	}
}



int EvoRand::distinctRand(int* excluded, int size, int min, int max)
{
	srand((unsigned int)time(NULL));
	int res;
	bool unq = false;
	int i;
	while (!unq) // until generates a random number which is not in excluded[]
	{
		res = min + (rand() % (max - min)); // random in range [min; max]
		unq = true;
		for (i = 0; i < size && unq; ++i)
		{
			if (excluded[i] == res)
				unq = false;
		}
	}

	return res;
}