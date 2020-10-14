#pragma once



namespace EvoMath
{
	double weightedLehmerMean(double* weights, double* values, int size);
	double mean(double* values, int size);
	double weightedMean(double* weights, double* values, int size);
}
