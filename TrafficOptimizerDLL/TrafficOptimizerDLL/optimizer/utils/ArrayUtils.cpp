#include "../../pch.h"
#include "ArrayUtils.h"
#include <stdlib.h>     // srand, rand 
#include <time.h>       // time 



using namespace std;



void EvoArray::add(double* arr1, double* arr2, double* sum, int size)
{
	for (int i = 0; i < size; ++i)
	{
		sum[i] = arr1[i] + arr2[i];
	}
}



void EvoArray::subtract(double* arr1, double* arr2, double* difference, int size)
{
	for (int i = 0; i < size; ++i)
	{
		difference[i] = arr1[i] - arr2[i];
	}
}



void EvoArray::multiply(double* arr, double factor, double* product, int size)
{
	for (int i = 0; i < size; ++i)
	{
		product[i] = factor * arr[i];
	}
}


void EvoArray::multiply(int* arr, int multiplier, int* product, int size)
{
	for (int i = 0; i < size; ++i)
	{
		product[i] = arr[i] * multiplier;
	}
}



void EvoArray::hadamardProduct(double* arr1, double* arr2, double* res, int size)
{
	for (int i = 0; i < size; ++i)
	{
		res[i] = arr1[i] * arr2[i];
	}
}



double EvoArray::sum(double* arr, int size)
{
	double res = 0.0;
	for (int i = 0; i < size; ++i)
	{
		res += arr[i];
	}

	return res;
}



int EvoArray::sum(int* arr, int size)
{
	int res = 0;
	for (int i = 0; i < size; ++i)
	{
		res += arr[i];
	}

	return res;
}



void EvoArray::copy(double* source, double* destination, int size)
{
	for (int i = 0; i < size; ++i)
	{
		destination[i] = source[i];
	}
}



void EvoArray::squeeze(double* arr, double* min, double* max, int size)
{
	for (int i = 0; i < size; ++i)
	{
		if (arr[i] < min[i])
			arr[i] = min[i];
		else if (arr[i] > max[i])
			arr[i] = max[i];
	}
}



int EvoArray::maxValueIdx(double* arr, int size)
{
	if (size <= 0)
		return -1;

	double best = arr[0];
	int bestIdx = 0;
	for (int i = 1; i < size; ++i)
	{
		if (arr[i] > best)
		{
			best = arr[i];
			bestIdx = i;
		}
	}

	return bestIdx;
}



double EvoArray::minValue(double* arr, int size, int* pIdx)
{
	if (size <= 0)
	{
		return 0.0;
		*pIdx = -1;
	}

	double min = arr[0];
	int minIdx = 0;
	for (int i = 1; i < size; ++i)
	{
		if (arr[i] < min)
		{
			min = arr[i];
			minIdx = i;
		}
	}

	if (pIdx != nullptr)
		*pIdx = minIdx;

	return min;
}



double EvoArray::maxValue(double* arr, int size, int* pIdx)
{
	if (size <= 0)
	{
		return 0.0;
		*pIdx = -1;
	}

	double max = arr[0];
	int maxIdx = 0;
	for (int i = 1; i < size; ++i)
	{
		if (arr[i] > max)
		{
			max = arr[i];
			maxIdx = i;
		}
	}

	if (pIdx != nullptr)
		*pIdx = maxIdx;

	return max;
}



void EvoArray::topNIdxes(double* arr, int size, int* topIdxes, int n)
{
	double* topValues = new double[n];
	double worstInTop;
	int idxOfWorstInTop;

	// populate top
	for (int i = 0; i < n; ++i)
	{
		topIdxes[i] = i;
		topValues[i] = arr[i];
	}

	worstInTop = minValue(topValues, n, &idxOfWorstInTop);

	// find the top n individuals
	for (int i = n; i < size; ++i)
	{
		if (arr[i] > worstInTop)
		{
			topIdxes[idxOfWorstInTop] = i;
			topValues[idxOfWorstInTop] = arr[i];
			worstInTop = minValue(topValues, n, &idxOfWorstInTop);
		}
	}

	delete[] topValues;
}



void EvoArray::worstNIdxes(double* arr, int size, int* worstIdxes, int n)
{
	double* worstValues = new double[n];
	double bestInWorst;
	int idxOfBestInWorst;

	// populate top
	for (int i = 0; i < n; ++i)
	{
		worstIdxes[i] = i;
		worstValues[i] = arr[i];
	}

	bestInWorst = maxValue(worstValues, n, &idxOfBestInWorst);

	// find the top n individuals
	for (int i = n; i < size; ++i)
	{
		if (arr[i] < bestInWorst)
		{
			worstIdxes[idxOfBestInWorst] = i;
			worstValues[idxOfBestInWorst] = arr[i];
			bestInWorst = maxValue(worstValues, n, &idxOfBestInWorst);
		}
	}

	delete[] worstValues;
}



void EvoArray::eraseRows(double* arr, int rows, int cols, int* idxes, int idxesSize)
{
	bubbleSort(idxes, idxesSize);
	int insIdx = 0;
	int takeIdx = 0;
	int idxToErase = idxes[0] * cols;
	int idxOfIdxToErase = 0;
	int arrSize = rows * cols;
	while (takeIdx < arrSize)
	{
		if (takeIdx == idxToErase)
		{
			takeIdx += cols;
			++idxOfIdxToErase;
			if (idxOfIdxToErase < idxesSize)
				idxToErase = idxes[idxOfIdxToErase] * cols;
		}
		else
		{
			if (insIdx != takeIdx)
			{
				for (int col = 0; col < cols; ++col)
					arr[insIdx + col] = arr[takeIdx + col];
			}

			insIdx += cols;
			takeIdx += cols;
		}
	}
}



void swap(int* xp, int* yp)
{
	int temp = *xp;
	*xp = *yp;
	*yp = temp;
}



void EvoArray::bubbleSort(int* arr, int n)
{
	int i, j;
	for (i = 0; i < n - 1; ++i)
	{
		// Last i elements are already in place  
		for (j = 0; j < n - i - 1; ++j)
		{
			if (arr[j] > arr[j + 1])
				swap(&arr[j], &arr[j + 1]);
		}
	}
}



int EvoArray::idxOfBest(double* arr, int size, double* pValue)
{
	if (size == 0)
		return -1;
	if (size == 1)
		return 0;

	double bestValue = arr[0];
	int bestIdx = 0;
	for (int i = 1; i < size; ++i)
	{
		if (arr[i] > bestValue)
		{
			bestValue = arr[i];
			bestIdx = i;
		}
	}

	if (pValue != nullptr)
		*pValue = bestValue;

	return bestIdx;
}