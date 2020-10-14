#pragma once



namespace Mutation
{
	/*
		Differential evolution / random perturbed vector / 1 difference vector
		mutant = inidividual_rand1 + f x (individual_rand2 - individual_rand3)

		@population - array with individuals [populationSize x individualSize]
		@mutants - array to which individuals will be put
		@populationSize - number of individuals
		@individualSize - number of genes
		@f - scaling factor by which the difference vector will be multiplied
	*/
	void DERand1(double* population, double* mutants, int populationSize, int individualSize, 
		double f, double* mins, double* maxes);
	void DECurrentToPBestArch1(double* population, double* fitnessArr, double* mutants, 
		double* fs, double p, int populationSize, int individualSize, double* archive, 
		int archiveSize, double* mins, double* maxes);
	void DECurrentToPArrayBestArch1(double* population, double* fitnessArr, double* mutants,
		double* fs, double* ps, int populationSize, int individualSize, double* archive,
		int archiveSize, double* mins, double* maxes);
}
