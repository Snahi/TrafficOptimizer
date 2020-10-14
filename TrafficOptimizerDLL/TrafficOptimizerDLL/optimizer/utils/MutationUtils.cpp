#include "../../pch.h"
#include "MutationUtils.h"
#include <stdlib.h>     // srand, rand 
#include <time.h>       // time 
#include "ArrayUtils.h"
#include "RandomUtils.h"



using namespace std;



// helpers ////////////////////////////////////////////////////////////////////////////////////////



void JADECorrection(double* mutants, double* population, double* min, double* max, int size, 
	int individualSize)
{
	int realIdx = 0;
	double* pMutant;
	double* pIndividual;
	for (int indivIdx = 0; indivIdx < size; ++indivIdx)
	{
		pMutant = mutants + realIdx;
		pIndividual = population + realIdx;
		for (int geneIdx = 0; geneIdx < individualSize; ++geneIdx)
		{
			if (pMutant[geneIdx] < min[geneIdx])
				pMutant[geneIdx] = (min[geneIdx] + pIndividual[geneIdx]) / 2.0;
			else if (pMutant[geneIdx] > max[geneIdx])
				pMutant[geneIdx] = (max[geneIdx] + pIndividual[geneIdx]) / 2.0;
		}

		realIdx += individualSize;
	}
}



// exposed functions //////////////////////////////////////////////////////////////////////////////



void Mutation::DERand1(double* population, double* mutants, int populationSize, 
	int individualSize, double f, double* mins, double* maxes)
{
	/*
		ids of random individuals from which a mutant will be created. [0] draw 1, 
		[1] draw 2, [2] draw 3; draw 1, 2 and 3 must have be different.
	*/
	int ids[3];

	srand((unsigned int)time(NULL));
	double* pMutant; // idx of mutant in the mutants array
	for (int mutantIdx = 0; mutantIdx < populationSize; ++mutantIdx)
	{
		// choose participants
		ids[0] = rand() % populationSize;
		while (ids[0] == mutantIdx)
			ids[0] = rand() % populationSize;

		ids[1] = rand() % populationSize;
		while (ids[1] == mutantIdx || ids[1] == ids[0])
			ids[1] = rand() % populationSize;

		ids[2] = rand() % populationSize;
		while (ids[2] == mutantIdx || ids[2] == ids[1] || ids[2] == ids[0])
			ids[2] = rand() % populationSize;

		// create a mutant
		pMutant = mutants + mutantIdx * individualSize;

		// X - indivdual
		// X1 - X2
		EvoArray::subtract(population + ids[1] * individualSize, 
			population + ids[2] * individualSize, pMutant, individualSize);
		// f x (X1 - X2)
		EvoArray::multiply(pMutant, f, pMutant, individualSize);
		// X0 + f x (X1 - X2)
		EvoArray::add(population + ids[0] * individualSize, pMutant, pMutant, individualSize);

		// if some values are out of range move them to the limits
		EvoArray::squeeze(pMutant, mins, maxes, individualSize);
	}
}



void Mutation::DECurrentToPBestArch1(double* population, double* fitnessArr, double* mutants,
	double* fs, double p, int populationSize, int individualSize, double* archive,
	int archiveSize, double* mins, double* maxes)
{
	/*
		ids of individuals to use in mutation. 
		[0] -> current
		[1] -> pbest (chosen from topN best individuals)
		[2] -> random from population
		[3] -> random from population U archive (if the id is bigger than population size then it 
			is from the archive).
	*/
	int chosenIds[4]; 
	int realChosenIds[4];	// chosen ids multiplied by individual size
	int realMutantIdx = 0;	// real idx of the mutant (indivIdx * indivSize)
	double* pMutant;		// pointer to the current mutant
	double* diffToPBest = new double[individualSize]; // Fi x (Xpbest - Xi)
	int topN = (int)(p * populationSize); // number of the best individuals from which pbest will be chosen
	if (topN < 1)
		topN = 1;
	int* topIdxes = new int[topN]; // idxes of topN best individuals
	EvoArray::topNIdxes(fitnessArr, populationSize, topIdxes, topN);
	srand((unsigned int)time(NULL));
	for (int indivIdx = 0; indivIdx < populationSize; ++indivIdx)
	{
		chosenIds[0] = indivIdx; // Xi
		chosenIds[1] = topIdxes[rand() % topN]; // Xpbest
		chosenIds[2] = EvoRand::distinctRand(chosenIds, 2, 0, populationSize); // Xrp
		chosenIds[3] = EvoRand::distinctRand(chosenIds, 3, 0, populationSize + archiveSize); // Xrp+a

		// transform idxes to real idxes (multiply by individual size)
		EvoArray::multiply(chosenIds, individualSize, realChosenIds, 4);
		pMutant = mutants + realMutantIdx;

		// Xrp - Xrp+a -> store in pMutant
		if (chosenIds[3] < populationSize) // if Xrp+a is from population
		{
			EvoArray::subtract(population + realChosenIds[2], population + realChosenIds[3],
				pMutant, individualSize);
		}
		else // if Xrp+a is from archive
		{
			EvoArray::subtract(population + realChosenIds[2],
				archive + ((chosenIds[3] - populationSize) * individualSize), pMutant, 
				individualSize);
		}

		// Xpbest - Xi -> store in diffToPBest
		EvoArray::subtract(population + realChosenIds[1], population + realChosenIds[0], 
			diffToPBest, individualSize);

		// Xpbest - Xi + Xrp - Xrp+a -> store in pMutant
		EvoArray::add(diffToPBest, pMutant, pMutant, individualSize);
		// Fi x (Xpbest - Xi + Xrp - Xrp+a) -> store in pMutant
		EvoArray::multiply(pMutant, fs[indivIdx], pMutant, individualSize);
		// Xi + Fi * (Xpbest - Xi + Xrp - Xrp+a)
		EvoArray::add(population + realChosenIds[0], pMutant, pMutant, individualSize);

		realMutantIdx += individualSize;
	}

	// correct out-of-bounds values
	JADECorrection(mutants, population, mins, maxes, populationSize, individualSize);
	
	delete[] topIdxes;
	delete[] diffToPBest;
}



void Mutation::DECurrentToPArrayBestArch1(double* population, double* fitnessArr, double* mutants,
	double* fs, double* ps, int populationSize, int individualSize, double* archive,
	int archiveSize, double* mins, double* maxes)
{
	/*
		ids of individuals to use in mutation.
		[0] -> current
		[1] -> pbest (chosen from topN best individuals)
		[2] -> random from population
		[3] -> random from population U archive (if the id is bigger than population size then it
			is from the archive).
	*/
	int chosenIds[4];
	int realChosenIds[4];	// chosen ids multiplied by individual size
	int realMutantIdx = 0;	// real idx of the mutant (indivIdx * indivSize)
	double* pMutant;		// pointer to the current mutant
	double* diffToPBest = new double[individualSize]; // Fi x (Xpbest - Xi)
	int topN; // how many top individuals will be considered for pBest
	int* topIdxes = new int[populationSize]; // idxes of topN best individuals
	srand((unsigned int)time(NULL));
	for (int indivIdx = 0; indivIdx < populationSize; ++indivIdx)
	{
		topN = (int)(ps[indivIdx] * populationSize); // number of the best individuals from which pbest will be chosen
		if (topN < 1)
			topN = 1;
		EvoArray::topNIdxes(fitnessArr, populationSize, topIdxes, topN);

		chosenIds[0] = indivIdx; // Xi
		chosenIds[1] = topIdxes[rand() % topN]; // Xpbest
		chosenIds[2] = EvoRand::distinctRand(chosenIds, 2, 0, populationSize); // Xrp
		chosenIds[3] = EvoRand::distinctRand(chosenIds, 3, 0, populationSize + archiveSize); // Xrp+a

		// transform idxes to real idxes (multiply by individual size)
		EvoArray::multiply(chosenIds, individualSize, realChosenIds, 4);
		pMutant = mutants + realMutantIdx;

		// Xrp - Xrp+a -> store in pMutant
		if (chosenIds[3] < populationSize) // if Xrp+a is from population
		{
			EvoArray::subtract(population + realChosenIds[2], population + realChosenIds[3],
				pMutant, individualSize);
		}
		else // if Xrp+a is from archive
		{
			EvoArray::subtract(population + realChosenIds[2],
				archive + ((chosenIds[3] - populationSize) * individualSize), pMutant,
				individualSize);
		}

		// Xpbest - Xi -> store in diffToPBest
		EvoArray::subtract(population + realChosenIds[1], population + realChosenIds[0],
			diffToPBest, individualSize);

		// Xpbest - Xi + Xrp - Xrp+a -> store in pMutant
		EvoArray::add(diffToPBest, pMutant, pMutant, individualSize);
		// Fi x (Xpbest - Xi + Xrp - Xrp+a) -> store in pMutant
		EvoArray::multiply(pMutant, fs[indivIdx], pMutant, individualSize);
		// Xi + Fi * (Xpbest - Xi + Xrp - Xrp+a)
		EvoArray::add(population + realChosenIds[0], pMutant, pMutant, individualSize);

		realMutantIdx += individualSize;
	}

	// correct out-of-bounds values
	JADECorrection(mutants, population, mins, maxes, populationSize, individualSize);

	delete[] topIdxes;
	delete[] diffToPBest;
}