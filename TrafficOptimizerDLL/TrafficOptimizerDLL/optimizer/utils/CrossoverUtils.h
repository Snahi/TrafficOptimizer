#pragma once



namespace Crossover
{
	void DEBinomial(double* population, double* mutants, double* offspring, int populationSize, 
		int individualSize, double crossoverRate);
	void DEBinomial(double* population, double* mutants, double* offspring, int populationSize,
		int individualSize, double* crossoverRate);
	void DEBinomialWGD(double* population, double* mutants, double* offspring, 
		int populationSize,	int individualSize, double* crossoverRate, int* wgh, 
		int geneHistorySize, double gqImportance, bool* modifiedGenes);
}
