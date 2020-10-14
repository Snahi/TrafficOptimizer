#pragma once
#include "Recorder.h"
#include "../algorithms/Algorithm.h"
#include <string>
#include "../../xml/tinyxml2.h"
#include "../ModelInterface.h"
#include <unordered_map>



class StatsRecorder : public Recorder
{
public:
	// constructors ///////////////////////////////////////////////////////////////////////////////
	StatsRecorder(std::string filePath, Algorithm* pAlgorithm, ModelInterface* pModel);
	// methods ////////////////////////////////////////////////////////////////////////////////////
	virtual RecordResult record() override;

protected:
	virtual StartRecordingResult startRecordingInner() override;
	virtual StopRecordingResult stopRecordingInner() override;

private:
	// fields /////////////////////////////////////////////////////////////////////////////////////
	std::unordered_map<std::string, double> algorithmState;
	tinyxml2::XMLDocument doc;
};
