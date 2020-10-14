#pragma once



namespace EvoRand {
	/*
		@individual - vector which will be filled with random numbers considering the 
					min/max constraints
		@mins - vector with the minimum values
		@maxes - vector with the maximum values
		@size - size of individual
	*/
	void randomIndividualConstrained(double* individual, double* mins, double* maxes, int size);
	void randomPopulationConstrained(double* population, double* mins, double* maxes, 
		int populationSize, int individualSize);
	int distinctRand(int* excluded, int size, int min, int max);
}
