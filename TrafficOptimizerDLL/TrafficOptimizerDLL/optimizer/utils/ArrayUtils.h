#pragma once



namespace EvoArray
{
	void add(double* arr1, double* arr2, double* sum, int size);
	void subtract(double* arr1, double* arr2, double* difference, int size);
	void multiply(double* arr, double factor, double* product, int size);
	void multiply(int* arr, int multiplier, int* product, int size);
	void hadamardProduct(double* arr1, double* arr2, double* res, int size);
	double sum(double* arr, int size);
	int sum(int* arr, int size);
	void copy(double* source, double* destination, int size);
	void squeeze(double* arr, double* min, double* max, int size);
	int maxValueIdx(double* arr, int size);
	double minValue(double* arr, int size, int* pIdx);
	double maxValue(double* arr, int size, int* pIdx);
	void topNIdxes(double* arr, int size, int* topIdxes, int n);
	void worstNIdxes(double* arr, int size, int* worstIdxes, int n);
	void eraseRows(double* arr, int rows, int cols, int* idxes, int idxesSize);
	void bubbleSort(int* arr, int n);
	int idxOfBest(double* arr, int size, double* pValue = nullptr);
}
