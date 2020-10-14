#include "../pch.h"
#include "ModelLoader.h"
#include <algorithm>

using namespace std;
using namespace tinyxml2;



// constructors ///////////////////////////////////////////////////////////////////////////////////



ModelLoader::ModelLoader(string& path)
{
	this->pModel = nullptr;
	this->path = path;
}



// destructor /////////////////////////////////////////////////////////////////////////////////////



ModelLoader::~ModelLoader()
{
	for (auto pairCar : cars)
		delete pairCar.second;
}



// methods ////////////////////////////////////////////////////////////////////////////////////////



LoadResult ModelLoader::load()
{
	LoadResult result;

	this->pModel = new Model();

	XMLDocument modelDoc;
	if (modelDoc.LoadFile(path.c_str()) == XML_SUCCESS)
	{
		if (loadJunctions(modelDoc, result) &&
			loadGenerators(modelDoc, result) &&
			loadRoads(modelDoc, result) &&
			loadConflictGatesWithRoads(result) &&
			loadRoutes(modelDoc, result) &&
			loadCars(modelDoc, result) &&
			setTemplateCars(result) &&
			addConflictGatesToCompoundGates(result))
		{
			vector<ModelWarning> warnings = pModel->findWarnings();
			result.setWarnings(warnings);
			result.setModel(pModel);
		}
	}
	else
	{
		processLoadErrorToString(modelDoc, result);
		result.setErrMsg("Error in file " + path + "\n" + result.getErrMsg());
	}

	// clean up if failed
	if (result.fail())
	{
		if (pModel != nullptr)
		{
			delete pModel;
			pModel = nullptr;
		}
	}

	return result;
}



// junctions //////////////////////////////////////////////////////////////////////////////////////



bool ModelLoader::loadJunctions(tinyxml2::XMLDocument& doc, LoadResult& res)
{
	bool ret = true;
	XMLNode* junctionsXML = doc.FirstChildElement(TAG_JUNCTIONS); // <junctions>...</junctions>
	if (junctionsXML != nullptr)
	{
		XMLElement* pJunctionXML = junctionsXML->FirstChildElement(); // <junction> or <roundabout>
		const char* junctionTagName = nullptr;
		while (pJunctionXML != nullptr && ret == true)
		{
			junctionTagName = pJunctionXML->Value();

			if (strcmp(junctionTagName, TAG_JUNCTION) == 0) // <junction>
				ret = loadClassicalJunction(pJunctionXML, res);
			else if (strcmp(junctionTagName, TAG_ROUNDABOUT) == 0) // <roundabout>
				ret = loadRoundabout(pJunctionXML, res);
			else
			{
				ret = false;
				res.setErrCode(LOAD_ERR_UNKNOWN_JUNCTION_TYPE);
				res.setErrLine(pJunctionXML->GetLineNum());
				res.setErrMsg("Unknown junction tag <" + string(junctionTagName) + "> at line " 
					+ to_string(res.getErrLine()));
			}
				
			pJunctionXML = pJunctionXML->NextSiblingElement();
		}
	}
	else // no junctions tag
	{
		res.setErrCode(LOAD_ERR_NO_JUNCTIONS);
		res.setErrMsg("There isn't a junction tag in the model file");
		ret = false;
	}

	return ret;
}



bool ModelLoader::loadJunction(int id, XMLElement* pJunctionXML, LoadResult& res)
{
	bool ret = true;

	// lights systems, lights and lights groups
	XMLElement* pLightsSystemsXML = pJunctionXML->FirstChildElement(TAG_LIGHTS_SYSTEMS);
	if (pLightsSystemsXML != nullptr)
		ret = loadLightsSystems(pLightsSystemsXML, res, id);

	if (ret)
	{
		// junction inputs
		XMLElement* pInputsXML = pJunctionXML->FirstChildElement(TAG_JUNCTION_INPUTS);
		if (pInputsXML != nullptr)
		{
			if (!loadJunctionInputs(pInputsXML, res, id))
				ret = false;
		}
		else // no <inputs> section
		{
			ret = false;
			res.setErrCode(LOAD_ERR_NO_JUNCTION_INPUTS_SECTION);
			res.setErrLine(pJunctionXML->GetLineNum());
			res.setErrMsg("The tag <" + string(TAG_JUNCTION_INPUTS) +
				"> was not specified in the junction at line " + to_string(res.getErrLine()));
		}
	}

	return ret;
}



bool ModelLoader::loadClassicalJunction(tinyxml2::XMLElement* pJunctionXML, LoadResult& res)
{
	bool ret = true;

	// obtain id
	int id;
	XMLError attrErr = pJunctionXML->QueryIntAttribute(ATTR_JUNCTION_ID, &id);
	processXMLError(attrErr, pJunctionXML->GetLineNum(), XMLDocument::ErrorIDToName(attrErr), res);

	// go deeper
	if (res.success())
	{
		if (!isIdValid(id))
		{
			res.setErrCode(LOAD_ERR_INVALID_ID);
			res.setErrLine(pJunctionXML->GetLineNum());
			res.setErrMsg("Junction at line " + to_string(res.getErrLine()) + " has invalid id");
			ret = false;
		}
		else
		{
			int createJunctionRes = pModel->createJunction(id);
			if (createJunctionRes == CREATE_JUNCTION_SUCCESS)
			{
				ret = loadJunction(id, pJunctionXML, res);
			}
			else if (createJunctionRes == CREATE_JUNCTION_ALREADY_EXISTS)
			{
				ret = false;
				res.setErrCode(LOAD_ERR_JUNCTION_ALREADY_EXISTS);
				res.setErrLine(pJunctionXML->GetLineNum());
				res.setErrMsg("Junction defined at line " + to_string(res.getErrLine()) +
					" has an id which is already used by another junction");
			}
			else
			{
				ret = false;
				setUnknownCodeError(pJunctionXML->GetLineNum(), res);
			}
		}
	}
	else
		ret = false;

	return ret;
}



bool ModelLoader::loadRoundabout(tinyxml2::XMLElement* pRoundaboutXML, LoadResult& res)
{
	bool ret = true;

	int id;
	double length, speed;
	if (readRoundaboutAttributes(pRoundaboutXML, res, &id, &length, &speed))
	{
		if (!isIdValid(id))
		{
			res.setErrCode(LOAD_ERR_INVALID_ID);
			res.setErrLine(pRoundaboutXML->GetLineNum());
			res.setErrMsg("Roundabout at line " + to_string(res.getErrLine()) + " has invalid id");
			ret = false;
		}
		else
		{
			int createRes = pModel->createRoundabout(id, length, speed);
			if (processCreateRoundaboutResult(createRes, res, pRoundaboutXML->GetLineNum()))
			{
				if (loadJunction(id, pRoundaboutXML, res))
				{
					if (loadRoundaboutOutputs(pRoundaboutXML, res, id))
						ret = loadRoundaboutGate(pRoundaboutXML, res, id);
					else
						ret = false;
				}
				else
					ret = false;
			}
			else
				ret = false;
		}
	}
	else
		ret = false;

	return ret;
}



bool ModelLoader::processCreateRoundaboutResult(int result, LoadResult& res, int line)
{
	if (result == CREATE_ROUNDABOUT_SUCCESS)
		return true;
	else
		res.setErrLine(line);

	switch (result)
	{
	case CREATE_ROUNDABOUT_ALREADY_EXISTS:
		res.setErrCode(LOAD_ERR_JUNCTION_ALREADY_EXISTS);
		res.setErrMsg("Roundabout at line " + to_string(line) + " has a non-unique id");
		return false;
	case CREATE_ROUNDABOUT_TOO_SHORT:
		res.setErrCode(LOAD_ERR_ROUNDABOUT_TOO_SHORT);
		res.setErrMsg("Roundabout at line " + to_string(line) +
			" has too small length. The minimum roundabout length is " + 
			to_string(MIN_ROUNDABOUT_LENGTH));
		return false;
	case CREATE_ROUNDABOUT_INCORRECT_SPEED:
		res.setErrCode(LOAD_ERR_ROUNDABOUT_INCORRECT_SPEED);
		res.setErrMsg("Roundabout at line " + to_string(line) +
			" has incorrect speed. The speed must be in range (0.0; " + 
			to_string(MAX_ROUNDABOUT_SPEED) + "]");
		return false;
	default:
		res.setErrCode(LOAD_ERR_ROUNDABOUT_UNKNOWN_ERROR);
		res.setErrMsg("Roundabout at line " + to_string(line) + 
			" caused an unknown error. The error code is " + to_string(result));
		return false;
	}
}



bool ModelLoader::loadRoundaboutOutputs(XMLElement* pRoundaboutXML, LoadResult& res,
	int roundaboutId)
{
	bool ret = true;

	XMLElement* pOutputsXML = pRoundaboutXML->FirstChildElement(TAG_ROUNDABOUT_OUTPUTS);
	if (pOutputsXML != nullptr)
	{
		XMLElement* pOutputXML = pOutputsXML->FirstChildElement(TAG_ROUNDABOUT_OUTPUT);
		while (pOutputXML != nullptr && ret)
		{
			ret = loadRoundaboutOutput(pOutputXML, res, roundaboutId);
			pOutputXML = pOutputXML->NextSiblingElement(TAG_ROUNDABOUT_OUTPUT);
		}
	}

	return ret;
}



bool ModelLoader::loadRoundaboutOutput(XMLElement* pOutputXML, LoadResult& res, int roundaboutId)
{
	bool ret = true;

	// get position
	double position;
	XMLError positionErr = pOutputXML->QueryDoubleAttribute(ATTR_ROUNDABOUT_OUTPUT_POSITION, 
		&position);
	processXMLError(positionErr, pOutputXML->GetLineNum(), XMLDocument::ErrorIDToName(positionErr), 
		res);

	// create junction input (output contains input, for consistency)
	if (res.success())
	{
		// generate an unique id for the junction input, without the knowledge of the user because
		// the user will never need to access that input
		int junctionInputId = pModel->generateUniqueIdForJunctionInput();
		ret = createRoundaboutOutputInternalInput(junctionInputId, roundaboutId, pOutputXML, res);
		
		// create the output
		if (ret)
			ret = processAddRoundaboutOutputResult(
				pModel->addRoundaboutOutput(roundaboutId, position, junctionInputId), res,
				pOutputXML->GetLineNum());
	}
	else // error during processing the id attribute
		ret = false;

	return ret;
}



bool ModelLoader::createRoundaboutOutputInternalInput(int junctionInputId, int roundaboutId,
	tinyxml2::XMLElement* pOutputXML, LoadResult& res)
{
	bool ret = true;

	switch (pModel->createJunctionInput(junctionInputId, roundaboutId))
	{
	case CREATE_JUNCTION_INPUT_SUCCESS:
		ret = loadGates(pOutputXML, res, junctionInputId);
		break;
	case CREATE_JUNCTION_INPUT_ALREADY_EXISTS:
		ret = false;
		res.setErrCode(LOAD_ERR_JUNCTION_INPUT_ALREADY_EXISTS);
		res.setErrLine(pOutputXML->GetLineNum());
		res.setErrMsg("Roundabout output defined at line " + to_string(res.getErrLine()) +
			" caused a serious error. The id generated for a hidden input is not unique.");
		break;
	case CREATE_JUNCTION_INPUT_JUNCTION_NOT_FOUND:
		ret = false;
		res.setErrCode(LOAD_ERR_JUNCTION_INPUT_NO_SUCH_JUNCTION);
		res.setErrLine(pOutputXML->GetLineNum());
		res.setErrMsg("Roundabout output defined at line " + to_string(res.getErrLine()) +
			" references a non-existing roundabout");
		break;
	}

	return ret;
}



bool ModelLoader::processAddRoundaboutOutputResult(int result, LoadResult& res, int line)
{
	if (result == ADD_ROUNDABOUT_OUTPUT_SUCCESS)
		return true;
	else
		res.setErrLine(line);

	switch (result)
	{
	case ADD_ROUNDABOUT_OUTPUT_CANT_ADD_TO_ROUNDABOUT:
		res.setErrCode(LOAD_ERR_ADD_ROUNDABOUT_CANT_ADD_OUTPUT);
		res.setErrMsg("Roundabout output at line " + to_string(line) +
			" couldn't be added to the roundabout. The output may overlapp with other output or input.");
		return false;
	case ADD_ROUNDABOUT_OUTPUT_JUNCTION_HAS_WRONG_NUMBER_OF_GATES:
		res.setErrCode(LOAD_ERR_ADD_ROUNDABOUT_OUTPUT_WRONG_NUMBER_OF_GATES);
		res.setErrMsg("Roundabout output at line " + to_string(line) + " must have exactly one gate.");
		return false;
	case ADD_ROUNDABOUT_OUTPUT_JUNCTION_INPUT_TAKEN:
		res.setErrCode(LOAD_ERR_ADD_ROUNDABOUT_OUTPUT_INPUT_TAKEN);
		res.setErrMsg("Roundabout output at line " + to_string(line) + 
			" includes input which is already in use. Serious internal error.");
		return false;
	case ADD_ROUNDABOUT_OUTPUT_NO_SUCH_JUNCTION_INPUT:
		res.setErrCode(LOAD_ERR_ADD_ROUNDABOUT_GATE_NO_SUCH_INPUT);
		res.setErrMsg("Roundabout output at line " + to_string(line) +
			" references a non-existing junction input. Serious internal error.");
		return false;
	case ADD_ROUNDABOUT_OUTPUT_NO_SUCH_ROUNDABOUT:
		res.setErrCode(LOAD_ERR_ADD_ROUNDABOUT_OUTPUT_NO_SUCH_ROUNDABOUT);
		res.setErrMsg("Roundabout output at line " + to_string(line) +
			" references a non-existing roundabout. Serious internal error.");
		return false;
	default:
		res.setErrCode(LOAD_ERR_ADD_ROUNDABOUT_OUTPUT_UNKNOWN_ERROR);
		res.setErrMsg("Roundabout output at line " + to_string(line) +
			" caused an unknown error. Error code: " + to_string(result));
		return false;
	}
}



bool ModelLoader::readRoundaboutAttributes(XMLElement* pRoundaboutXML, LoadResult& res, int* pId,
	double* pLength, double* pSpeed)
{
	int line = pRoundaboutXML->GetLineNum();

	// read id
	XMLError idErr = pRoundaboutXML->QueryIntAttribute(ATTR_ROUNDABOUT_ID, pId);
	processXMLError(idErr, line, XMLDocument::ErrorIDToName(idErr), res);
	if (res.fail())
		return false;

	// read length
	XMLError lengthErr = pRoundaboutXML->QueryDoubleAttribute(ATTR_ROUNDABOUT_LENGTH, pLength);
	processXMLError(lengthErr, line, XMLDocument::ErrorIDToName(lengthErr), res);
	if (res.fail())
		return false;

	// read speed
	XMLError speedErr = pRoundaboutXML->QueryDoubleAttribute(ATTR_ROUNDABOUT_SPEED, pSpeed);
	processXMLError(speedErr, line, XMLDocument::ErrorIDToName(speedErr), res);
	
	return res.success();
}



// lights systems /////////////////////////////////////////////////////////////////////////////////



bool ModelLoader::loadLightsSystems(XMLElement* pLightsSystemsXML, LoadResult& res, int junctionId)
{
	bool ret = true;

	XMLElement* pLightsSystemXML = pLightsSystemsXML->FirstChildElement(TAG_LIGHTS_SYSTEM); // <lights_system>
	while (pLightsSystemXML != nullptr && ret == true)
	{
		ret = loadLightsSystem(pLightsSystemXML, res, junctionId);
		pLightsSystemXML = pLightsSystemXML->NextSiblingElement(TAG_LIGHTS_SYSTEM);
	}

	return ret;
}



bool ModelLoader::loadLightsSystem(XMLElement* pLightsSystemXML, LoadResult& res, int junctionId)
{
	bool ret = true;

	// obtain id
	int id;
	XMLError attrErr = pLightsSystemXML->QueryIntAttribute(ATTR_LIGHTS_SYSTEM_ID, &id);
	processXMLError(attrErr, pLightsSystemXML->GetLineNum(), XMLDocument::ErrorIDToName(attrErr), res);

	if (res.success())
	{
		if (!isIdValid(id))
		{
			res.setErrCode(LOAD_ERR_INVALID_ID);
			res.setErrLine(pLightsSystemXML->GetLineNum());
			res.setErrMsg("Lights system at line " + to_string(res.getErrLine()) + " has invalid id");
			ret = false;
		}
		else
		{
			// create lights system
			switch (pModel->createLightsSystem(id, junctionId))
			{
			case CREATE_LIGHTS_SYSTEM_ALREADY_EXISTS:
				ret = false;
				res.setErrCode(LOAD_ERR_LIGHTS_SYSTEM_ALREADY_EXISTS);
				res.setErrLine(pLightsSystemXML->GetLineNum());
				res.setErrMsg("Lights system at line " + to_string(res.getErrLine()) +
					" has a non-unique id");
				break;
			case CREATE_LIGHTS_SYSTEM_NO_SUCH_JUNCTION:
				ret = false;
				res.setErrCode(LOAD_ERR_LIGHTS_SYSTEM_NO_SUCH_JUNCTION);
				res.setErrLine(pLightsSystemXML->GetLineNum());
				res.setErrMsg("Lights system at line " + to_string(res.getErrLine()) +
					" references a non-existing junction");
				break;
			}

			// load lights and lights groups
			if (res.success())
			{
				// load lights
				XMLElement* pLightsXML = pLightsSystemXML->FirstChildElement(TAG_LIGHTS); // <lights>
				while (pLightsXML != nullptr && ret == true)
				{
					ret = loadLights(pLightsXML, res, id);
					pLightsXML = pLightsXML->NextSiblingElement(TAG_LIGHTS);
				}

				// load lights groups
				if (ret)
				{
					XMLElement* pLightsGroupXML = pLightsSystemXML->FirstChildElement(TAG_LIGHTS_GROUP); // <lights_group>
					while (pLightsGroupXML != nullptr && ret == true)
					{
						ret = loadLightsGroup(pLightsGroupXML, res, id);
						pLightsGroupXML = pLightsGroupXML->NextSiblingElement(TAG_LIGHTS_GROUP);
					}
				}
			}
		}
	}
	else // can't obtain id
		ret = false;

	return ret;
}



// lights /////////////////////////////////////////////////////////////////////////////////////////



bool ModelLoader::loadLights(XMLElement* pLightsXML, LoadResult& res, int lightsSystemId)
{
	bool ret = true;

	// obtain id
	int id;
	XMLError attrErr = pLightsXML->QueryIntAttribute(ATTR_LIGHTS_ID, &id);
	processXMLError(attrErr, pLightsXML->GetLineNum(), XMLDocument::ErrorIDToName(attrErr), res);

	if (res.success())
	{
		if (!isIdValid(id))
		{
			res.setErrCode(LOAD_ERR_INVALID_ID);
			res.setErrLine(pLightsXML->GetLineNum());
			res.setErrMsg("Lights at line " + to_string(res.getErrLine()) + " have invalid id");
			ret = false;
		}
		else
		{
			switch (pModel->createLights(id, lightsSystemId))
			{
			case CREATE_LIGHTS_ALREADY_EXISTS:
				ret = false;
				res.setErrCode(LOAD_ERR_LIGHTS_ALREADY_EXIST);
				res.setErrLine(pLightsXML->GetLineNum());
				res.setErrMsg("Lights at line " + to_string(res.getErrLine()) + " have non-unique id");
				break;
			case CREATE_LIGHTS_NO_SUCH_LIGHTS_SYSTEM:
				ret = false;
				res.setErrCode(LOAD_ERR_LIGHTS_NO_SUCH_LIGHTS_SYSTEM);
				res.setErrLine(pLightsXML->GetLineNum());
				res.setErrMsg("Lights at line " + to_string(res.getErrLine()) +
					" reference a non-existing lights system");
				break;
			}
		}
	}
	else
		ret = false;

	return ret;
}



// lights groups //////////////////////////////////////////////////////////////////////////////////



bool ModelLoader::loadLightsGroup(XMLElement* pLightsGroupXML, LoadResult& res, int lightsSystemId)
{
	int line = pLightsGroupXML->GetLineNum();

	// obtain id
	int id = readLightsGroupId(pLightsGroupXML, res, line);
	if (res.fail())
		return false;

	// obtain min_duration
	double minDuration = readLightsGroupMinDuration(pLightsGroupXML, res, line);
	if (res.fail())
		return false;

	// obtain max_duration
	double maxDuration = readLightsGroupMaxDuration(pLightsGroupXML, res, line);
	if (res.fail())
		return false;

	// obtain duration
	double duration = readLightsGroupDuration(pLightsGroupXML, res, line, maxDuration);
	if (res.fail())
		return false;

	// check if duration is in the proper range
	if (!areLightsGroupDurationsValid(duration, minDuration, maxDuration, res, line))
		return false;

	// read green lights
	vector<int> greenLightsIds;
	readLightsGroupGreenLights(pLightsGroupXML, res, line, greenLightsIds);
	if (res.fail())
		return false;

	// read red lights
	vector<int> redLightsIds;
	readLightsGroupRedLights(pLightsGroupXML, res, line, redLightsIds);
	if (res.fail())
		return false;

	// add to the model
	return createLightsGroup(id, lightsSystemId, greenLightsIds, redLightsIds, duration, 
		minDuration, maxDuration, res, line);
}



int ModelLoader::readLightsGroupId(XMLElement* pLightsGroupXML, LoadResult& res, int line)
{
	int id;
	XMLError idErr = pLightsGroupXML->QueryIntAttribute(ATTR_LIGHTS_GROUP_ID, &id);
	processXMLError(idErr, line, XMLDocument::ErrorIDToName(idErr), res);

	if (res.success() && !isIdValid(id))
	{
		res.setErrCode(LOAD_ERR_INVALID_ID);
		res.setErrLine(pLightsGroupXML->GetLineNum());
		res.setErrMsg("Lights group at line " + to_string(res.getErrLine()) + " have invalid id");
	}

	return id;
}



double ModelLoader::readLightsGroupDuration(tinyxml2::XMLElement* pLightsGroupXML, LoadResult& res,
	int line, double defaultDuration)
{
	// obtain duration
	double duration;
	XMLError durationErr = pLightsGroupXML->QueryDoubleAttribute(ATTR_LIGHTS_GROUP_DURATION, &duration);
	if (durationErr == XML_NO_ATTRIBUTE)
		duration = defaultDuration;
	else
		processXMLError(durationErr, line, XMLDocument::ErrorIDToName(durationErr), res);

	if (res.success() && duration <= 0)
	{
		res.setErrCode(LOAD_ERR_LIGHTS_GROUP_NON_POSITIVE_DURATION);
		res.setErrLine(line);
		res.setErrMsg("Lights group at line " + to_string(line) + " has a non-positive duration");
	}

	return duration;
}



double ModelLoader::readLightsGroupMinDuration(XMLElement* pLightsGroupXML, LoadResult& res,
	int line)
{
	double minDuration;
	XMLError minDurationErr = pLightsGroupXML->QueryDoubleAttribute(ATTR_LIGHTS_GROUP_MIN_DURATION,
		&minDuration);

	if (minDurationErr == XML_NO_ATTRIBUTE)
		minDuration = DEFAULT_MIN_LIGHTS_GROUP_DURATION;
	else
		processXMLError(minDurationErr, line, XMLDocument::ErrorIDToName(minDurationErr), res);

	if (res.success())
	{
		if (minDuration <= 0)
		{
			res.setErrCode(LOAD_ERR_LIGHTS_GROUP_NEGATIVE_NON_POSITIVE_MIN_DURATION);
			res.setErrLine(line);
			res.setErrMsg("Lights group at line " + to_string(line) +
				" has a non-positive minimal duration");
		}
	}

	return minDuration;
}



double ModelLoader::readLightsGroupMaxDuration(XMLElement* pLightsGroupXML, LoadResult& res,
	int line)
{
	double maxDuration;
	XMLError maxDurationErr =
		pLightsGroupXML->QueryDoubleAttribute(ATTR_LIGHTS_GROUP_MAX_DURATION, &maxDuration);
	processXMLError(maxDurationErr, line, XMLDocument::ErrorIDToName(maxDurationErr), res);

	if (maxDurationErr == XML_NO_ATTRIBUTE)
		maxDuration = DEFAULT_MAX_LIGHTS_GROUP_DURATION;
	else
		processXMLError(maxDurationErr, line, XMLDocument::ErrorIDToName(maxDurationErr), res);

	if (res.success())
	{
		if (maxDuration <= 0)
		{
			res.setErrCode(LOAD_ERR_LIGHTS_GROUP_NEGATIVE_NON_POSITIVE_MAX_DURATION);
			res.setErrLine(line);
			res.setErrMsg("Lights group at line " + to_string(line) +
				" has a non-positive maximal duration");
		}
	}

	return maxDuration;
}



bool ModelLoader::areLightsGroupDurationsValid(double duration, double minDuration,
	double maxDuration, LoadResult& res, int line)
{
	if (minDuration > maxDuration)
	{
		res.setErrCode(LOAD_ERR_MIN_DURATION_GREATER_THAN_MAX_DURATION);
		res.setErrLine(line);
		res.setErrMsg("Lights group at line " + to_string(line) +
			" has the minimum duration (" + to_string(minDuration) +
			") greater than the maximum duration (" + to_string(maxDuration) + ")");

		return false;
	}
	else if (duration < minDuration)
	{
		res.setErrCode(LOAD_ERR_TOO_SMALL_DURATION);
		res.setErrLine(line);
		res.setErrMsg("Lights group at line " + to_string(line) +
			" has its duration lower than the minimum duration (" + to_string(minDuration) + ")");

		return false;
	}
	else if (duration > maxDuration)
	{
		res.setErrCode(LOAD_ERR_TOO_BIG_DURATION);
		res.setErrLine(line);
		res.setErrMsg("Lights group at line " + to_string(line) +
			" has its duration greater than the maximum duration (" + to_string(maxDuration) + ")");

		return false;
	}
	else
		return true;
}



void ModelLoader::readLightsGroupGreenLights(XMLElement* pLightsGroupXML,
	LoadResult& res, int line, vector<int>& lightsIds)
{
	XMLElement* pGreenXML = pLightsGroupXML->FirstChildElement(TAG_GREEN_LIGHTS); // <green>
	if (pGreenXML != nullptr)
	{
		XMLElement* pLightsXML = pGreenXML->FirstChildElement(TAG_LIGHTS); // <lights id="..."/>
		int lightsId;
		while (pLightsXML != nullptr && res.success())
		{
			XMLError lightsErr = pLightsXML->QueryIntAttribute(ATTR_LIGHTS_ID, &lightsId);
			processXMLError(lightsErr, line, XMLDocument::ErrorIDToName(lightsErr), res);

			if (res.success())
			{
				lightsIds.push_back(lightsId);
				pLightsXML = pLightsXML->NextSiblingElement(TAG_LIGHTS);
			}
		}
	}
	// if no <green> section, then just don't add any green lights to that group
}



void ModelLoader::readLightsGroupRedLights(XMLElement* pLightsGroupXML, LoadResult& res, int line,
	vector<int>& lightsIds)
{
	XMLElement* pRedXML = pLightsGroupXML->FirstChildElement(TAG_RED_LIGHTS); // <red>
	if (pRedXML != nullptr)
	{
		XMLElement* pLightsXML = pRedXML->FirstChildElement(TAG_LIGHTS); // <lights id="..."/>
		int lightsId;
		while (pLightsXML != nullptr && res.success())
		{
			XMLError lightsErr = pLightsXML->QueryIntAttribute(ATTR_LIGHTS_ID, &lightsId);
			processXMLError(lightsErr, line, XMLDocument::ErrorIDToName(lightsErr), res);

			if (res.success())
			{
				if (!isIdValid(lightsId))
				{
					res.setErrCode(LOAD_ERR_INVALID_ID);
					res.setErrLine(pLightsXML->GetLineNum());
					res.setErrMsg("Lights at line " + to_string(res.getErrLine()) + " have invalid id");
				}
				else
				{
					lightsIds.push_back(lightsId);
					pLightsXML = pLightsXML->NextSiblingElement(TAG_LIGHTS);
				}
			}
		}
	}
	// if no <green> section, then just don't add any green lights to that group
}



bool ModelLoader::createLightsGroup(int lightsGroupId, int lightsSystemId, 
	std::vector<int>& greenLightsIds, std::vector<int>& redLightsIds, double duration, 
	double minDuration, double maxDuration,	LoadResult& res, int line)
{
	int createRes = pModel->createLightsGroup(lightsGroupId, lightsSystemId, greenLightsIds,
		redLightsIds, duration, minDuration, maxDuration);

	switch (createRes)
	{
		case CREATE_LIGHTS_SYSTEM_SUCCESS: // DO NOTHING
			break;
		case CREATE_LIGHTS_GROUP_ALREADY_EXISTS:
			res.setErrCode(LOAD_ERR_LIGHTS_GROUP_ALREADY_EXISTS);
			res.setErrLine(line);
			res.setErrMsg("Lights group at line " + to_string(line) + " has non-unique id");
			return false;
		case CREATE_LIGHTS_GROUP_NOT_ALL_LIGHTS_COVERED:
			res.setErrCode(LOAD_ERR_LIGHTS_GROUP_DOES_NOT_COVER_ALL_LIGHTS);
			res.setErrLine(line);
			res.setErrMsg("Lights group at line " + to_string(line) +
				" doesn't cover all lights in the lights system with id " + to_string(lightsSystemId));
			return false;
		case CREATE_LIGHTS_GROUP_LIGHT_IN_GREEN_AND_RED_AT_THE_SAME_TIME:
			res.setErrCode(LOAD_ERR_LIGHTS_GROUP_LIGHTS_IN_GREEN_AND_RED_AT_THE_SAME_TIME);
			res.setErrLine(line);
			res.setErrMsg("Lights group at line " + to_string(line) +
				" has lights which are in green and red at the same time");
			return false;
		case CREATE_LIGHTS_GROUP_LIGHTS_FROM_MULTIPLE_SYSTEMS:
			res.setErrCode(LOAD_ERR_LIGHTS_GROUP_LIGHTS_FROM_DIFFERENT_SYSTEM);
			res.setErrLine(line);
			res.setErrMsg("Lights group at line " + to_string(line) + 
				" has lights which are part of a different lights system");
			return false;
		default: // all other errors should be catched before, here just in case
			res.setErrCode(LOAD_ERR_LIGHTS_GROUP_UNKNOWN_ERROR);
			res.setErrLine(line);
			res.setErrMsg("Lights group at line " + to_string(line) +
				" caused an unexpected error during creating the lights group. The error code was " 
				+ to_string(createRes));
			return false;
	}

	return true;
}



// junction inputs ////////////////////////////////////////////////////////////////////////////////



bool ModelLoader::loadJunctionInputs(XMLElement* pInputsXML, LoadResult& res, 
	int junctionId)
{
	bool ret = true;

	XMLElement* pInputXML = pInputsXML->FirstChildElement(TAG_INPUT);
	while (pInputXML != nullptr && ret == true)
	{
		ret = loadJunctionInput(pInputXML, res, junctionId);
		pInputXML = pInputXML->NextSiblingElement(TAG_INPUT);
	}

	return ret;
}



bool ModelLoader::loadJunctionInput(tinyxml2::XMLElement* pInputXML, LoadResult& res, 
	int junctionId)
{
	bool ret = true;

	// get id
	int id;
	XMLError attrErr = pInputXML->QueryIntAttribute(ATTR_JUNCTION_INPUT_ID, &id);
	processXMLError(attrErr, pInputXML->GetLineNum(), XMLDocument::ErrorIDToName(attrErr), res);

	// go deeper
	if (res.success())
	{
		if (!isIdValid(id))
		{
			res.setErrCode(LOAD_ERR_INVALID_ID);
			res.setErrLine(pInputXML->GetLineNum());
			res.setErrMsg("Junction input at line " + to_string(res.getErrLine()) + " has invalid id");
			ret = false;
		}
		else
		{
			switch (pModel->createJunctionInput(id, junctionId))
			{
			case CREATE_JUNCTION_INPUT_SUCCESS:
				ret = loadGates(pInputXML, res, id);
				break;
			case CREATE_JUNCTION_INPUT_ALREADY_EXISTS:
				ret = false;
				res.setErrCode(LOAD_ERR_JUNCTION_INPUT_ALREADY_EXISTS);
				res.setErrLine(pInputXML->GetLineNum());
				res.setErrMsg("Junction input defined at line " + to_string(res.getErrLine()) +
					" has an id which is in use by another junction input");
				break;
			case CREATE_JUNCTION_INPUT_JUNCTION_NOT_FOUND:
				ret = false;
				res.setErrCode(LOAD_ERR_JUNCTION_INPUT_NO_SUCH_JUNCTION);
				res.setErrLine(pInputXML->GetLineNum());
				res.setErrMsg("Junction input defined at line " + to_string(res.getErrLine()) +
					" references non-existing junction");
				break;
			}
		}
	}
	else // error during processing the id attribute
		ret = false;

	return ret;
}



// gates //////////////////////////////////////////////////////////////////////////////////////////



bool ModelLoader::loadGates(tinyxml2::XMLElement* pInputXML, LoadResult& res, int inputId)
{
	bool ret = true;
	XMLElement* pGateXML = pInputXML->FirstChildElement();

	while (pGateXML != nullptr && ret)
	{
		ret = loadGate(pGateXML, res, inputId);
		pGateXML = pGateXML->NextSiblingElement();
	}

	return ret;
}



bool ModelLoader::loadGate(XMLElement* pGateXML, LoadResult& res, int inputId)
{
	const char* type = pGateXML->Value(); // e.g. always_open_gate, conflict_gate...

	// obtain id
	int id;
	XMLError idError = pGateXML->QueryIntAttribute(ATTR_GATE_ID, &id);
	if (idError == XML_NO_ATTRIBUTE)
	{
		res.setErrCode(LOAD_ERR_GATE_NO_ID);
		res.setErrLine(pGateXML->GetLineNum());
		res.setErrMsg("Gate at line " + to_string(res.getErrLine()) + " hasn't got an id");
	}
	else
		processXMLError(idError, pGateXML->GetLineNum(), XMLDocument::ErrorIDToName(idError), res);

	if (res.fail())
		return false;

	if (!isIdValid(id))
	{
		res.setErrCode(LOAD_ERR_INVALID_ID);
		res.setErrLine(pGateXML->GetLineNum());
		res.setErrMsg("Gate at line " + to_string(res.getErrLine()) + " have invalid id");
		return false;
	}

	if (strcmp(type, TAG_ALWAYS_OPEN_GATE) == 0)
		return createAlwaysOpenGate(id, res, inputId, pGateXML->GetLineNum());
	else if (strcmp(type, TAG_ALWAYS_CLOSED_GATE) == 0)
		return createAlwaysClosedGate(id, res, inputId, pGateXML->GetLineNum());
	else if (strcmp(type, TAG_COMPOUND_GATE) == 0)
		return createCompoundGate(id, res, inputId, pGateXML);
	else if (strcmp(type, TAG_CONFLICT_GATE) == 0)
		return createConflictGate(id, res, inputId, pGateXML);
	else if (strcmp(type, TAG_LIGHTS_GATE) == 0)
		return createLightsGate(id, res, inputId, pGateXML);
	else if (strcmp(type, TAG_ROUNDABOUT_GATE) == 0)
		/*
			the gate will be created separately in roundabout to assure data integrity, but it must
			be included here too because otherwise it would return the unknown gate error.
		*/
		return true;
	else // unknown gate
	{
		res.setErrCode(LOAD_ERR_UNKNOWN_GATE);
		res.setErrLine(pGateXML->GetLineNum());
		res.setErrMsg("Unknown gate type \"" + string(type) + "\" at line " + 
			to_string(res.getErrLine()));
		return false;
	}
}



bool ModelLoader::processGateError(int err, LoadResult& res, int line)
{
	if (err == CREATE_GATE_SUCCESS)
		return true;

	res.setErrLine(line);

	switch (err)
	{
	case CREATE_GATE_NO_SUCH_INPUT:
		res.setErrCode(LOAD_ERR_GATE_NO_SUCH_INPUT);
		res.setErrMsg("Gate at line " + to_string(line) + 
			" references a non-existing junction input");
		return false;
	case CREATE_GATE_ALREADY_EXISTS:
		res.setErrCode(LOAD_ERR_GATE_ALREADY_EXISTS);
		res.setErrMsg("Gate at line " + to_string(line) + " has a non-unique id");
		return false;
	case CREATE_GATE_NO_SUCH_GATE:
		res.setErrCode(LOAD_ERR_GATE_NO_SUCH_GATE);
		res.setErrMsg("Gate at line " + to_string(line) + " references a non-existing gate");
		return false;
	case CREATE_GATE_NO_COMPONENTS:
		res.setErrCode(LOAD_ERR_GATE_NO_COMPONENTS);
		res.setErrMsg("Compound gate at line " + to_string(line) + " doesn't have any components");
		return false;
	case CREATE_GATE_COMPONENT_ALREADY_IN_USE:
		res.setErrCode(LOAD_ERR_GATE_COMPONENT_IN_USE);
		res.setErrMsg("Compound gate at line " + to_string(line) + 
			" contains a component which is already in use");
		return false;
	case CREATE_GATE_NO_SUCH_LIGHTS:
		res.setErrCode(LOAD_ERR_GATE_NO_SUCH_LIGHTS);
		res.setErrMsg("Lights gate at line " + to_string(line) + 
			" references a non-existing lights");
		return false;
	case CREATE_ROUNDABOUT_GATE_NO_SUCH_ROUNDABOUT:
		res.setErrCode(LOAD_ERR_ROUNDABOUT_GATE_NO_SUCH_ROUNDABOUT);
		res.setErrMsg("Roundabout gate at line " + to_string(line) + 
			" references a non-existing roundabout");
		return false;
	case CREATE_ROUNDABOUT_GATE_CANT_CONNECT:
		res.setErrCode(LOAD_ERR_ROUNDABOUT_GATE_CANT_CONNECT);
		res.setErrMsg("Roundabout gate at line " + to_string(line) +
			" couldn't be attached to the roundabout. Check if it its position doesn't overlapp " +
			"with other gate or output. Also, the position may be negative or greater than the " +
			"roundabout length.");
		return false;
	case CREATE_GATE_NEGATIVE_CONFLICT_POSITION:
		res.setErrCode(LOAD_ERR_CONFLICT_GATE_NEGATIVE_CONFLICT_POSITION);
		res.setErrMsg("Conflict gate at line " + to_string(line) + 
			" has a conflict with a negative position.");
		return false;
	case CREATE_GATE_NEGATIVE_REQUIRED_DURATION:
		res.setErrCode(LOAD_ERR_CONFLICT_GATE_NEGATIVE_DURATION);
		res.setErrMsg("Conflict gate at line " + to_string(line) +
			" has negative duration.");
		return false;
	case CREATE_GATE_NEGATIVE_CONFLICT_START:
		res.setErrCode(LOAD_ERR_CONFLICT_GATE_NEGATIVE_START);
		res.setErrMsg("Conflict gate at line " + to_string(line) +
			" has a conflict with a negative start.");
		return false;
	case CREATE_GATE_CONFLICT_ROAD_NOT_FOUND:
		res.setErrCode(LOAD_ERR_CONFLICT_GATE_CONFLICT_ROAD_NOT_FOUND);
		res.setErrMsg("Conflict gate at line " + to_string(line) +
			" references a non-existing or incorrect conflict road.");
		return false;
	default:
		res.setErrCode(LOAD_ERR_GATE_UNKNOWN_ERROR);
		res.setErrMsg("Creating gate at line " + to_string(line) + 
			" caused an unknown error with the error code " + to_string(err));
		return false;
	}
}



// always open gate ///////////////////////////////////////////////////////////////////////////////



bool ModelLoader::createAlwaysOpenGate(int id, LoadResult& res, int inputId, int line)
{
	int creationRes = pModel->createAlwaysOpenGate(id, inputId);

	return processGateError(creationRes, res, line);
}



// always closed gate /////////////////////////////////////////////////////////////////////////////



bool ModelLoader::createAlwaysClosedGate(int id, LoadResult& res, int inputId, int line)
{
	int creationRes = pModel->createAlwaysClosedGate(id, inputId);

	return processGateError(creationRes, res, line);
}



// compound gate //////////////////////////////////////////////////////////////////////////////////



bool ModelLoader::createCompoundGate(int id, LoadResult& res, int inputId, 
	XMLElement* pCompoundGateXML)
{
	bool ret = true;
	int line = pCompoundGateXML->GetLineNum();

	// read component gates ids
	vector<int> componentGatesIds;
	/*
		Component gates which are of the type conflict gate are not added initially to the 
		compund gate. Instead they are stored and added later after all conflict gates are 
		created. The conflict gates ids will be stored here.
	*/
	vector<int> componentConflictGatesIds;
	int componentId;
	XMLElement* pComponentGateXML = pCompoundGateXML->FirstChildElement(TAG_COMPONENT_GATE);
	while (pComponentGateXML != nullptr && ret)
	{
		XMLError idError = pComponentGateXML->QueryIntAttribute(ATTR_COMPONENT_GATE_ID, &componentId);
		processXMLError(idError, line, XMLDocument::ErrorIDToName(idError), res);

		if (res.success())
		{
			if (isInConflictGates(componentId))
				componentConflictGatesIds.push_back(componentId);
			else
				componentGatesIds.push_back(componentId);

			pComponentGateXML = pComponentGateXML->NextSiblingElement(TAG_COMPONENT_GATE);
		}
		else
			ret = false;
	}

	// create the gate
	if (ret)
	{
		int creationRes = pModel->createCompoundGate(id, inputId, componentGatesIds);
		ret = processGateError(creationRes, res, line);
		
		// if has conflict gates put it away for later
		if (ret && componentConflictGatesIds.size() > 0)
		{
			compoundGatesWithConflicts[id] = componentConflictGatesIds;
		}
	}

	return ret;
}



// conflict gate //////////////////////////////////////////////////////////////////////////////////



bool ModelLoader::createConflictGate(int id, LoadResult& res, int inputId, 
	XMLElement* pConflictGateXML)
{
	bool ret = true;

	// create the temporary conflict gate
	ConflictGateXML gate(id, pConflictGateXML->GetLineNum(), inputId);

	// read conflicts
	vector<int> roadsIds;
	double start, duration, position;
	int numOfConflicts = 0;
	XMLElement* pConflictXML = pConflictGateXML->FirstChildElement(TAG_CONFLICT);
	while (pConflictXML != nullptr && ret)
	{
		if (loadConflict(pConflictXML, roadsIds, &start, &duration, &position, res))
		{
			gate.addConflict(roadsIds, start, duration, position);
			pConflictXML = pConflictXML->NextSiblingElement(TAG_CONFLICT);
			++numOfConflicts;
		}
		else
			ret = false;
	}

	if (ret && numOfConflicts > 0)
	{
		conflictGates.push_back(gate);
	}
	else if (ret && numOfConflicts == 0)
	{
		res.setErrCode(LOAD_ERR_NO_CONFLICTS);
		res.setErrLine(pConflictGateXML->GetLineNum());
		res.setErrMsg("Conflict gate at line " + to_string(res.getErrLine()) +
			" doesn't have any conflicts");
		ret = false;
	}

	return ret;
}



bool ModelLoader::validateRoadsInConflictGates(LoadResult& res)
{
	bool ret = true;

	int creationRes;
	auto itGates = conflictGates.begin();
	while (itGates != conflictGates.end())
	{
		creationRes = pModel->createConflictGate((*itGates).getId(), (*itGates).getInputId(),
			(*itGates).getConflictRoadsIds(), (*itGates).getConflictsStarts(),
			(*itGates).getConflictsDurations(), (*itGates).getConflictsPositions());

		if (processGateError(creationRes, res, (*itGates).getLine()))
			itGates = conflictGates.erase(itGates);
		else
		{
			ret = false;
			++itGates;
		}
	}

	return ret;
}



bool ModelLoader::loadConflictGatesWithRoads(LoadResult& res)
{
	int s1 = max((int) conflictGates.size(), 1);
	int s2 = max((int) roadsFromConflictGates.size(), 1);
	int s3 = max((int) multilaneRoadsFromConflictGates.size(), 1);
	int maxReps = s1 * s2 * s3;
	int reps = 0;
	bool ret = false;

	while (reps < maxReps && ret == false)
	{
		ret = validateRoadsInConflictGates(res);
		ret = addClassicalRoadsFromConflictGates(res) && ret;
		ret = addFinalRoadsFromConflictGates(res) && ret;
		ret = addClassicalMultilaneRoadsFromConflictGates(res) && ret;
		ret = addFinalMultilaneRoadsFromConflictGates(res) && ret;

		++reps;
	}

	if (ret)
	{
		res.setErrCode(LOAD_SUCCESS);
		res.setErrLine(-1);
		res.setErrMsg("");
	}
	else
	{
		// run once again to get the proper error
		ret = validateRoadsInConflictGates(res) &&
			addClassicalRoadsFromConflictGates(res) &&
			addFinalRoadsFromConflictGates(res) &&
			addClassicalMultilaneRoadsFromConflictGates(res) &&
			addFinalMultilaneRoadsFromConflictGates(res);
	}

	return ret;
}



bool ModelLoader::isInConflictGates(int id)
{
	for (ConflictGateXML& gate : conflictGates)
	{
		if (gate.getId() == id)
			return true;
	}

	return false;
}



bool ModelLoader::loadConflict(tinyxml2::XMLElement* pConflictXML, std::vector<int>& roadsIds, 
	double* pStart,	double* pDuration, double* pPosition, LoadResult& res)
{
	bool ret = true;

	// read conflict roads
	vector<int> conflictRoadsIds;
	if (readConflictRoads(pConflictXML, conflictRoadsIds, res))
	{
		if (conflictRoadsIds.size() > 0)
		{
			double start, conflictPos, duration;
			readConflictAttributes(pConflictXML, &start, &conflictPos, &duration, 
				conflictRoadsIds.size() > 1, res);
			if (res.success())
			{
				*pPosition = conflictPos;
				*pStart = start;
				*pDuration = duration;
				for (int id : conflictRoadsIds)
					roadsIds.push_back(id);
			}
			else
				ret = false;
		}
		else
			ret = false;
	}
	else
		ret = false;

	return ret;
}



bool ModelLoader::readConflictRoads(tinyxml2::XMLElement* pConflictXML, 
	std::vector<int>& conflictRoadsIds,	LoadResult& res)
{
	bool ret = true;

	XMLElement* pConflictRoadXML = pConflictXML->FirstChildElement(TAG_CONFLICT_ROAD);
	int conflictRoadId;
	while (pConflictRoadXML != nullptr && ret)
	{
		XMLError idError = pConflictRoadXML->QueryIntAttribute(ATTR_CONFLICT_ROAD_ID, 
			&conflictRoadId);
		processXMLError(idError, pConflictRoadXML->GetLineNum(),
			XMLDocument::ErrorIDToName(idError), res);
		if (res.success())
		{
			if (find(conflictRoadsIds.begin(), conflictRoadsIds.end(), conflictRoadId) ==
				conflictRoadsIds.end()) // unique id in conflicts
			{
				conflictRoadsIds.push_back(conflictRoadId);
				pConflictRoadXML = pConflictRoadXML->NextSiblingElement(TAG_CONFLICT_ROAD);
			}
			else // such id was already included
			{
				res.setErrCode(LOAD_ERR_CONFLICT_ROAD_MULTIPLE_OCCURRENCES);
				res.setErrLine(pConflictRoadXML->GetLineNum());
				res.setErrMsg("Conflict road at line " + to_string(res.getErrLine()) + 
					" was already included in conflict roads");
				ret = false;
			}
		}
		else
			ret = false;
	}

	return ret;
}



bool ModelLoader::readConflictAttributes(tinyxml2::XMLElement* pConflictXML, double* pStart,
	double* pConflictPos, double* pDuration, bool multipleConflictRoads, LoadResult& res)
{
	int conflictLine = pConflictXML->GetLineNum();

	// read duration
	XMLError durationErr = pConflictXML->QueryDoubleAttribute(ATTR_CONFLICT_DURATION,
		pDuration);
	processXMLError(durationErr, conflictLine, XMLDocument::ErrorIDToName(durationErr), res);

	if (res.fail())
		return false;

	// read conflict position
	XMLError conflictPosErr = pConflictXML->QueryDoubleAttribute(ATTR_CONFLICT_POSITON,
		pConflictPos);
	processXMLError(conflictPosErr, conflictLine, XMLDocument::ErrorIDToName(conflictPosErr), res);

	if (res.fail())
		return false;

	// read start position or assign 0.0 if there is only one conflict road
	*pStart = 0.0;
	if (multipleConflictRoads)
	{
		XMLError startErr = pConflictXML->QueryDoubleAttribute(ATTR_CONFLICT_START, pStart);
		processXMLError(startErr, conflictLine, XMLDocument::ErrorIDToName(startErr), res);
		if (res.fail())
			return false;
	}

	return true;
}



bool ModelLoader::addConflictGatesToCompoundGates(LoadResult& res)
{
	bool ret = true;
	int compoundGateId;
	int addRes;
	for (auto& pairGateConflicts : compoundGatesWithConflicts)
	{
		compoundGateId = pairGateConflicts.first;
		vector<int>& components = pairGateConflicts.second;

		for (int componentId : components)
		{
			addRes = pModel->addComponentGate(compoundGateId, componentId);

			if (addRes != ADD_COMPONENT_GATE_SUCCESS)
			{
				ret = false;
				switch (addRes)
				{
				case ADD_COMPONENT_GATE_ALREADY_IN_USE:
					res.setErrCode(LOAD_ERR_ADD_COMPONENT_GATE_ALREADY_IN_USE);
					res.setErrMsg("Component gate with id " + to_string(componentId) +
						" is already in use.");
					break;
				case ADD_COMPONENT_GATE_NO_SUCH_GATE:
					res.setErrCode(LOAD_ERR_ADD_COMPONENT_GATE_NO_SUCH_COMPOUND_GATE);
					res.setErrMsg("There is no gate with id " + to_string(compoundGateId) +
						", so a component gate can't be added to it.");
					break;
				case ADD_COMPONENT_GATE_NOT_COMPOUND_GATE:
					res.setErrCode(LOAD_ERR_ADD_COMPONENT_GATE_NOT_COMPOUND_GATE);
					res.setErrMsg("The gate with id " + to_string(compoundGateId) +
						" is not a compound gate.");
					break;
				case ADD_COMPONENT_GATE_NO_SUCH_COMPONENT:
					res.setErrCode(LOAD_ERR_ADD_COMPONENT_GATE_NO_SUCH_COMPONENT);
					res.setErrMsg("Component gate with id " + to_string(componentId) +
						" doesn't exist.");
					break;
				default:
					res.setErrCode(LOAD_ERR_ADD_COMPONENT_GATE_UNKNOWN_ERROR);
					res.setErrMsg("Unknown error occurred during processing a compound gate with id " +
						to_string(compoundGateId) + " and its component with id " +
						to_string(componentId));
					break;
				}
			}
		}
	}

	return ret;
}



// lights gate ////////////////////////////////////////////////////////////////////////////////////



bool ModelLoader::createLightsGate(int id, LoadResult& res, int inputId, 
	XMLElement* pLightsGateXML)
{
	// read lights id
	int lightsId;
	XMLError lightsErr = pLightsGateXML->QueryIntAttribute(ATTR_LIGHTS_GATE_LIGHTS, &lightsId);
	processXMLError(lightsErr, pLightsGateXML->GetLineNum(), XMLDocument::ErrorIDToName(lightsErr), 
		res);

	if (res.success())
	{
		int line = pLightsGateXML->GetLineNum();
		int creationRes = pModel->createGradeSeparatedLightsGate(id, inputId, lightsId);
		if (processGateError(creationRes, res, line))
			return true;
		else
			return false;
	}
	else
		return false;
}



// roundabout gate ////////////////////////////////////////////////////////////////////////////////



bool ModelLoader::loadRoundaboutGate(XMLElement* pRoundaboutXML, LoadResult& res, int roundaboutId)
{
	bool ret = true;

	/*
		When this function is called it should be already checked if there is the <inputs> section
		so the errors in res won't be set, but null checks will be performed anyway and false
		will be returned in case of null.
	*/
	XMLElement* pInputsXML = pRoundaboutXML->FirstChildElement(TAG_JUNCTION_INPUTS);
	if (pInputsXML != nullptr)
	{
		XMLElement* pInputXML = pInputsXML->FirstChildElement(TAG_INPUT);
		XMLElement* pRoundaboutGateXML = nullptr;
		int gateLine;
		while (pInputXML != nullptr && ret)
		{
			int inputId;
			XMLError inputIdErr = pInputXML->QueryIntAttribute(ATTR_JUNCTION_INPUT_ID, &inputId);
			processXMLError(inputIdErr, pInputXML->GetLineNum(),
				XMLDocument::ErrorIDToName(inputIdErr), res);

			if (res.success())
			{
				// look for a roundabout gate
				pRoundaboutGateXML = pInputXML->FirstChildElement(TAG_ROUNDABOUT_GATE);
				if (pRoundaboutGateXML != nullptr)
				{
					gateLine = pRoundaboutGateXML->GetLineNum();

					int id;
					XMLError idErr = pRoundaboutGateXML->QueryIntAttribute(ATTR_GATE_ID, &id);
					processXMLError(idErr, gateLine, XMLDocument::ErrorIDToName(idErr), res);

					if (res.success())
					{
						if (isIdValid(id))
						{
							double position;
							XMLError posErr = pRoundaboutGateXML->QueryDoubleAttribute(
								ATTR_ROUNDABOUT_GATE_POSITION, &position);
							processXMLError(posErr, gateLine, XMLDocument::ErrorIDToName(posErr), 
								res);

							if (res.success())
							{
								int createRes = pModel->createRoundaboutGate(id, roundaboutId, 
									inputId, position);
								ret = processGateError(createRes, res, gateLine);
							}
							else
								ret = false;
						}
						else // invalid id
						{
							ret = false;
							res.setErrCode(LOAD_ERR_INVALID_ID);
							res.setErrLine(gateLine);
							res.setErrMsg("Roundabout gate at line " + to_string(gateLine) + 
								" has an invalid id");
						}
					}
					else
						ret = false;
				}
				else // no roundabout gate defined
				{
					ret = false;
					res.setErrCode(LOAD_ERR_NO_ROUNDABOUT_GATE);
					res.setErrLine(pInputXML->GetLineNum());
					res.setErrMsg("Roundabout input at line " + to_string(res.getErrLine()) +
						" doesn't have a roundabout gate specified");
				}
			}
			else
				ret = false;

			pInputXML = pInputXML->NextSiblingElement(TAG_INPUT);
		}
	}
	else
		ret = true;

	return ret;
}



// generators /////////////////////////////////////////////////////////////////////////////////////



bool ModelLoader::loadGenerators(XMLDocument& doc, LoadResult& res)
{
	bool ret = true;

	XMLElement* pGeneratorsXML = doc.FirstChildElement(TAG_GENERATORS);
	if (pGeneratorsXML != nullptr)
	{
		XMLElement* pGeneratorXML = pGeneratorsXML->FirstChildElement(); // any type of generator
		while (pGeneratorXML != nullptr && ret)
		{
			ret = loadGenerator(pGeneratorXML, res);
			pGeneratorXML = pGeneratorXML->NextSiblingElement(); // any type of generator
		}
	}
	else // no generators section
	{
		ret = false;
		res.setErrCode(LOAD_ERR_NO_GENERATORS_SECTION);
		res.setErrMsg("There is no <generators> section.");
	}

	return ret;
}



bool ModelLoader::loadGenerator(XMLElement* pGeneratorXML, LoadResult& res)
{
	bool ret = true;

	// id
	int id;
	XMLError idErr = pGeneratorXML->QueryIntAttribute(ATTR_GENERATOR_ID, &id);
	processXMLError(idErr, pGeneratorXML->GetLineNum(), XMLDocument::ErrorIDToName(idErr), res);

	if (res.success())
	{
		if (isIdValid(id))
		{
			// template car
			int templateCar;
			XMLError templateErr = pGeneratorXML->QueryIntAttribute(ATTR_GENERATOR_TEMPLATE_CAR, &templateCar);
			processXMLError(templateErr, pGeneratorXML->GetLineNum(), XMLDocument::ErrorIDToName(templateErr), res);

			if (res.success())
				ret = loadGeneratorByType(pGeneratorXML, res, id, templateCar);
			else
			{
				res.setErrCode(LOAD_ERR_GENERATOR_NO_TEMPLATE_CAR);
				res.setErrLine(pGeneratorXML->GetLineNum());
				res.setErrMsg("Generator at line " + to_string(res.getErrLine()) + 
					" doesn't have a template car.");
				ret = false;
			}
		}
		else // invalid id
		{
			res.setErrCode(LOAD_ERR_INVALID_ID);
			res.setErrLine(pGeneratorXML->GetLineNum());
			res.setErrMsg("Generator at line " + to_string(res.getErrLine()) +
				" has an invalid id.");
			ret = false;
		}
	}
	else // error read id
		ret = false;

	return ret;
}



bool ModelLoader::loadGeneratorByType(XMLElement* pGeneratorXML, LoadResult& res,
	int generatorId, int templateCarId)
{
	bool ret = true;

	const char* type = pGeneratorXML->Value();

	if (strcmp(type, UNIFORM_GENERATOR_STRING) == 0)
		ret = loadUniformGenerator(pGeneratorXML, res, generatorId, templateCarId);
	else // unknown generator type
	{
		res.setErrCode(LOAD_ERR_UNKNOWN_GENERATOR);
		res.setErrLine(pGeneratorXML->GetLineNum());
		res.setErrMsg("Unknown generator type <" + string(type) + "> at line " + 
			to_string(res.getErrLine()));
		ret = false;
	}

	if (ret == true)
		ret = loadInnerGenerators(pGeneratorXML, res, generatorId);

	return ret;
}



bool ModelLoader::loadUniformGenerator(XMLElement* pUniformGeneratorXML, LoadResult& res,
	int generatorId, int templateCarId)
{
	int line = pUniformGeneratorXML->GetLineNum();

	int carsPerPeriod;
	double period;
	
	if (readUniformGeneratorAttributes(pUniformGeneratorXML, res, &carsPerPeriod, &period))
	{
		string typeStr = UNIFORM_GENERATOR_STRING;
		vector<string> params;
		params.push_back(to_string(carsPerPeriod));
		params.push_back(to_string(period));

		return createGenerator(generatorId, typeStr, params, res, line, templateCarId);
	}
	else
		return false;
}



bool ModelLoader::readUniformGeneratorAttributes(tinyxml2::XMLElement* pUniformGeneratorXML,
	LoadResult& res, int* pCarsPerPeriod, double* pPeriod)
{
	int line = pUniformGeneratorXML->GetLineNum();

	// obtain cars per period
	XMLError carsErr = pUniformGeneratorXML->QueryIntAttribute(ATTR_GENERATOR_CARS_PER_PERIOD,
		pCarsPerPeriod);
	processXMLError(carsErr, line, XMLDocument::ErrorIDToName(carsErr), res);

	if (res.fail())
		return false;

	if (*pCarsPerPeriod < 0)
	{
		res.setErrCode(LOAD_ERR_GENERATOR_NEGATIVE_CARS_PER_PERIOD);
		res.setErrLine(line);
		res.setErrMsg("Generator at line " + to_string(line) +
			" has negative number of cars per period.");
		return false;
	}

	// obtain period
	XMLError periodErr = pUniformGeneratorXML->QueryDoubleAttribute(ATTR_GENERATOR_PERIOD, pPeriod);
	processXMLError(periodErr, line, XMLDocument::ErrorIDToName(periodErr), res);

	if (res.fail())
		return false;

	if (*pPeriod <= 0.0)
	{
		res.setErrCode(LOAD_ERR_GENERATOR_TOO_SMALL_PERIOD);
		res.setErrLine(line);
		res.setErrMsg("Generator at line " + to_string(line) + " has a negative period.");
		return false;
	}

	return true;
}



bool ModelLoader::createGenerator(int id, string& type, std::vector<string>& params, 
	LoadResult& res, int line, int templateCarId)
{
	int globalId;
	switch (pModel->createGenerator(id, type, params, &globalId))
	{
	case CREATE_GENERATOR_SUCCESS:
		generatorsTemplateCars[globalId] = templateCarId;
		return true;
	case CREATE_GENERATOR_PARSING_ERROR:
		res.setErrCode(LOAD_ERR_GENERATOR_PARSING_ERROR);
		res.setErrLine(line);
		res.setErrMsg("Couldn't parse generator at line " + to_string(line));
		return false;
	case CREATE_GENERATOR_ALREADY_EXISTS:
		res.setErrCode(LOAD_ERR_GENERATOR_ALREADY_EXISTS);
		res.setErrLine(line);
		res.setErrMsg("Generator at line " + to_string(line) + " has a non-unique id.");
		return false;
	default:
		res.setErrCode(LOAD_ERR_GENERATOR_UNKNOWN_ERROR);
		res.setErrLine(line);
		res.setErrMsg("Generator at line " + to_string(line) + " caused an unknown error.");
		return false;
	}
}



bool ModelLoader::loadInnerGenerators(tinyxml2::XMLElement* pGeneratorXML, LoadResult& res,
	int generatorId)
{
	bool ret = true;

	XMLElement* pInnerGeneratorXML = pGeneratorXML->FirstChildElement();
	while (pInnerGeneratorXML != nullptr && ret)
	{
		ret = loadInnerGenerator(pInnerGeneratorXML, res, generatorId);
		pInnerGeneratorXML = pInnerGeneratorXML->NextSiblingElement();
	}

	return ret;
}



bool ModelLoader::loadInnerGenerator(tinyxml2::XMLElement* pInnerGeneratorXML, LoadResult& res,
	int generatorId)
{
	bool ret = true;

	const char* type = pInnerGeneratorXML->Value();

	int templateCar;
	XMLError err = pInnerGeneratorXML->QueryIntAttribute(ATTR_GENERATOR_TEMPLATE_CAR, &templateCar);
	processXMLError(err, pInnerGeneratorXML->GetLineNum(), XMLDocument::ErrorIDToName(err), res);

	if (res.success())
	{
		if (strcmp(type, UNIFORM_GENERATOR_STRING) == 0)
			ret = loadInnerUniformGenerator(pInnerGeneratorXML, res, generatorId, templateCar);
		else // unknown generator type
		{
			res.setErrCode(LOAD_ERR_UNKNOWN_GENERATOR);
			res.setErrLine(pInnerGeneratorXML->GetLineNum());
			res.setErrMsg("Unknown generator type <" + string(type) + "> at line " +
				to_string(res.getErrLine()));
			ret = false;
		}
	}
	else
	{
		res.setErrCode(LOAD_ERR_GENERATOR_NO_TEMPLATE_CAR);
		res.setErrLine(pInnerGeneratorXML->GetLineNum());
		res.setErrMsg("Generator at line " + to_string(res.getErrLine()) + 
			" doesn't have a template car.");
		ret = false;
	}

	return ret;
}



bool ModelLoader::loadInnerUniformGenerator(tinyxml2::XMLElement* pUniformGeneratorXML, 
	LoadResult& res, int generatorId, int templateCar)
{
	int line = pUniformGeneratorXML->GetLineNum();

	int carsPerPeriod;
	double period;

	if (readUniformGeneratorAttributes(pUniformGeneratorXML, res, &carsPerPeriod, &period))
	{
		string typeStr = UNIFORM_GENERATOR_STRING;
		vector<string> params;
		params.push_back(to_string(carsPerPeriod));
		params.push_back(to_string(period));

		return attachGenerator(generatorId, typeStr, params, res, line, templateCar);
	}
	else
		return false;
}



bool ModelLoader::attachGenerator(int id, string& type, std::vector<string>& params, 
	LoadResult& res, int line, int templateCar)
{
	int globalId;
	switch (pModel->attachToGenerator(id, type, params, &globalId))
	{
	case ATTACH_TO_GENERATOR_SUCCESS:
		generatorsTemplateCars[globalId] = templateCar;
		return true;
	case ATTACH_TO_GENERATOR_NO_SUCH_GENERATOR:
		res.setErrCode(LOAD_ERR_ATTACH_GENERATOR_NO_SUCH_GENERATOR);
		res.setErrLine(line);
		res.setErrMsg("Inner generator at line " + to_string(line) + 
			" references a non-existing generator.");
		return false;
	case ATTACH_TO_GENERATOR_PARSING_ERROR:
		res.setErrCode(LOAD_ERR_GENERATOR_PARSING_ERROR);
		res.setErrLine(line);
		res.setErrMsg("Couldn't parse the generator at line " + to_string(line));
		return false;
	default:
		res.setErrCode(LOAD_ERR_ATTACH_GENERATOR_UNKNOWN_ERROR);
		res.setErrLine(line);
		res.setErrMsg("Generator at line " + to_string(line) + " caused an unknown error.");
		return false;
	}
}



// roads //////////////////////////////////////////////////////////////////////////////////////////



bool ModelLoader::loadRoads(XMLDocument& doc, LoadResult& res)
{
	bool ret = true;

	XMLElement* pRoadsXML = doc.FirstChildElement(TAG_ROADS);
	if (pRoadsXML != nullptr)
	{
		XMLElement* pRoadXML = pRoadsXML->FirstChildElement(); // any type of road
		while (pRoadXML != nullptr && ret)
		{
			ret = loadRoad(pRoadXML, res);
			pRoadXML = pRoadXML->NextSiblingElement(); // any type of road
		}
	}
	else
	{
		res.setErrCode(LOAD_ERR_NO_ROADS_SECTION);
		res.setErrMsg("There is no roads section.");
		ret = false;
	}

	return ret;
}



bool ModelLoader::loadRoad(XMLElement* pRoadXML, LoadResult& res)
{
	const char* type = pRoadXML->Value();

	if (strcmp(type, TAG_ROAD) == 0)
		return loadClassicalRoad(pRoadXML, res);
	else if (strcmp(type, TAG_INITIAL_ROAD) == 0)
		return loadInitialRoad(pRoadXML, res);
	else if (strcmp(type, TAG_FINAL_ROAD) == 0)
		return loadFinalRoad(pRoadXML, res);
	else if (strcmp(type, TAG_MULTILANE_ROAD) == 0)
		return loadClassicalMultilaneRoad(pRoadXML, res);
	else if (strcmp(type, TAG_INITIAL_MULTILANE_ROAD) == 0)
		return loadInitialMultilaneRoad(pRoadXML, res);
	else if (strcmp(type, TAG_FINAL_MULTILANE_ROAD) == 0)
		return loadFinalMultilaneRoad(pRoadXML, res);
	else if (strcmp(type, TAG_SLIP_ROAD) == 0)
		return loadSlipRoad(pRoadXML, res);
	else
	{
		res.setErrCode(LOAD_ERR_UNKNOWN_ROAD_TYPE);
		res.setErrLine(pRoadXML->GetLineNum());
		res.setErrMsg("Unknown road type <" + string(type) + "> at line " + 
			to_string(res.getErrLine()));
		return false;
	}
}



bool ModelLoader::loadClassicalRoad(tinyxml2::XMLElement* pRoadXML, LoadResult& res)
{
	bool ret = true;

	int id, input;
	double length;
	if (readRoadAttributes(pRoadXML, res, &id, nullptr, &input, &length))
	{
		vector<int> gates;
		if (readRoadGates(pRoadXML, res, gates))
		{
			vector<SpeedLimit> speedLimits;
			if (readSpeedLimits(pRoadXML, res, speedLimits))
			{
				int createRes = pModel->createRoad(id, gates, input, length, speedLimits);
				ret = processCreateRoadResult(createRes, res, pRoadXML->GetLineNum());

				// in case the gate is a conflict gate check if it is in the temporary list conflictGates
				if (createRes == CREATE_ROAD_NO_SUCH_GATE)
				{
					bool found = false;
					for (int i = 0; i < (int) gates.size() && !found; ++i)
					{
						if (isInConflictGates(gates[i]))
						{
							roadsFromConflictGates.push_back(RoadXML(id, gates, input, length, speedLimits,
								pRoadXML->GetLineNum(), false)); // process later
							ret = true;
							found = true;
						}
						else
							ret = false;
					}
				}
			}
			else
				ret = false;
		}
		else
			ret = false;
	}
	else
		ret = false;

	return ret;
}



bool ModelLoader::loadInitialRoad(tinyxml2::XMLElement* pRoadXML, LoadResult& res)
{
	bool ret = true;

	int id, generator, input;
	double length;
	if (readRoadAttributes(pRoadXML, res, &id, &generator, &input, &length))
	{
		vector<SpeedLimit> speedLimits;
		if (readSpeedLimits(pRoadXML, res, speedLimits))
		{
			ret = processCreateRoadResult(
				pModel->createExternalRoad(id, generator, input, length, speedLimits), res, 
				pRoadXML->GetLineNum());
		}
		else
			ret = false;
	}
	else
		ret = false;

	return ret;
}



bool ModelLoader::loadFinalRoad(tinyxml2::XMLElement* pRoadXML, LoadResult& res)
{
	bool ret = true;

	int id;
	double length;
	if (readRoadAttributes(pRoadXML, res, &id, nullptr, nullptr, &length))
	{
		vector<int> gates;
		if (readRoadGates(pRoadXML, res, gates))
		{
			vector<SpeedLimit> speedLimits;
			if (readSpeedLimits(pRoadXML, res, speedLimits))
			{
				int createRes = pModel->createFinalRoad(id, gates, length, speedLimits);
				ret = processCreateRoadResult(createRes, res, pRoadXML->GetLineNum());

				// in case the gate is a conflict gate check if it is in the temporary list conflictGates
				if (createRes == CREATE_ROAD_NO_SUCH_GATE)
				{
					bool found = false;
					for (int i = 0; i < (int) gates.size() && !found; ++i)
					{
						if (isInConflictGates(gates[i]))
						{
							roadsFromConflictGates.push_back(RoadXML(id, gates, -1, length, speedLimits,
								pRoadXML->GetLineNum(), true)); // process later
							ret = true;
							found = true;
						}
						else
							ret = false;
					}
				}
			}
			else
				ret = false;
		}
		else
			ret = false;
	}
	else
		ret = false;

	return ret;
}



bool ModelLoader::loadSlipRoad(tinyxml2::XMLElement* pRoadXML, LoadResult& res)
{
	bool ret = true;

	int id, joinedRoad;
	double joinPosition, length;
	if (readRoadAttributes(pRoadXML, res, &id, nullptr, nullptr, &length, &joinedRoad, 
		&joinPosition))
	{
		vector<int> gates;
		if (readRoadGates(pRoadXML, res, gates))
		{
			ret = processCreateRoadResult(
				pModel->createSlipRoad(id, gates, joinedRoad, joinPosition, length), res,
				pRoadXML->GetLineNum());
		}
		else
			ret = false;
	}
	else
		ret = false;

	return ret;
}



bool ModelLoader::hasConflictGate(vector<vector<int>>& gatesIds)
{
	for (vector<int> gatesIdsInner : gatesIds)
	{
		for (int id : gatesIdsInner)
		{
			if (isInConflictGates(id))
				return true;
		}
	}

	return false;
}



bool ModelLoader::loadClassicalMultilaneRoad(tinyxml2::XMLElement* pRoadXML, LoadResult& res)
{
	bool ret = true;

	double length;
	if (readRoadAttributes(pRoadXML, res, nullptr, nullptr, nullptr, &length))
	{
		vector<SpeedLimit> speedLimits;
		if (readSpeedLimits(pRoadXML, res, speedLimits))
		{
			vector<int> ids, inputs;
			vector<vector<int>> gates;
			if (readLanes(pRoadXML, res, ids, nullptr, &gates, &inputs))
			{
				int createRes = pModel->createMultiLaneRoad(ids, gates, inputs, length, speedLimits);
				ret = processCreateRoadResult(createRes, res, pRoadXML->GetLineNum());

				// in case the gate is a conflict gate check if it is in the temporary list conflictGates
				if (createRes == CREATE_ROAD_NO_SUCH_GATE)
				{
					bool isConflictGate = false;
					
					if (hasConflictGate(gates))
					{
						multilaneRoadsFromConflictGates.push_back(
							MultilaneRoadXML(gates, inputs, length, speedLimits, pRoadXML->GetLineNum(),
								ids)); // process later
						ret = true;
					}
					else
						ret = false;
				}
			}
			else
				ret = false;
		}
		else
			ret = false;
	}
	else
		ret = false;

	return ret;
}



bool ModelLoader::loadInitialMultilaneRoad(tinyxml2::XMLElement* pRoadXML, LoadResult& res)
{
	bool ret = true;

	double length;
	if (readRoadAttributes(pRoadXML, res, nullptr, nullptr, nullptr, &length))
	{
		vector<SpeedLimit> speedLimits;
		if (readSpeedLimits(pRoadXML, res, speedLimits))
		{
			vector<int> ids, generators, inputs;
			if (readLanes(pRoadXML, res, ids, &generators, nullptr, &inputs))
			{
				ret = processCreateRoadResult(
					pModel->createExternalMultiLaneRoad(ids, generators, inputs, length, 
						speedLimits), 
					res, pRoadXML->GetLineNum());
			}
			else
				ret = false;
		}
		else
			ret = false;
	}
	else
		ret = false;

	return ret;
}



bool ModelLoader::loadFinalMultilaneRoad(tinyxml2::XMLElement* pRoadXML, LoadResult& res)
{
	bool ret = true;

	double length;
	if (readRoadAttributes(pRoadXML, res, nullptr, nullptr, nullptr, &length))
	{
		vector<SpeedLimit> speedLimits;
		if (readSpeedLimits(pRoadXML, res, speedLimits))
		{
			vector<int> ids;
			vector<vector<int>> gates;
			if (readLanes(pRoadXML, res, ids, nullptr, &gates, nullptr))
			{
				int createRes = pModel->createFinalMultiLaneRoad(ids, gates, length, speedLimits);
				ret = processCreateRoadResult(createRes, res, pRoadXML->GetLineNum());

				// in case the gate is a conflict gate check if it is in the temporary list conflictGates
				if (createRes == CREATE_ROAD_NO_SUCH_GATE)
				{
					bool isConflictGate = false;

					if (hasConflictGate(gates))
					{
						multilaneRoadsFromConflictGates.push_back(
							MultilaneRoadXML(gates, length, speedLimits, pRoadXML->GetLineNum(), 
								ids)); // process later
						ret = true;
					}
					else
						ret = false;
				}
			}
			else
				ret = false;
		}
		else
			ret = false;
	}
	else
		ret = false;

	return ret;
}



bool ModelLoader::readRoadAttributes(XMLElement* pRoadXML, LoadResult& res, int* pId, 
	int* pGeneratorId, int* pJunctionInputId, double* pLength,
	int* pJoinedRoad, double* pJoinPosition)
{
	int line = pRoadXML->GetLineNum();

	// id
	if (pId != nullptr)
	{
		XMLError err = pRoadXML->QueryIntAttribute(ATTR_ROAD_ID, pId);
		processXMLError(err, line, XMLDocument::ErrorIDToName(err), res);

		if (res.fail())
			return false;
	}

	// generator
	if (pGeneratorId != nullptr)
	{
		XMLError err = pRoadXML->QueryIntAttribute(ATTR_ROAD_GENERATOR, pGeneratorId);
		processXMLError(err, line, XMLDocument::ErrorIDToName(err), res);

		if (res.fail())
			return false;
	}

	// junction input
	if (pJunctionInputId != nullptr)
	{
		XMLError err = pRoadXML->QueryIntAttribute(ATTR_ROAD_JUNCTION_INPUT, pJunctionInputId);
		processXMLError(err, line, XMLDocument::ErrorIDToName(err), res);

		if (res.fail())
			return false;
	}

	// length
	if (pLength != nullptr)
	{
		XMLError err = pRoadXML->QueryDoubleAttribute(ATTR_ROAD_LENGTH, pLength);
		processXMLError(err, line, XMLDocument::ErrorIDToName(err), res);

		if (res.fail())
			return false;
	}

	// joined road
	if (pJoinedRoad != nullptr)
	{
		XMLError err = pRoadXML->QueryIntAttribute(ATTR_ROAD_JOINED_ROAD, pJoinedRoad);
		processXMLError(err, line, XMLDocument::ErrorIDToName(err), res);

		if (res.fail())
			return false;
	}

	// join position
	if (pJoinedRoad != nullptr)
	{
		XMLError err = pRoadXML->QueryDoubleAttribute(ATTR_ROAD_JOIN_POS, pJoinPosition);
		processXMLError(err, line, XMLDocument::ErrorIDToName(err), res);

		if (res.fail())
			return false;
	}

	return true;
}



bool ModelLoader::readRoadGates(tinyxml2::XMLElement* pRoadXML, LoadResult& res, 
	vector<int>& gates)
{
	bool ret = true;

	XMLElement* pGatesXML = pRoadXML->FirstChildElement(TAG_GATES);
	if (pGatesXML != nullptr)
	{
		XMLElement* pGateXML = pGatesXML->FirstChildElement(TAG_GATE);
		bool gateExists = false;
		while (pGateXML != nullptr && ret)
		{
			ret = readRoadGate(pGateXML, res, gates);
			pGateXML = pGateXML->NextSiblingElement(TAG_GATE);
			gateExists = true;
		}

		if (!gateExists)
		{
			res.setErrCode(LOAD_ERR_ROAD_NO_GATES);
			res.setErrLine(pRoadXML->GetLineNum());
			res.setErrMsg("Road at line " + to_string(res.getErrLine()) + " doesn't have any gates.");
			ret = false;
		}
	}
	else // no gates
	{
		res.setErrCode(LOAD_ERR_ROAD_NO_GATES);
		res.setErrLine(pRoadXML->GetLineNum());
		res.setErrMsg("Road at line " + to_string(res.getErrLine()) + " doesn't have any gates.");
		ret = false;
	}

	return ret;
}



bool ModelLoader::readRoadGate(tinyxml2::XMLElement* pGateXML, LoadResult& res, 
	vector<int>& gates)
{
	int id;
	XMLError err = pGateXML->QueryIntAttribute(ATTR_GATE_ID, &id);
	processXMLError(err, pGateXML->GetLineNum(), XMLDocument::ErrorIDToName(err), res);

	if (res.success())
	{
		gates.push_back(id);
		return true;
	}
	else
	{
		res.setErrCode(LOAD_ERR_ROAD_GATE_NO_ID);
		res.setErrLine(pGateXML->GetLineNum());
		res.setErrMsg("Road gate at line " + to_string(res.getErrLine()) + 
			" doesn't have the id attribute");

		return false;
	}
}



bool ModelLoader::addClassicalRoadsFromConflictGates(LoadResult& res)
{
	bool ret = true;

	int creationRes;
	/*
		Will be performed for all roads, even if there was failure, because some roads may reference
		gates which couldn't be created at this moment, so the method will be called again but without
		these roads which were successfully added.
	*/
	auto itRoads = roadsFromConflictGates.begin();
	while (itRoads != roadsFromConflictGates.end())
	{
		if (!(*itRoads).isFinal())
		{
			creationRes = pModel->createRoad((*itRoads).getId(), (*itRoads).getGatesIds(),
				(*itRoads).getInputId(), (*itRoads).getLength(), (*itRoads).getSpeedLimits());

			if (processCreateRoadResult(creationRes, res, (*itRoads).getLine()))
				itRoads = roadsFromConflictGates.erase(itRoads);
			else
			{
				ret = false;
				++itRoads;
			}
		}
		else
			++itRoads;
	}

	return ret;
}



bool ModelLoader::addFinalRoadsFromConflictGates(LoadResult& res)
{
	bool ret = true;

	int creationRes;
	/*
		Will be performed for all roads, even if there was failure, because some roads may reference
		gates which couldn't be created at this moment, so the method will be called again but without
		these roads which were successfully added.
	*/
	auto itRoads = roadsFromConflictGates.begin();
	while (itRoads != roadsFromConflictGates.end())
	{
		if ((*itRoads).isFinal())
		{
			creationRes = pModel->createFinalRoad((*itRoads).getId(), (*itRoads).getGatesIds(),
				(*itRoads).getLength(), (*itRoads).getSpeedLimits());

			if (processCreateRoadResult(creationRes, res, (*itRoads).getLine()))
				itRoads = roadsFromConflictGates.erase(itRoads);
			else
			{
				ret = false;
				++itRoads;
			}
		}
		else
			++itRoads;
	}

	return ret;
}



bool ModelLoader::addClassicalMultilaneRoadsFromConflictGates(LoadResult& res)
{
	bool ret = true;

	int creationRes;
	/*
		Will be performed for all roads, even if there was failure, because some roads may reference
		gates which couldn't be created at this moment, so the method will be called again but without
		these roads which were successfully added.
	*/
	auto itRoads = multilaneRoadsFromConflictGates.begin();
	while (itRoads != multilaneRoadsFromConflictGates.end())
	{
		if (!(*itRoads).isFinal())
		{
			creationRes = pModel->createMultiLaneRoad((*itRoads).getLanesIds(), (*itRoads).getGatesIds(),
				(*itRoads).getInputsIds(), (*itRoads).getLength(), (*itRoads).getSpeedLimits());

			if (processCreateRoadResult(creationRes, res, (*itRoads).getLine()))
				itRoads = multilaneRoadsFromConflictGates.erase(itRoads);
			else
			{
				ret = false;
				++itRoads;
			}
		}
		else
			++itRoads;
	}

	return ret;
}



bool ModelLoader::addFinalMultilaneRoadsFromConflictGates(LoadResult& res)
{
	bool ret = true;

	int creationRes;
	/*
		Will be performed for all roads, even if there was failure, because some roads may reference
		gates which couldn't be created at this moment, so the method will be called again but without
		these roads which were successfully added.
	*/
	auto itRoads = multilaneRoadsFromConflictGates.begin();
	while (itRoads != multilaneRoadsFromConflictGates.end())
	{
		if ((*itRoads).isFinal())
		{
			creationRes = pModel->createFinalMultiLaneRoad((*itRoads).getLanesIds(), 
				(*itRoads).getGatesIds(), (*itRoads).getLength(), (*itRoads).getSpeedLimits());

			if (processCreateRoadResult(creationRes, res, (*itRoads).getLine()))
				itRoads = multilaneRoadsFromConflictGates.erase(itRoads);
			else
			{
				ret = false;
				++itRoads;
			}
		}
		else
			++itRoads;
	}

	return ret;
}



bool ModelLoader::processCreateRoadResult(int result, LoadResult& res, int line)
{
	if (result == CREATE_ROAD_SUCCESS)
		return true;
	else
		res.setErrLine(line);

	string preText = "Road at line " + to_string(line) + " ";

	switch (result)
	{
	case CREATE_ROAD_ALREADY_EXISTS:
		res.setErrCode(LOAD_ERR_ROAD_ALREADY_EXISTS);
		res.setErrMsg(preText + "has a non-unique id.");
		return false;
	case CREATE_ROAD_NOT_CONTINUOUS_SPEED_LIMITS:
		res.setErrCode(LOAD_ERR_ROAD_NOT_CONTINUOUS_SPEED_LIMITS);
		res.setErrMsg(preText + 
			"has non-continuous speed limits. Speed limits must start exactly where their " + 
			"precedessor ended or at the beginning of the road if it is the first speed limit.");
		return false;
	case CREATE_ROAD_NOT_ENOUGH_SPEED_LIMITS:
		res.setErrCode(LOAD_ERR_ROAD_WITH_NO_SPEED_LIMITS);
		res.setErrMsg(preText + "doesn't have any speed limits.");
		return false;
	case CREATE_ROAD_NOT_WHOLE_ROAD_COVERED:
		res.setErrCode(LOAD_ERR_ROAD_NOT_WHOLE_ROAD_COVERED_BY_SPEED_LIMITS);
		res.setErrMsg(preText + "isn't completely covered by speed limits.");
		return false;
	case CREATE_ROAD_NO_SUCH_GATE:
		res.setErrCode(LOAD_ERR_ROAD_NO_SUCH_GATE);
		res.setErrMsg(preText + "references a non-existing gate.");
		return false;
	case CREATE_ROAD_GATE_ALREADY_TAKEN:
		res.setErrCode(LOAD_ERR_ROAD_GATE_TAKEN);
		res.setErrMsg(preText + "is joined to a gate which is already connected to another road.");
		return false;
	case CREATE_ROAD_TOO_SHORT_ROAD:
		res.setErrCode(LOAD_ERR_ROAD_TOO_SHORT);
		res.setErrMsg(preText + "is too short.");
		return false;
	case CREATE_ROAD_NO_SUCH_INPUT:
		res.setErrCode(LOAD_ERR_ROAD_NO_SUCH_INPUT);
		res.setErrMsg(preText + "references a non-existing junction input.");
		return false;
	case CREATE_ROAD_INPUT_ALREADY_TAKEN:
		res.setErrCode(LOAD_ERR_ROAD_INPUT_TAKEN);
		res.setErrMsg(preText + "is joined to an input which is already taken.");
		return false;
	case CREATE_ROAD_NO_SUCH_GENERATOR:
		res.setErrCode(LOAD_ERR_ROAD_NO_SUCH_GENERATOR);
		res.setErrMsg(preText + "references a non-existing generator.");
		return false;
	case CREATE_ROAD_GENERATOR_ALREADY_TAKEN:
		res.setErrCode(LOAD_ERR_ROAD_GENERATOR_TAKEN);
		res.setErrMsg(preText + "references a generator which is already in use.");
		return false;
	case CREATE_MULTILANE_ROAD_VECTORS_SIZES_MISMATCH:
		res.setErrCode(LOAD_ERR_MULTILANE_ROAD_SIZES_MISMATCH);
		res.setErrMsg(preText + "has errors in lanes attributes.");
		return false;
	case CREATE_SLIP_ROAD_NO_SUCH_JOINED_ROAD:
		res.setErrCode(LOAD_ERR_SLIP_ROAD_NO_SUCH_JOINED_ROAD);
		res.setErrMsg(preText + "joins a road which doesn't exist.");
		return false;
	case CREATE_SLIP_ROAD_JOIN_POSITION_NEGATIVE:
		res.setErrCode(LOAD_ERR_SLIP_ROAD_NEGATIVE_JOIN_POSITION);
		res.setErrMsg(preText + "has a negative join position.");
		return false;
	case CREATE_SLIP_ROAD_JOIN_POSITION_TOO_BIG:
		res.setErrCode(LOAD_ERR_SLIP_ROAD_TOO_BIG_JOIN_POS);
		res.setErrMsg(preText + 
			"has a join position which is greater than the joined road's length.");
		return false;
	case CREATE_SLIP_ROAD_TOO_SHORT:
		res.setErrCode(LOAD_ERR_SLIP_ROAD_TOO_SHORT);
		res.setErrMsg(preText + "is too short.");
		return false;
	default:
		res.setErrCode(LOAD_ERR_ROAD_UNKNOWN_ERR);
		res.setErrMsg(preText + "caused an unknown error.");
		return false;
	}
}



bool ModelLoader::readSpeedLimits(XMLElement* pRoadXML, LoadResult& res, 
	vector<SpeedLimit>& speedLimits)
{
	bool ret = true;

	XMLElement* pSpeedLimitsXML = pRoadXML->FirstChildElement(TAG_SPEED_LIMITS);
	if (pSpeedLimitsXML != nullptr)
	{
		XMLElement* pSpeedLimitXML = pSpeedLimitsXML->FirstChildElement(TAG_SPEED_LIMIT);
		while (pSpeedLimitXML != nullptr && ret)
		{
			ret = readSpeedLimit(pSpeedLimitXML, res, speedLimits);
			pSpeedLimitXML = pSpeedLimitXML->NextSiblingElement(TAG_SPEED_LIMIT);
		}
	}
	else
	{
		int line = pRoadXML->GetLineNum();
		res.setErrCode(LOAD_ERR_ROAD_NO_SPEED_LIMITS_SECTION);
		res.setErrLine(line);
		res.setErrMsg("There isn't a <" + string(TAG_SPEED_LIMITS) + 
			"> section in road at line " + to_string(line));
		ret = false;
	}

	return ret;
}



bool ModelLoader::readSpeedLimit(tinyxml2::XMLElement* pSpeedLimitXML, LoadResult& res,
	std::vector<SpeedLimit>& speedLimits)
{
	// read attributes
	int line = pSpeedLimitXML->GetLineNum();
	double value, start, end;

	XMLError valueErr = pSpeedLimitXML->QueryDoubleAttribute(ATTR_SPEED_LIMIT_VALUE, &value);
	processXMLError(valueErr, line, XMLDocument::ErrorIDToName(valueErr), res);
	if (res.fail())
		return false;

	XMLError startErr = pSpeedLimitXML->QueryDoubleAttribute(ATTR_SPEED_LIMIT_START, &start);
	processXMLError(startErr, line, XMLDocument::ErrorIDToName(startErr), res);
	if (res.fail())
		return false;

	XMLError endErr = pSpeedLimitXML->QueryDoubleAttribute(ATTR_SPEED_LIMIT_END, &end);
	processXMLError(endErr, line, XMLDocument::ErrorIDToName(endErr), res);
	if (res.fail())
		return false;

	// create the speed limit
	speedLimits.push_back(SpeedLimit(start, end, value));

	return true;
}



bool ModelLoader::readLanes(XMLElement* pRoadXML, LoadResult& res, vector<int>& ids,
	vector<int>* pGenerators, vector<vector<int>>* pGates, vector<int>* pJunctionInputs)
{
	bool ret = true;

	XMLElement* pLanesXML = pRoadXML->FirstChildElement(TAG_LANES);
	if (pLanesXML != nullptr)
	{
		XMLElement* pLaneXML = pLanesXML->FirstChildElement(TAG_LANE);
		while (pLaneXML != nullptr && ret)
		{
			ret = readLane(pLaneXML, res, ids, pGenerators, pGates, pJunctionInputs);
			pLaneXML = pLaneXML->NextSiblingElement(TAG_LANE);
		}
	}
	else // no lanes section
	{
		res.setErrCode(LOAD_ERR_MULTILANE_ROAD_NO_LANES_SECTION);
		res.setErrLine(pRoadXML->GetLineNum());
		res.setErrMsg("Multilane road at line " + to_string(res.getErrLine()) + 
			" hasn't got the <" + TAG_LANES + "> section.");
		ret = false;
	}

	return ret;
}



bool ModelLoader::readLane(XMLElement* pLaneXML, LoadResult& res, vector<int>& ids,
	vector<int>* pGenerators, vector<vector<int>>* pGates, vector<int>* pJunctionInputs)
{
	int line = pLaneXML->GetLineNum();

	// id
	int id;
	XMLError idErr = pLaneXML->QueryIntAttribute(ATTR_ROAD_ID, &id);
	processXMLError(idErr, line, XMLDocument::ErrorIDToName(idErr), res);

	if (res.fail())
		return false;

	ids.push_back(id);

	// generator
	if (pGenerators != nullptr)
	{
		int generator;
		XMLError generatorErr = pLaneXML->QueryIntAttribute(ATTR_ROAD_GENERATOR, &generator);
		processXMLError(generatorErr, line, XMLDocument::ErrorIDToName(generatorErr), res);

		if (res.fail())
			return false;

		pGenerators->push_back(generator);
	}

	// gates
	if (pGates != nullptr)
	{
		vector<int> gatesIds;
		if (readRoadGates(pLaneXML, res, gatesIds))
		{
			pGates->push_back(gatesIds);
		}
		else
			return false;
	}

	// junction input
	if (pJunctionInputs != nullptr)
	{
		int input;
		XMLError inputErr = pLaneXML->QueryIntAttribute(ATTR_ROAD_JUNCTION_INPUT, &input);
		processXMLError(inputErr, line, XMLDocument::ErrorIDToName(inputErr), res);

		if (res.fail())
			return false;

		pJunctionInputs->push_back(input);
	}

	return true;
}



// cars ///////////////////////////////////////////////////////////////////////////////////////////



bool ModelLoader::loadRoutes(XMLDocument& doc, LoadResult& res)
{
	bool ret = true;

	XMLElement* pRoutes = doc.FirstChildElement(TAG_ROUTES);
	if (pRoutes != nullptr)
	{
		XMLElement* pRouteXML = pRoutes->FirstChildElement(TAG_ROUTE);
		while (pRouteXML != nullptr && ret)
		{
			ret = loadRoute(pRouteXML, res);
			pRouteXML = pRouteXML->NextSiblingElement(TAG_ROUTE);
		}
	}
	else
	{
		res.setErrCode(LOAD_ERR_NO_ROUTES_SECTION);
		res.setErrMsg("There is no <" + string(TAG_ROUTES) + "> section");
		ret = false;
	}

	return ret;
}



bool ModelLoader::loadRoute(XMLElement* pRouteXML, LoadResult& res)
{
	bool ret = true;

	int line = pRouteXML->GetLineNum();

	int id;
	XMLError err = pRouteXML->QueryAttribute(ATTR_ROUTE_ID, &id);
	processXMLError(err, line, XMLDocument::ErrorIDToName(err), res);

	if (res.success())
	{
		int roadId;
		vector<int> route;
		XMLElement* pRoadXML = pRouteXML->FirstChildElement(TAG_ROAD);
		while (pRoadXML != nullptr && ret)
		{
			XMLError idErr = pRoadXML->QueryAttribute(ATTR_ROAD_ID, &roadId);
			processXMLError(idErr, line, XMLDocument::ErrorIDToName(idErr), res);

			if (res.success())
			{
				route.push_back(roadId);
				pRoadXML = pRoadXML->NextSiblingElement(TAG_ROAD);
			}
			else
				ret = false;
		}

		if (ret)
		{
			int createRes = pModel->createRoute(id, route);
			switch (createRes)
			{
			case CREATE_ROUTE_SUCCESS:
				break;
			case CREATE_ROUTE_NON_UNIQUE_ID:
				res.setErrCode(LOAD_ERR_ROUTE_NON_UNIQUE_ID);
				res.setErrLine(line);
				res.setErrMsg("Route at line " + to_string(line) + " has a non-unique id");
				ret = false;
				break;
			case CREATE_ROUTE_WRONG_ROUTE:
				res.setErrCode(LOAD_ERR_ROUTE_WRONG_ROUTE);
				res.setErrLine(line);
				res.setErrMsg("Route at line " + to_string(line) + " is incorrect.");
				ret = false;
				break;
			default:
				res.setErrCode(LOAD_ERR_ROUTE_UNKNOWN_ERR);
				res.setErrLine(line);
				res.setErrMsg("Route at line " + to_string(line) + " caused an unknown error.");
				ret = false;
				break;
			}
		}
	}
	else
		ret = false;

	return ret;
}



bool ModelLoader::loadCars(XMLDocument& doc, LoadResult& res)
{
	bool ret = true;

	XMLElement* pCarsXML = doc.FirstChildElement(TAG_CARS);
	if (pCarsXML != nullptr)
	{
		XMLElement* pCarXML = pCarsXML->FirstChildElement(TAG_CAR);
		while (pCarXML != nullptr && ret)
		{
			ret = loadCar(pCarXML, res);
			pCarXML = pCarXML->NextSiblingElement(TAG_CAR);
		}
	}
	else
	{
		res.setErrCode(LOAD_ERR_CAR_NO_CARS_SECTION);
		res.setErrMsg("There is no <" + string(TAG_CARS) + "> section.");
		ret = false;
	}

	return ret;
}



bool ModelLoader::loadCar(XMLElement* pCarXML, LoadResult& res)
{
	bool ret = true;

	int id, routeId;
	double length, speed, avgGas, maxGas, avgBrake, strongBrake, maxBrake, minGap, reactionTime;
	
	if (readCarAttributes(pCarXML, res, &id, &routeId, &length, &speed, &avgGas, &maxGas, &avgBrake,
		&strongBrake, &maxBrake, &minGap, &reactionTime))
	{
		if (isIdValid(id))
		{
			// check if id is unique
			if (cars.find(id) == cars.end())
			{
				vector<Road*>* pRoute = pModel->getRoute(routeId);
				if (pRoute != nullptr)
				{
					Car* pCar = new Car(pRoute, length, speed, avgGas, maxGas, avgBrake, strongBrake,
						maxBrake, minGap, reactionTime);

					cars[id] = pCar;
				}
				else
				{
					res.setErrCode(LOAD_ERR_CAR_NO_SUCH_ROUTE);
					res.setErrLine(pCarXML->GetLineNum());
					res.setErrMsg("Car at line " + to_string(res.getErrLine()) +
						" references a non-existing route");
					ret = false;
				}
			}
			else // non-unique id
			{
				res.setErrCode(LOAD_ERR_CAR_NON_UNIQUE_ID);
				res.setErrLine(pCarXML->GetLineNum());
				res.setErrMsg("Car at line " + to_string(res.getErrLine()) + 
					" has a non-unique id.");
				ret = false;
			}
		}
		else // invalid id
		{
			res.setErrCode(LOAD_ERR_INVALID_ID);
			res.setErrLine(pCarXML->GetLineNum());
			res.setErrMsg("Car at line " + to_string(res.getErrLine()) + " has an invalid id.");
			ret = false;
		}
		
	}
	else
		ret = false;

	return ret;
}



bool ModelLoader::readCarAttributes(XMLElement* pCarXML, LoadResult& res, int* pId, int* pRouteId,
	double* pLength, double* pSpeed, double* pAvgGas, double* pMaxGas, double* pAvgBrake, 
	double* pStrongBrake, double* pMaxBrake, double* pMinGap, double* pReactionTime)
{
	int line = pCarXML->GetLineNum();
	XMLError err;

	// id
	err = pCarXML->QueryIntAttribute(ATTR_CAR_ID, pId);
	processXMLError(err, line, XMLDocument::ErrorIDToName(err), res);

	if (res.fail())
		return false;

	// route id
	err = pCarXML->QueryIntAttribute(ATTR_CAR_ROUTE, pRouteId);
	processXMLError(err, line, XMLDocument::ErrorIDToName(err), res);

	if (res.fail())
		return false;

	// length
	err = pCarXML->QueryDoubleAttribute(ATTR_CAR_LENGTH, pLength);
	processXMLError(err, line, XMLDocument::ErrorIDToName(err), res);
	if (err == XML_NO_ATTRIBUTE)
		*pLength = DEFAULT_LENGTH;

	if (res.fail() && err != XML_NO_ATTRIBUTE)
		return false;

	if (*pLength <= 0.0)
	{
		res.setErrCode(LOAD_ERR_CAR_TOO_SHORT);
		res.setErrLine(line);
		res.setErrMsg("Car at line " + to_string(line) + " is too short.");
		return false;
	}

	// speed
	err = pCarXML->QueryDoubleAttribute(ATTR_CAR_SPEED, pSpeed);
	processXMLError(err, line, XMLDocument::ErrorIDToName(err), res);
	if (err == XML_NO_ATTRIBUTE)
		*pSpeed = DEFAULT_SPEED;

	if (res.fail() && err != XML_NO_ATTRIBUTE)
		return false;

	if (*pSpeed <= 0.0)
	{
		res.setErrCode(LOAD_ERR_CAR_SPEED_TOO_SMALL);
		res.setErrLine(line);
		res.setErrMsg("Car at line " + to_string(line) + " has too small speed.");
		return false;
	}

	// avg gas
	err = pCarXML->QueryDoubleAttribute(ATTR_CAR_AVG_GAS, pAvgGas);
	processXMLError(err, line, XMLDocument::ErrorIDToName(err), res);
	if (err == XML_NO_ATTRIBUTE)
		*pAvgGas = DEFAULT_AVG_GAS;

	if (res.fail() && err != XML_NO_ATTRIBUTE)
		return false;

	if (*pAvgGas <= 0.0)
	{
		res.setErrCode(LOAD_ERR_CAR_AVG_GAS_TOO_SMALL);
		res.setErrLine(line);
		res.setErrMsg("Car at line " + to_string(line) + " has too small avg gas.");
		return false;
	}

	// max gas
	err = pCarXML->QueryDoubleAttribute(ATTR_CAR_MAX_GAS, pMaxGas);
	processXMLError(err, line, XMLDocument::ErrorIDToName(err), res);
	if (err == XML_NO_ATTRIBUTE)
		*pMaxGas = DEFAULT_MAX_GAS;

	if (res.fail() && err != XML_NO_ATTRIBUTE)
		return false;

	if (*pMaxGas < *pAvgGas)
	{
		res.setErrCode(LOAD_ERR_CAR_MAX_GAS_TOO_SMALL);
		res.setErrLine(line);
		res.setErrMsg("Car at line " + to_string(line) + " has too small avg gas.");
		return false;
	}

	// avg brake
	err = pCarXML->QueryDoubleAttribute(ATTR_CAR_AVG_BRAKE, pAvgBrake);
	processXMLError(err, line, XMLDocument::ErrorIDToName(err), res);
	if (err == XML_NO_ATTRIBUTE)
		*pAvgBrake = DEFAULT_AVG_BRAKE;

	if (res.fail() && err != XML_NO_ATTRIBUTE)
		return false;

	if (*pAvgBrake >= 0.0)
	{
		res.setErrCode(LOAD_ERR_CAR_AVG_BRAKE_NON_NEGATIVE);
		res.setErrLine(line);
		res.setErrMsg("Car at line " + to_string(line) + " has non-negative avg brake.");
		return false;
	}

	// strong brake
	err = pCarXML->QueryDoubleAttribute(ATTR_CAR_STRONG_BRAKE, pStrongBrake);
	processXMLError(err, line, XMLDocument::ErrorIDToName(err), res);
	if (err == XML_NO_ATTRIBUTE)
		*pStrongBrake = DEFAULT_STRONG_BRAKE;

	if (res.fail() && err != XML_NO_ATTRIBUTE)
		return false;

	if (*pAvgBrake < *pStrongBrake)
	{
		res.setErrCode(LOAD_ERR_CAR_STRONG_BRAKE_TOO_WEAK);
		res.setErrLine(line);
		res.setErrMsg("Car at line " + to_string(line) + " has too weak strong brake.");
		return false;
	}

	// max brake
	err = pCarXML->QueryDoubleAttribute(ATTR_CAR_MAX_BRAKE, pMaxBrake);
	processXMLError(err, line, XMLDocument::ErrorIDToName(err), res);
	if (err == XML_NO_ATTRIBUTE)
		*pMaxBrake = DEFAULT_MAX_BRAKE;

	if (res.fail() && err != XML_NO_ATTRIBUTE)
		return false;

	if (*pStrongBrake < *pMaxBrake)
	{
		res.setErrCode(LOAD_ERR_CAR_MAX_BRAKE_TOO_WEAK);
		res.setErrLine(line);
		res.setErrMsg("Car at line " + to_string(line) + " has too weak max brake.");
		return false;
	}

	// min gap
	err = pCarXML->QueryDoubleAttribute(ATTR_CAR_MIN_GAP, pMinGap);
	processXMLError(err, line, XMLDocument::ErrorIDToName(err), res);
	if (err == XML_NO_ATTRIBUTE)
		*pMinGap = DEFAULT_GAP;

	if (res.fail() && err != XML_NO_ATTRIBUTE)
		return false;

	if (*pMinGap < 0.0)
	{
		res.setErrCode(LOAD_ERR_CAR_NEGATIVE_MIN_GAP);
		res.setErrLine(line);
		res.setErrMsg("Car at line " + to_string(line) + " has a negative min gap.");
		return false;
	}

	// reaction time
	err = pCarXML->QueryDoubleAttribute(ATTR_CAR_REACTION_TIME, pReactionTime);
	processXMLError(err, line, XMLDocument::ErrorIDToName(err), res);
	if (err == XML_NO_ATTRIBUTE)
		*pReactionTime = DEFAULT_REACTION_TIME;

	if (res.fail() && err != XML_NO_ATTRIBUTE)
		return false;

	if (*pReactionTime < 0.0)
	{
		res.setErrCode(LOAD_ERR_CAR_NEGATIVE_REACTION_TIME);
		res.setErrLine(line);
		res.setErrMsg("Car at line " + to_string(line) + " has a negative reaction time.");
		return false;
	}

	return true;
}



bool ModelLoader::setTemplateCars(LoadResult& res)
{
	for (auto generatorTemplate : generatorsTemplateCars)
	{
		auto itCar = cars.find(generatorTemplate.second);
		if (itCar != cars.end())
		{
			if (pModel->setTemplateCarByGlobalId(generatorTemplate.first, itCar->second->clone()) !=
				SET_TEMPLATE_CAR_SUCCESS)
			{
				res.setErrCode(LOAD_ERR_GENERATOR_CANT_SET_TEMPLATE_CAR);
				res.setErrMsg("Could not set the template car with id " + 
					to_string(generatorTemplate.second) + 
					". Most likely the car's route doesn't start with the generator's route.");
				return false;
			}
		}
		else
		{
			res.setErrCode(LOAD_ERR_GENERATOR_NO_SUCH_TEMPLATE_CAR);
			res.setErrMsg("There is no template car with id " + to_string(generatorTemplate.second));
			return false;
		}
	}

	return true;
}



// processing errors //////////////////////////////////////////////////////////////////////////////



void ModelLoader::processXMLError(const XMLError& error, int errorLine, const char* errorName, 
	LoadResult& res)
{
	res.setErrMsg(errorName);

	switch (error)
	{
	case XMLError::XML_SUCCESS:
		res.setErrCode(LOAD_SUCCESS);
		break;
	case XMLError::XML_ERROR_FILE_NOT_FOUND:
		res.setErrCode(LOAD_ERR_FILE_NOT_FOUND);
		break;
	case XMLError::XML_ERROR_FILE_COULD_NOT_BE_OPENED:
		res.setErrCode(LOAD_ERR_CANT_OPEN);
		break;
	case XMLError::XML_ERROR_FILE_READ_ERROR:
		res.setErrCode(LOAD_ERR_FILE_READ);
		break;
	case XMLError::XML_CAN_NOT_CONVERT_TEXT:
		res.setErrCode(LOAD_ERR_CONVERSION);
		res.setErrLine(errorLine);
		break;
	case XMLError::XML_ELEMENT_DEPTH_EXCEEDED:
		res.setErrCode(LOAD_ERR_DEPTH_EXCEEDED);
		break;
	case XMLError::XML_ERROR_COUNT:
		res.setErrCode(LOAD_ERR_COUNT);
		break;
	case XMLError::XML_ERROR_EMPTY_DOCUMENT:
		res.setErrCode(LOAD_ERR_EMPTY);
		break;
	case XMLError::XML_ERROR_MISMATCHED_ELEMENT:
		res.setErrCode(LOAD_ERR_MISMATCHED_ELEMENT);
		res.setErrLine(errorLine);
		break;
	case XMLError::XML_ERROR_PARSING:
		res.setErrCode(LOAD_ERR_PARSING);
		res.setErrLine(errorLine);
		break;
	case XMLError::XML_ERROR_PARSING_ATTRIBUTE:
		res.setErrCode(LOAD_ERR_PARSING_ATTRIBUTE);
		res.setErrLine(errorLine);
		break;
	case XMLError::XML_ERROR_PARSING_CDATA:
		res.setErrCode(LOAD_ERR_PARSING_CDATA);
		res.setErrLine(errorLine);
		break;
	case XMLError::XML_ERROR_PARSING_COMMENT:
		res.setErrCode(LOAD_ERR_PARSING_COMMENT);
		res.setErrLine(errorLine);
		break;
	case XMLError::XML_ERROR_PARSING_DECLARATION:
		res.setErrCode(LOAD_ERR_PARSING_DECLARATION);
		res.setErrLine(errorLine);
		break;
	case XMLError::XML_ERROR_PARSING_ELEMENT:
		res.setErrCode(LOAD_ERR_PARSING_ELEMENT);
		res.setErrLine(errorLine);
		break;
	case XMLError::XML_ERROR_PARSING_TEXT:
		res.setErrCode(LOAD_ERR_PARSING_TEXT);
		res.setErrLine(errorLine);
		break;
	case XMLError::XML_ERROR_PARSING_UNKNOWN:
		res.setErrCode(LOAD_ERR_PARSING_UNKNOWN);
		res.setErrLine(errorLine);
		break;
	case XMLError::XML_NO_ATTRIBUTE:
		res.setErrCode(LOAD_ERR_XML_NO_ATTRIBUTE);
		res.setErrLine(errorLine);
		break;
	case XMLError::XML_NO_TEXT_NODE:
		res.setErrCode(LOAD_ERR_XML_NO_TEXT_NODE);
		res.setErrLine(errorLine);
		break;
	case XMLError::XML_WRONG_ATTRIBUTE_TYPE:
		res.setErrCode(LOAD_ERR_WRONG_ATTRIBUTE_TYPE);
		res.setErrLine(errorLine);
		break;
	default:
		res.setErrCode(LOAD_ERR_UNKNOWN);
		res.setErrMsg("Unknown error occurred during loading the file");
	}

	if (res.getErrLine() >= 0)
		res.setErrMsg(res.getErrMsg() + " at line " + to_string(res.getErrLine()));
}



void ModelLoader::processLoadErrorToString(XMLDocument& doc, LoadResult& res)
{
	processXMLError(doc.ErrorID(), doc.ErrorLineNum(), doc.ErrorName(), res);
}



void ModelLoader::setUnknownCodeError(int errorLine, LoadResult& res)
{
	res.setErrCode(LOAD_ERR_UNDEFINED_RESULT_CODE);
	res.setErrLine(errorLine);
	res.setErrMsg("Unknown result code was returned after processing line number "
		+ to_string(res.getErrLine()));
}



bool ModelLoader::isIdValid(int id)
{
	return id <= MAX_ALLOWED_USER_DEFINED_ID;
}



