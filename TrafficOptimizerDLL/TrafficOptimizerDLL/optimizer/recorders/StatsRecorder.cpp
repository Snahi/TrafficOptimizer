#include "../../pch.h"
#include "StatsRecorder.h"
#include <unordered_map>



using namespace std;
using namespace tinyxml2;



// constructors ///////////////////////////////////////////////////////////////////////////////////



StatsRecorder::StatsRecorder(string filePath, Algorithm* pAlgorithm, ModelInterface* pModel) 
	: Recorder(filePath, pAlgorithm, pModel)
{
}



// methods ////////////////////////////////////////////////////////////////////////////////////////



StartRecordingResult StatsRecorder::startRecordingInner()
{
	// check if the algorithm is not null
	if (pAlgorithm == nullptr)
		return StartRecordingResult::NULL_ALGORITHM;

	return StartRecordingResult::SUCCESS;
}



RecordResult StatsRecorder::record()
{
	algorithmState.clear();
	pAlgorithm->getState(algorithmState);

	int problemSize = pAlgorithm->getIndividualSize();

	// get the best individual and it's fitness
	double* bestIndividual = new double[problemSize];
	double bestFitness = pAlgorithm->getBestSolution(bestIndividual);
	// convert the best individual to string
	string bestIndividualStr;
	for (int i = 0; i < problemSize; ++i)
		bestIndividualStr.append(to_string(bestIndividual[i]) + " ");

	// create a xml record
	XMLElement* pRecordXML = doc.NewElement(TAG_POPULATION);
	for (auto& pairState : algorithmState)
		pRecordXML->SetAttribute(pairState.first.c_str(), pairState.second);

	// add all attributes to the xml record
	pRecordXML->SetAttribute(ATTR_BEST_FITNESS, bestFitness);
	pRecordXML->SetAttribute(ATTR_BEST_INDIVIDUAL, bestIndividualStr.c_str());
	
	doc.InsertFirstChild(pRecordXML);

	delete[] bestIndividual;

	return RecordResult::SUCCESS;
}



StopRecordingResult StatsRecorder::stopRecordingInner()
{
	XMLError err = doc.SaveFile(filePath.c_str());

	if (err == XMLError::XML_SUCCESS)
		return StopRecordingResult::SUCCESS;
	else
		return StopRecordingResult::COULD_NOT_WRITE_TO_FILE;
}