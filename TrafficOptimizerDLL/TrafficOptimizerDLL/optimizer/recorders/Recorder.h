#pragma once
#include <string>
#include "../algorithms/Algorithm.h"
#include "../ModelInterface.h"
#include <mutex>
#include <unordered_map>



// constants //////////////////////////////////////////////////////////////////////////////////////
#define INDIVIDUAL_SEPARATOR "\n"
#define GENE_SEPARATOR " "
#define TAG_POPULATION "population"
#define TAG_STATISTICS "statistics"
#define TAG_RESULT "result"
#define TAG_GROUPS_SOULTIONS "groups"
#define TAG_SYSTEMS_SOLUTIONS "systems"
#define ATTR_BEST_FITNESS "best_fitness"
#define ATTR_GENERATIONS "num_of_generations"
#define ATTR_BEST_INDIVIDUAL "best_individual"
#define SEPARATOR_ID_SOLUTION " "
#define SEPARATOR_SOLUTIONS "\n"



enum class StartRecordingResult
{
	SUCCESS,
	NULL_ALGORITHM,
	CLEAR_FILE
};



enum class RecordResult
{
	SUCCESS
};



enum class StopRecordingResult
{
	SUCCESS,
	COULD_NOT_WRITE_TO_FILE,
	NULL_POPULATION,
	COULD_NOT_SAVE_RESULT
};



/*
	Usage:
		1. call startRecording(...)
		2. whenever a new population is created call record()
		3. If optimisation is finished call stopRecording()
*/
class Recorder
{
public:
	// constructors ///////////////////////////////////////////////////////////////////////////////
	Recorder(std::string& filePath, Algorithm* pAlgorithm, ModelInterface* pModel);

	// methods ////////////////////////////////////////////////////////////////////////////////////
	StartRecordingResult startRecording();
	virtual RecordResult record() = 0;
	StopRecordingResult stopRecording();
	void setAlgorithm(Algorithm* pAlgorithm);
	bool saveResult(std::string path, double bestFitness, double* bestSolution);
	static bool readResult(std::string path, std::unordered_map<int, double>& durations,
		std::unordered_map<int, double>& offsets, double* pBestFitness);

protected:
	// fields /////////////////////////////////////////////////////////////////////////////////////
	std::string filePath;
	Algorithm* pAlgorithm;
	ModelInterface* pModel;

	// methods ////////////////////////////////////////////////////////////////////////////////////
	virtual StartRecordingResult startRecordingInner() = 0;
	virtual StopRecordingResult stopRecordingInner() = 0;

private:
	// methods ////////////////////////////////////////////////////////////////////////////////////
	bool saveResult();
};
