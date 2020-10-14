#pragma once
#include "optimizer/ModelInterface.h"
#include "optimizer/evaluators/Evaluator.h"
#include "optimizer/stop_conditions/StopCondition.h"
#include "optimizer/recorders/Recorder.h"
#include "optimizer/Optimizer.h"
#include <thread>
#include <mutex>
#include <unordered_map>
#include <string>
#include <vector>


/*
	Class which represents single run of an optimization. Also, it serves as an interface and the
	running model.
*/
class Optimization : public ProgressListener, public StateListener
{
public:
	// const //////////////////////////////////////////////////////////////////////////////////////
	static const int ERR_CODE_NOT_SET = -1;
	static const int ERR_CODE_SUCCESS = 0;
	static const int ERR_CODE_SAVE_ERR = 1;
	static const int ERR_CODE_UNEXPECTED_ERR = 2;

	// constructors ///////////////////////////////////////////////////////////////////////////////
	Optimization(ModelInterface* pModel);

	~Optimization();
	// methods ////////////////////////////////////////////////////////////////////////////////////
	bool run();
	bool stop();
	bool saveCurrentResult(const char* path);
	bool isFinished();
	// overrides
	virtual void progressChanged(double progress) override;
	double getProgress(bool* pSuccess);
	virtual void stateChanged(std::unordered_map<std::string, double>& state,
		double* bestSolution) override;
	// getters && setters
	void setEvaluator(Evaluator* pEvaluator);
	Evaluator* getEvaluator();
	void setStopCondition(StopCondition* pStopCondition);
	void setRecorder(Recorder* pRecorder);
	bool setAlgorithm(Algorithm* pAlgorithm);
	Algorithm* getAlgorithm();
	ModelInterface* getModel();
	bool getVariableParamValues(std::string name, std::vector<double>& values);
	std::vector<std::string>& getVariableParamsNames();
	double getBestFitness(bool* pSuccess);
	int getErrCode();
	void getDurations(std::unordered_map<int, double>& durations);
	void getOffsetes(std::unordered_map<int, double>& offsets);

private:
	// fields /////////////////////////////////////////////////////////////////////////////////////
	ModelInterface* pModel;
	Evaluator* pEvaluator;
	StopCondition* pStopCondition;
	Recorder* pRecorder;
	Algorithm* pAlgorithm;
	std::thread* pRunThread;
	Optimizer* pOptimizer;
	/* mutProgress must be acquired before accessing it */
	double progress;
	std::mutex mutProgress;
	/* 
		 Parameters of the optimization (algorithm) which change. 
		 <param name, params since last get>.
		 MutVariableParams must be acquired before accessing it (excluding before runThread is 
		 started). The vector for which getVariableParamValues is called is cleared afterwards.
	*/
	std::unordered_map<std::string, std::vector<double>> variableParams;
	std::mutex mutVariableParams;
	std::vector<std::string> variableParamsNames;
	/* mutVariableParams must be acquired before accessing it. */
	double bestFitness;
	/* 
		flag indicating whether bestFitness has been obtained. It is used so that the user is not
		getting the confusing DBL_MAX if the initialization takes a lot of time. It's set to 
		true after the first call to this as a StateListener is performed.
	*/
	bool bestFitnessKnown;
	/* mutVariableParams must be acquired before accessing it. */
	double* bestSolution;
	int solutionSize;
	/* 
		code which is set after the optimization is finished. Before accessing it mutOptiFinshed
		must be acquired. 
	*/
	int errCode;
	/* true if optimization is finished. Before accessing it mutOptiFinished must be acquired. */
	bool finished;
	std::mutex mutOptiFinished;
	/* <lights group id, duration> */
	std::unordered_map<int, double> resultDurations;
	/* <lights system id, offset> */
	std::unordered_map<int, double> resultOffsets;
	// methods ////////////////////////////////////////////////////////////////////////////////////
	void runThreadFun();
};

