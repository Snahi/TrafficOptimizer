#include "../../pch.h"
#include "ResultOnlyRecorder.h"


using namespace std;



// constructors ///////////////////////////////////////////////////////////////////////////////////



ResultOnlyRecorder::ResultOnlyRecorder(string filePath, Algorithm* pAlgorithm, 
	ModelInterface* pModel) 
	: Recorder(filePath, pAlgorithm, pModel)
{
}



StartRecordingResult ResultOnlyRecorder::startRecordingInner()
{
	// check if the algorithm is not null
	if (pAlgorithm == nullptr)
		return StartRecordingResult::NULL_ALGORITHM;

	return StartRecordingResult::SUCCESS;
}



RecordResult ResultOnlyRecorder::record()
{
	return RecordResult::SUCCESS;
}



StopRecordingResult ResultOnlyRecorder::stopRecordingInner()
{
	return StopRecordingResult::SUCCESS;
}