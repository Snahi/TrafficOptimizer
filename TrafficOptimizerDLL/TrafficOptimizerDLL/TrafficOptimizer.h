#pragma once

#ifdef TRAFFIC_OPTIMIZER_EXPORTS
#define TRAFFIC_OPTIMIZER_API __declspec(dllexport)
#else
#define TRAFFIC_OPTIMIZER_API __declspec(dllimport)
#endif

#include <atlsafe.h>



extern "C" TRAFFIC_OPTIMIZER_API void loadModel(const char* path, const char* modelType, 
	int* pResult, int* pErrLine);

extern "C" TRAFFIC_OPTIMIZER_API bool prepareOptimization(const char* evaluatorStr, 
	double timeoutSeconds, const char* stopConditionStr, double endValue, bool withOptimum, 
	double optimum, const char* recorderStr, const char* savePath);

extern "C" TRAFFIC_OPTIMIZER_API bool prepareLSHADE(int maxNumOfEvaluations, 
	double rInitPop, double rInitArc, double p, int historySize);

extern "C" TRAFFIC_OPTIMIZER_API bool prepareLSHADEWGD(int maxNumOfEvaluations,
	double rInitPop, double rInitArc, double p, int historySize, int geneHistorySize, 
	double gqImportance);

extern "C" TRAFFIC_OPTIMIZER_API bool runOptimization();

extern "C" TRAFFIC_OPTIMIZER_API double getProgress(bool* pSuccess);

extern "C" TRAFFIC_OPTIMIZER_API bool getVariableParamsNames(SAFEARRAY** pResult);

extern "C" TRAFFIC_OPTIMIZER_API bool getParamValues(const char* paramName,
	SAFEARRAY * *pResult);

extern "C" TRAFFIC_OPTIMIZER_API double getBestFitness(bool* pSuccess);

extern "C" TRAFFIC_OPTIMIZER_API bool isFinished(bool* pSuccess);

extern "C" TRAFFIC_OPTIMIZER_API int getErrCode(bool* pSuccess);

extern "C" TRAFFIC_OPTIMIZER_API bool deleteOptimization();

extern "C" TRAFFIC_OPTIMIZER_API bool getResult(SAFEARRAY * *pGroupsIds,
	SAFEARRAY** pDurations, SAFEARRAY** pSystemsIds, SAFEARRAY** pOffsets);

extern "C" TRAFFIC_OPTIMIZER_API bool readResult(const char* path, SAFEARRAY * *pGroupsIds,
	SAFEARRAY * *pDurations, SAFEARRAY * *pSystemsIds, SAFEARRAY * *pOffsets, 
	double* pBestFitness);

extern "C" TRAFFIC_OPTIMIZER_API bool stop();

extern "C" TRAFFIC_OPTIMIZER_API bool saveCurrResult(const char* path);
