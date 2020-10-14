#include "../../pch.h"
#include "MathUtils.h"
#include "ArrayUtils.h"



double EvoMath::weightedLehmerMean(double* weights, double* values, int size)
{
	double* wV = new double[size];
	double* wV2 = new double[size];
	EvoArray::hadamardProduct(weights, values, wV, size);
	EvoArray::hadamardProduct(wV, values, wV2, size);
	double numerator = EvoArray::sum(wV2, size);
	double denominator = EvoArray::sum(wV, size);

	delete[] wV;
	delete[] wV2;

	return numerator / denominator;
}



double EvoMath::mean(double* values, int size)
{
	double total = 0.0;
	for (int i = 0; i < size; ++i)
		total += values[i];

	return total / size;
}



double EvoMath::weightedMean(double* weights, double* values, int size)
{
	double total = 0.0;
	for (int i = 0; i < size; ++i)
		total += weights[i] * values[i];

	double totalWeights = EvoArray::sum(weights, size);
	return total / totalWeights;
}