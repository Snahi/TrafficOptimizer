#include "pch.h"
#include "TrafficOptimizer.h"
#include "Optimization.h"
#include "simplified_model/SimplifiedModelLoader.h"
#include "model/ModelLoader.h"
#include "optimizer/evaluators/Evaluator.h"
#include "optimizer/evaluators/AdditionalTimeEvaluator.h"
#include "optimizer/evaluators/AvgTimeEvaluator.h"
#include "optimizer/stop_conditions/StopCondition.h"
#include "optimizer/stop_conditions/EvaluationsOptSC.h"
#include "optimizer/stop_conditions/EvaluationsSC.h"
#include "optimizer/stop_conditions/TimeSC.h"
#include "optimizer/stop_conditions/TimeOptSC.h"
#include "optimizer/recorders/Recorder.h"
#include "optimizer/recorders/ResultOnlyRecorder.h"
#include "optimizer/recorders/StatsRecorder.h"
#include "optimizer/algorithms/Algorithm.h"
#include "optimizer/algorithms/LSHADE.h"
#include "optimizer/algorithms/LSHADEWGD.h"
#include <unordered_map>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <mutex>
#include <filesystem>
#include <vector>


using namespace std;



// constants //////////////////////////////////////////////////////////////////////////////////////
const char SIMPLIFIED_MODEL[] = "SimplifiedModel";
const char ADVANCED_MODEL[] = "AdvancedModel";
const int LOAD_MODEL_UNKNOWN_ERR = 17440;
const char SIMPLIFIED_MODEL_STRING[] = "SimplifiedModel";
const char ADVANCED_MODEL_STRING[] = "AdvancedModel";
const char ADDITIONAL_TIME_EVALUATOR_STR[] = "AdditionalTimeEvaluator";
const char AVG_TIME_EVALUATOR_STR[] = "AvgTimeEvaluator";
const char NUM_OF_EVALUATIONS_SC_STR[] = "NumOfEvaluations";
const char TIME_SC_STR[] = "Time";
const char RECORDER_RESULT_ONLY_STR[] = "RecResOnly";
const char RECORDER_RESULT_AND_STATS_STR[] = "RecResAndStats";



// global variables ///////////////////////////////////////////////////////////////////////////////
static Optimization* pOptimization = nullptr;



// function declarations //////////////////////////////////////////////////////////////////////////
bool loadSimplifiedModel(const char* path, ModelInterface** ppLoadedModel, int* pRes, 
	int* pErrLine);
bool loadAdvancedModel(const char* path, ModelInterface** ppLoadedModel, int* pRes, int* pErrLine);
Evaluator* createEvaluator(const char* evaluatorStr, ModelInterface* pModel, double timeoutSeconds);
StopCondition* createStopCondition(const char* stopCondStr, double endValue);
StopCondition* createStopCondition(const char* stopCondStr, double endValue, double optimum);
Recorder* createRecorder(const char* recorderStr, const char* savePath, ModelInterface* pModel);




// DLL functions implementations //////////////////////////////////////////////////////////////////

// loadModel //////////////////////////////////////////////////////////////////////////////////////
void loadModel(const char* path, const char* modelType, int* pResult, int* pErrLine)
{
	// load the model
	bool successfulLoad = false;
	ModelInterface* pModel = nullptr;
	if (strcmp(modelType, SIMPLIFIED_MODEL) == 0)
		successfulLoad = loadSimplifiedModel(path, &pModel, pResult, pErrLine);
	else if (strcmp(modelType, ADVANCED_MODEL) == 0)
		successfulLoad = loadAdvancedModel(path, &pModel, pResult, pErrLine);
	
	// create Optimization instance and add it to optimizations
	if (successfulLoad)
	{
		if (pOptimization != nullptr)
		{
			pOptimization->stop();
			delete pOptimization;
		}

		pOptimization = new Optimization(pModel);
	}
}



bool loadSimplifiedModel(const char* path, ModelInterface** ppLoadedModel, int* pRes, 
	int* pErrLine)
{
	SimplifiedModelLoader loader;
	*ppLoadedModel = loader.load(path, pRes, pErrLine);

	if (*pRes == LOAD_SUCCESS)
		return true;
	else
		return false;
}



bool loadAdvancedModel(const char* path, ModelInterface** ppLoadedModel, int* pRes, int* pErrLine)
{
	string pathStr = path;
	ModelLoader loader(pathStr);
	LoadResult loadRes = loader.load();

	*pRes = loadRes.getErrCode();
	if (loadRes.success())
	{
		*ppLoadedModel = loadRes.getModel();

		return true;
	}
	else
	{
		*pErrLine = loadRes.getErrLine();
		return false;
	}
}



// prepare optimization ///////////////////////////////////////////////////////////////////////////



bool prepareOptimization(const char* evaluatorStr, double timeoutSeconds, 
	const char* stopConditionStr, double endValue, 	bool withOptimum, double optimum, 
	const char* recorderStr, const char* savePath)
{
	if (pOptimization == nullptr)
		return false;

	ModelInterface* pModel = pOptimization->getModel();
	Evaluator* pEvaluator = createEvaluator(evaluatorStr, pModel, timeoutSeconds);
	StopCondition* pSC;
	
	if (withOptimum)
		pSC = createStopCondition(stopConditionStr, endValue, optimum);
	else
		pSC = createStopCondition(stopConditionStr, endValue);
	// transform path so that it fits C++ and platform, e.g. \\ -> /
	filesystem::path genericPath = savePath;
	genericPath.make_preferred();
	string properPath = genericPath.generic_u8string();
	Recorder* pRecorder = createRecorder(recorderStr, properPath.c_str(), pModel);

	if (pEvaluator == nullptr || pSC == nullptr || pRecorder == nullptr)
		return false;

	pOptimization->setEvaluator(pEvaluator);
	pOptimization->setStopCondition(pSC);
	pOptimization->setRecorder(pRecorder);

	return true;
}



Evaluator* createEvaluator(const char* evaluatorStr, ModelInterface* pModel, double timeoutSeconds)
{
	Evaluator* pEvaluator = nullptr;
	if (strcmp(evaluatorStr, ADDITIONAL_TIME_EVALUATOR_STR) == 0)
		pEvaluator = new AdditionalTimeEvaluator();
	else if (strcmp(evaluatorStr, AVG_TIME_EVALUATOR_STR) == 0)
		pEvaluator = new AvgTimeEvaluator();

	if (pEvaluator != nullptr)
		pEvaluator->init(pModel, timeoutSeconds);

	return pEvaluator;
}



StopCondition* createStopCondition(const char* stopCondStr, double endValue)
{
	if (strcmp(stopCondStr, NUM_OF_EVALUATIONS_SC_STR) == 0)
		return new EvaluationsSC((int) endValue, nullptr);
	else if (strcmp(stopCondStr, TIME_SC_STR) == 0)
		return new TimeSC(endValue);
	else
		return nullptr;
}



StopCondition* createStopCondition(const char* stopCondStr, double endValue, double optimum)
{
	if (strcmp(stopCondStr, NUM_OF_EVALUATIONS_SC_STR) == 0)
		return new EvaluationsOptSC((int)endValue, nullptr, optimum);
	else if (strcmp(stopCondStr, TIME_SC_STR) == 0)
		return new TimeOptSC(endValue, optimum);
	else
		return nullptr;
}



Recorder* createRecorder(const char* recorderStr, const char* savePath, ModelInterface* pModel)
{
	if (strcmp(recorderStr, RECORDER_RESULT_ONLY_STR) == 0)
		return new ResultOnlyRecorder(savePath, nullptr, pModel);
	else if (strcmp(recorderStr, RECORDER_RESULT_AND_STATS_STR) == 0)
		return new StatsRecorder(savePath, nullptr, pModel);
	else
		return nullptr;
}



bool prepareLSHADE(int maxNumOfEvaluations, double rInitPop, 
	double rInitArc, double p, int historySize)
{
	if (pOptimization == nullptr)
		return false;

	// assign default values to non-specified parameters (-1)
	maxNumOfEvaluations = maxNumOfEvaluations >= 0 ? maxNumOfEvaluations : 
		LSHADE::MAX_NUM_OF_EVALUATIONS_NOT_SET;
	rInitPop = rInitPop > 0 ? rInitPop : LSHADE::DEFAULT_R_N_INIT;
	rInitArc = rInitArc > 0 ? rInitArc : LSHADE::DEFAULT_R_ARC;
	p = p > 0 ? p : LSHADE::DEFAULT_P;
	historySize = historySize > 0 ? historySize : LSHADE::DEFAULT_H;

	Evaluator* pEvaluator = pOptimization->getEvaluator(); // algorithm needs evaluator which is already created
	Algorithm* pAlgorithm = new LSHADE(pEvaluator, maxNumOfEvaluations, rInitPop, 
		rInitArc, p, historySize);
	return pOptimization->setAlgorithm(pAlgorithm);
}



bool prepareLSHADEWGD(int maxNumOfEvaluations,
	double rInitPop, double rInitArc, double p, int historySize, int geneHistorySize,
	double gqImportance)
{
	if (pOptimization == nullptr)
		return false;

	// assign default values to non-specified parameters (-1)
	maxNumOfEvaluations = maxNumOfEvaluations >= 0 ? maxNumOfEvaluations :
		LSHADEWGD::MAX_NUM_OF_EVALUATIONS_NOT_SET;
	rInitPop = rInitPop > 0 ? rInitPop : LSHADEWGD::DEFAULT_R_N_INIT;
	rInitArc = rInitArc > 0 ? rInitArc : LSHADEWGD::DEFAULT_R_ARC;
	p = p > 0 ? p : LSHADEWGD::DEFAULT_P;
	historySize = historySize > 0 ? historySize : LSHADEWGD::DEFAULT_H;
	geneHistorySize = geneHistorySize > 0 ? geneHistorySize : LSHADEWGD::DEFAULT_GENE_HISTORY_SIZE;
	gqImportance = gqImportance > 0 && gqImportance < 1 ? gqImportance : 
		LSHADEWGD::DEFAULT_GQ_IMPORTANCE;

	Evaluator* pEvaluator = pOptimization->getEvaluator(); // algorithm needs evaluator which is already created
	Algorithm* pAlgorithm = new LSHADEWGD(pEvaluator, maxNumOfEvaluations, rInitPop,
		rInitArc, p, historySize, geneHistorySize, gqImportance);

	return pOptimization->setAlgorithm(pAlgorithm);
}



// run optimization ///////////////////////////////////////////////////////////////////////////////



bool runOptimization()
{
	if (pOptimization == nullptr)
		return false;

	return pOptimization->run();
}



double getProgress(bool* pSuccess)
{
	*pSuccess = true;

	if (pOptimization == nullptr)
	{
		*pSuccess = false;
		return 0.0;
	}

	double progress = pOptimization->getProgress(pSuccess);;

	return progress;
}



// get parameters names ///////////////////////////////////////////////////////////////////////////



bool getVariableParamsNames(SAFEARRAY** pResult)
{
	if (pOptimization == nullptr)
		return false;

	vector<string> paramNames = pOptimization->getVariableParamsNames();

	// convert param names to CComSafeArray and send
	try
	{
		CComSafeArray<BSTR> result(paramNames.size());
		int numOfParams = paramNames.size();
		for (int i = 0; i < numOfParams; ++i)
		{
			if (FAILED(result.SetAt(i, CComBSTR(paramNames[i].c_str()))))
				return false;
		}

		*pResult = result.Detach();
		return true;
	}
	catch (CAtlException)
	{
		return false;
	}
}



// get parameters /////////////////////////////////////////////////////////////////////////////////



bool getParamValues(const char* paramName, SAFEARRAY** pResult)
{
	if (pOptimization == nullptr)
		return false;

	vector<double> values;
	pOptimization->getVariableParamValues(paramName, values);

	// convert to CComSafeArray
	int numOfValues = values.size();
	try
	{
		CComSafeArray<DOUBLE> sa(numOfValues);
		for (int i = 0; i < numOfValues; i++)
		{
			sa[i] = values[i];
		}

		*pResult = sa.Detach();
	}
	catch (CAtlException)
	{
		return false;
	}
	
	return true;
}



// get best fitness ///////////////////////////////////////////////////////////////////////////////



double getBestFitness(bool* pSuccess)
{
	if (pOptimization == nullptr)
	{
		*pSuccess = false;
		return DBL_MAX;
	}

	double bestFitness = pOptimization->getBestFitness(pSuccess);

	return bestFitness;
}



// is finished ////////////////////////////////////////////////////////////////////////////////////



bool isFinished(bool* pSuccess)
{
	if (pOptimization == nullptr)
	{
		*pSuccess = false;
		return false;
	}

	*pSuccess = true;
	return pOptimization->isFinished();
}



// run optimization error code ////////////////////////////////////////////////////////////////////



int getErrCode(bool* pSuccess)
{
	if (pOptimization == nullptr)
	{
		*pSuccess = false;
		return false;
	}

	*pSuccess = true;
	return pOptimization->getErrCode();
}



// delete optimization ////////////////////////////////////////////////////////////////////////////



bool deleteOptimization()
{
	if (pOptimization == nullptr)
		return false;

	delete pOptimization;
	pOptimization = nullptr;

	return true;
}



// result /////////////////////////////////////////////////////////////////////////////////////////



bool fillIntSafeArray(CComSafeArray<INT>& arr, vector<int> source)
{
	int numOfValues = source.size();
	try
	{
		for (int i = 0; i < numOfValues; i++)
		{
			arr[i] = source[i];
		}
	}
	catch (CAtlException)
	{
		return false;
	}

	return true;
}



bool fillDoubleSafeArray(CComSafeArray<DOUBLE>& arr, vector<double> source)
{
	int numOfValues = source.size();
	try
	{
		for (int i = 0; i < numOfValues; i++)
		{
			arr[i] = source[i];
		}
	}
	catch (CAtlException)
	{
		return false;
	}

	return true;
}



bool getResult(SAFEARRAY** pGroupsIds,
	SAFEARRAY** pDurations, SAFEARRAY** pSystemsIds, SAFEARRAY** pOffsets)
{
	if (pOptimization == nullptr)
		return false;

	// obtain result
	unordered_map<int, double> durationsMap;
	unordered_map<int, double> offsetsMap;
	pOptimization->getDurations(durationsMap);
	pOptimization->getOffsetes(offsetsMap);

	// convert to CComSafeArray
	vector<int> groupsIdsSource;								// ids of lightsGroups in result
	vector<double> durationsSource;								// durations of lights groups in result
	for (auto& pairDuration : durationsMap)
	{
		groupsIdsSource.push_back(pairDuration.first);
		durationsSource.push_back(pairDuration.second);
	}
	vector<int> systemsIdsSource;								// ids of lightsSystems in result
	vector<double> offsetsSource;								// offests of lights systems in result
	for (auto& pairOffset : offsetsMap)
	{
		systemsIdsSource.push_back(pairOffset.first);
		offsetsSource.push_back(pairOffset.second);
	}
	CComSafeArray<INT> groupsIds(groupsIdsSource.size());
	CComSafeArray<DOUBLE> durations(durationsSource.size());
	CComSafeArray<INT> systemsIds(systemsIdsSource.size());
	CComSafeArray<DOUBLE> offsets(offsetsSource.size());
	if (!fillIntSafeArray(groupsIds, groupsIdsSource))
		return false;
	if (!fillDoubleSafeArray(durations, durationsSource))
		return false;
	if (!fillIntSafeArray(systemsIds, systemsIdsSource))
		return false;
	if (!fillDoubleSafeArray(offsets, offsetsSource))
		return false;

	*pGroupsIds = groupsIds.Detach();
	*pDurations = durations.Detach();
	*pSystemsIds = systemsIds.Detach();
	*pOffsets = offsets.Detach();

	return true;
}



bool sendResult(unordered_map<int, double>& durationsMap, unordered_map<int, double>& offsetsMap,
	SAFEARRAY** pGroupsIds, SAFEARRAY** pDurations,
	SAFEARRAY** pSystemsIds, SAFEARRAY** pOffsets)
{
	// convert to CComSafeArray
	vector<int> groupsIdsSource;								// ids of lightsGroups in result
	vector<double> durationsSource;								// durations of lights groups in result
	for (auto& pairDuration : durationsMap)
	{
		groupsIdsSource.push_back(pairDuration.first);
		durationsSource.push_back(pairDuration.second);
	}
	vector<int> systemsIdsSource;								// ids of lightsSystems in result
	vector<double> offsetsSource;								// offests of lights systems in result
	for (auto& pairOffset : offsetsMap)
	{
		systemsIdsSource.push_back(pairOffset.first);
		offsetsSource.push_back(pairOffset.second);
	}
	CComSafeArray<INT> groupsIds(groupsIdsSource.size());
	CComSafeArray<DOUBLE> durations(durationsSource.size());
	CComSafeArray<INT> systemsIds(systemsIdsSource.size());
	CComSafeArray<DOUBLE> offsets(offsetsSource.size());
	if (!fillIntSafeArray(groupsIds, groupsIdsSource))
		return false;
	if (!fillDoubleSafeArray(durations, durationsSource))
		return false;
	if (!fillIntSafeArray(systemsIds, systemsIdsSource))
		return false;
	if (!fillDoubleSafeArray(offsets, offsetsSource))
		return false;

	*pGroupsIds = groupsIds.Detach();
	*pDurations = durations.Detach();
	*pSystemsIds = systemsIds.Detach();
	*pOffsets = offsets.Detach();

	return true;
}



bool readResult(const char* path, SAFEARRAY** pGroupsIds, SAFEARRAY** pDurations, 
	SAFEARRAY** pSystemsIds, SAFEARRAY** pOffsets, double* pBestFitness)
{
	// obtain result
	filesystem::path pathGen = path;
	pathGen.make_preferred();
	string pathFinal = pathGen.generic_u8string();
	unordered_map<int, double> durationsMap;
	unordered_map<int, double> offsetsMap;

	if (Recorder::readResult(pathFinal, durationsMap, offsetsMap, pBestFitness))
		return sendResult(durationsMap, offsetsMap, pGroupsIds, pDurations, pSystemsIds, pOffsets);
	else
		return false;
}



// stop ///////////////////////////////////////////////////////////////////////////////////////////



bool stop()
{
	if (pOptimization == nullptr)
		return true;

	return pOptimization->stop();
}



// save ///////////////////////////////////////////////////////////////////////////////////////////



bool saveCurrResult(const char* path)
{
	if (pOptimization == nullptr)
		return false;

	return pOptimization->saveCurrentResult(path);
}