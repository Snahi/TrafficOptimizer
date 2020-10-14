#include "../../pch.h"
#include "Recorder.h"
#include "../../xml/tinyxml2.h"
#include <unordered_map>
#include <fstream>


using namespace tinyxml2;
using namespace std;



// constructors ///////////////////////////////////////////////////////////////////////////////////



Recorder::Recorder(string& filePath, Algorithm* pAlgorithm, ModelInterface* pModel)
	: filePath{ filePath }, pAlgorithm{ pAlgorithm }, pModel{ pModel } 
{
}



// methods ////////////////////////////////////////////////////////////////////////////////////////



StartRecordingResult Recorder::startRecording()
{
	try
	{
		ofstream ofs;
		ofs.open(filePath, std::ofstream::out | std::ofstream::trunc);
		ofs.close();
	}
	catch (exception)
	{
		return StartRecordingResult::CLEAR_FILE;
	}

	return startRecordingInner();
}



void getSolutionsXML(XMLElement* pResultXML, unordered_map<int,double>& solutions, 
	const char* tag)
{
	XMLElement* pSolutions = pResultXML->InsertNewChildElement(tag);
	string solStr;
	for (auto& pairSol : solutions)
	{
		solStr.append(to_string(pairSol.first));
		solStr.append(SEPARATOR_ID_SOLUTION);
		solStr.append(to_string(pairSol.second));
		solStr.append(SEPARATOR_SOLUTIONS);
	}

	// remove the last separator
	for (int i = 0; i < (int)strlen(SEPARATOR_SOLUTIONS); ++i)
		solStr.pop_back();

	pSolutions->SetText(solStr.c_str());
}



bool Recorder::saveResult()
{
	double* bestSolution = new double[pAlgorithm->getIndividualSize()];
	double bestFitness = pAlgorithm->getBestSolution(bestSolution);
	bool res = saveResult(filePath, bestFitness, bestSolution);
	delete[] bestSolution;

	return res;
}



bool Recorder::saveResult(string path, double bestFitness, double* bestSolution)
{
	XMLDocument doc;
	XMLError loadErr = doc.LoadFile(filePath.c_str());

	if (loadErr == XML_SUCCESS || loadErr == XML_ERROR_EMPTY_DOCUMENT ||
		loadErr == XML_ERROR_FILE_NOT_FOUND)
	{
		XMLElement* pResultXML = doc.NewElement(TAG_RESULT);
		// connect ids of elements with values
		unordered_map<int, double> groups; // lights group id, duration
		unordered_map<int, double> systems; // lights system id, offset
		pModel->translate(bestSolution, groups, systems);
		// add to XML
		pResultXML->SetAttribute(ATTR_BEST_FITNESS, bestFitness);
		getSolutionsXML(pResultXML, groups, TAG_GROUPS_SOULTIONS);
		getSolutionsXML(pResultXML, systems, TAG_SYSTEMS_SOLUTIONS);
		doc.InsertFirstChild(pResultXML);
		if (doc.SaveFile(path.c_str()) == XML_SUCCESS)
			return true;
		else
			return false;
	}
	else
		return false;
}



void stringSplit(string text, vector<string>& res, string& delimiter)
{
	size_t pos = 0;
	string token;
	while ((pos = text.find(delimiter)) != string::npos) {
		token = text.substr(0, pos);
		res.push_back(token);
		text.erase(0, pos + delimiter.length());
	}

	res.push_back(text);
}



bool stringToMap(string& text, unordered_map<int, double>& map, string idValSeparator, 
	string pairSeparator)
{
	vector<string> pairs;
	stringSplit(text, pairs, pairSeparator);
	vector<string> idVal;
	string idStr;
	string valueStr;
	int id;
	double value;
	for (string& pair : pairs)
	{
		stringSplit(pair, idVal, idValSeparator);
		if (idVal.size() == 2)
		{
			idStr = idVal[0];
			valueStr = idVal[1];
			try
			{
				id = stoi(idStr);
				value = stod(valueStr);
			}
			catch (exception)
			{
				return false;
			}

			map[id] = value;
		}
		else
			return false;
		
		idVal.clear();
	}

	return true;
}



bool Recorder::readResult(string path, unordered_map<int, double>& durations,
	unordered_map<int, double>& offsets, double* pBestFitness)
{
	XMLDocument doc;
	XMLError loadRes = doc.LoadFile(path.c_str());

	if (loadRes == XML_SUCCESS)
	{
		XMLElement* pResultXML = doc.FirstChildElement(TAG_RESULT);
		if (pResultXML != nullptr)
		{
			XMLElement* pDurationsXML = pResultXML->FirstChildElement(TAG_GROUPS_SOULTIONS);
			if (pDurationsXML != nullptr)
			{
				string durationsText = pDurationsXML->GetText();
				if (stringToMap(durationsText, durations, SEPARATOR_ID_SOLUTION, 
					SEPARATOR_SOLUTIONS))
				{
					XMLElement* pOffsetsXML = pResultXML->FirstChildElement(TAG_SYSTEMS_SOLUTIONS);
					if (pOffsetsXML != nullptr)
					{
						string offsetsText = pOffsetsXML->GetText();
						if (stringToMap(offsetsText, offsets, SEPARATOR_ID_SOLUTION, 
							SEPARATOR_SOLUTIONS))
						{
							XMLError fitErr = pResultXML->QueryDoubleAttribute(ATTR_BEST_FITNESS, 
								pBestFitness);
							if (fitErr == XML_SUCCESS)
								return true;
							else
								return false;
						}
						else
							return false;
					}
					else
						return false;
				}
				else
					return false;
			}
			else
				return false;
		}
		else
			return false;
		
	}
	else
		return false;
}



StopRecordingResult Recorder::stopRecording()
{
	StopRecordingResult res = stopRecordingInner();
	if (res == StopRecordingResult::SUCCESS)
	{
		if (!saveResult())
		{
			return StopRecordingResult::COULD_NOT_SAVE_RESULT;
		}
	}

	return res;
}



// getters && setters /////////////////////////////////////////////////////////////////////////////



void Recorder::setAlgorithm(Algorithm* pAlgorithm)
{
	this->pAlgorithm = pAlgorithm;
}