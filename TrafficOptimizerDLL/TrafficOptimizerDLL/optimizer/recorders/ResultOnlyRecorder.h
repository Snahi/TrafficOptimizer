#pragma once
#include "Recorder.h"
#include "../ModelInterface.h"
#include <string>



class ResultOnlyRecorder : public Recorder
{
public:
	// constructor
	ResultOnlyRecorder(std::string filePath, Algorithm* pAlgorithm, ModelInterface* pModel);

	// methods ////////////////////////////////////////////////////////////////////////////////////
	virtual RecordResult record() override;

protected:
	virtual StartRecordingResult startRecordingInner() override;
	virtual StopRecordingResult stopRecordingInner() override;
};
