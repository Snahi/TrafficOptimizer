#pragma once

#include <string>
#include "../xml/tinyxml2.h"
#include "Model.h"
#include "junction/gate/ConflictGate.h"
#include <utility>

// const //////////////////////////////////////////////////////////////////////////////////////////
// naming in xml
#define TAG_JUNCTIONS "junctions"
#define TAG_JUNCTION "junction"
#define ATTR_JUNCTION_ID "id"
#define TAG_ROUNDABOUT "roundabout"
#define ATTR_ROUNDABOUT_ID "id"
#define ATTR_ROUNDABOUT_LENGTH "length"
#define ATTR_ROUNDABOUT_SPEED "speed"
#define TAG_ROUNDABOUT_OUTPUTS "outputs"
#define TAG_ROUNDABOUT_OUTPUT "output"
#define ATTR_ROUNDABOUT_OUTPUT_POSITION "position"
#define TAG_LIGHTS_SYSTEMS "lights_systems"
#define TAG_LIGHTS_SYSTEM "lights_system"
#define ATTR_LIGHTS_SYSTEM_ID "id"
#define TAG_LIGHTS "lights"
#define ATTR_LIGHTS_ID "id"
#define TAG_LIGHTS_GROUP "lights_group"
#define ATTR_LIGHTS_GROUP_ID "id"
#define ATTR_LIGHTS_GROUP_DURATION "duration"
#define ATTR_LIGHTS_GROUP_MIN_DURATION "min_duration"
#define ATTR_LIGHTS_GROUP_MAX_DURATION "max_duration"
#define TAG_GREEN_LIGHTS "green"
#define TAG_RED_LIGHTS "red"
#define TAG_JUNCTION_INPUTS "inputs"
#define TAG_INPUT "input"
#define ATTR_JUNCTION_INPUT_ID "id"
#define ATTR_GATE_ID "id"
#define TAG_ALWAYS_OPEN_GATE "always_open_gate"
#define TAG_ALWAYS_CLOSED_GATE "always_closed_gate"
#define TAG_COMPOUND_GATE "compound_gate"
#define TAG_COMPONENT_GATE "component_gate"
#define ATTR_COMPONENT_GATE_ID "id"
#define TAG_CONFLICT_GATE "conflict_gate"
#define TAG_CONFLICT "conflict"
#define TAG_CONFLICT_ROAD "conflict_road"
#define ATTR_CONFLICT_ROAD_ID "id"
#define ATTR_CONFLICT_DURATION "duration"
#define ATTR_CONFLICT_POSITON "conflict_pos"
#define ATTR_CONFLICT_START "start"
#define TAG_LIGHTS_GATE "lights_gate"
#define ATTR_LIGHTS_GATE_ID "id"
#define ATTR_LIGHTS_GATE_LIGHTS "lights"
#define TAG_ROUNDABOUT_GATE "roundabout_gate"
#define ATTR_ROUNDABOUT_GATE_POSITION "position"
#define TAG_GENERATORS "generators"
#define ATTR_GENERATOR_ID "id"
#define ATTR_GENERATOR_CARS_PER_PERIOD "cars_per_period"
#define ATTR_GENERATOR_PERIOD "period"
#define TAG_ROADS "roads"
#define TAG_ROAD "road"
#define TAG_INITIAL_ROAD "initial_road"
#define TAG_FINAL_ROAD "final_road"
#define TAG_MULTILANE_ROAD "multilane_road"
#define TAG_INITIAL_MULTILANE_ROAD "initial_multilane_road"
#define TAG_FINAL_MULTILANE_ROAD "final_multilane_road"
#define ATTR_ROAD_ID "id"
#define ATTR_ROAD_GENERATOR "generator"
#define ATTR_GENERATOR_TEMPLATE_CAR "template_car"
#define TAG_GATES "gates"
#define TAG_GATE "gate"
#define ATTR_ROAD_JUNCTION_INPUT "junction_input"
#define ATTR_ROAD_LENGTH "length"
#define TAG_SPEED_LIMITS "speed_limits"
#define TAG_SPEED_LIMIT "speed_limit"
#define ATTR_SPEED_LIMIT_VALUE "value"
#define ATTR_SPEED_LIMIT_START "start"
#define ATTR_SPEED_LIMIT_END "end"
#define TAG_LANES "lanes"
#define TAG_LANE "lane"
#define TAG_SLIP_ROAD "slip_road"
#define ATTR_ROAD_JOINED_ROAD "joined_road"
#define ATTR_ROAD_JOIN_POS "join_position"
#define TAG_ROUTES "routes"
#define TAG_ROUTE "route"
#define ATTR_ROUTE_ID "id"
#define TAG_CARS "cars"
#define TAG_CAR "car"
#define ATTR_CAR_ID "id"
#define ATTR_CAR_ROUTE "route"
#define ATTR_CAR_LENGTH "length"
#define ATTR_CAR_SPEED "speed"
#define ATTR_CAR_AVG_GAS "avg_gas"
#define ATTR_CAR_MAX_GAS "max_gas"
#define ATTR_CAR_AVG_BRAKE "avg_brake"
#define ATTR_CAR_STRONG_BRAKE "strong_brake"
#define ATTR_CAR_MAX_BRAKE "max_brake"
#define ATTR_CAR_MIN_GAP "min_gap"
#define ATTR_CAR_REACTION_TIME "reaction_time"
// load
#define LOAD_SUCCESS 0
#define LOAD_ERR_FILE_NOT_FOUND 1
#define LOAD_ERR_CANT_OPEN 2
#define LOAD_ERR_FILE_READ 3
#define LOAD_ERR_CONVERSION 4
#define LOAD_ERR_DEPTH_EXCEEDED 5
#define LOAD_ERR_COUNT 6
#define LOAD_ERR_EMPTY 7
#define LOAD_ERR_MISMATCHED_ELEMENT 8
#define LOAD_ERR_PARSING 9
#define LOAD_ERR_PARSING_ATTRIBUTE 10
#define LOAD_ERR_PARSING_CDATA 11
#define LOAD_ERR_PARSING_COMMENT 12
#define LOAD_ERR_PARSING_DECLARATION 13
#define LOAD_ERR_PARSING_ELEMENT 14
#define LOAD_ERR_PARSING_TEXT 15
#define LOAD_ERR_PARSING_UNKNOWN 16
#define LOAD_ERR_XML_NO_ATTRIBUTE 17
#define LOAD_ERR_XML_NO_TEXT_NODE 18
#define LOAD_ERR_WRONG_ATTRIBUTE_TYPE 19
#define LOAD_ERR_UNKNOWN 20
#define LOAD_ERR_NO_JUNCTIONS 2001
#define LOAD_ERR_JUNCTION_ALREADY_EXISTS 2002
#define LOAD_ERR_NO_JUNCTION_INPUTS_SECTION 2003
#define LOAD_ERR_JUNCTION_INPUT_ALREADY_EXISTS 2004
#define LOAD_ERR_JUNCTION_INPUT_NO_SUCH_JUNCTION 2005
#define LOAD_ERR_LIGHTS_SYSTEM_NO_SUCH_JUNCTION 2007
#define LOAD_ERR_LIGHTS_SYSTEM_ALREADY_EXISTS 2008
#define LOAD_ERR_LIGHTS_ALREADY_EXIST 2009
#define LOAD_ERR_LIGHTS_NO_SUCH_LIGHTS_SYSTEM 2010
#define LOAD_ERR_LIGHTS_GROUP_NON_POSITIVE_DURATION 2011
#define LOAD_ERR_LIGHTS_GROUP_NEGATIVE_NON_POSITIVE_MIN_DURATION 2012
#define LOAD_ERR_LIGHTS_GROUP_NEGATIVE_NON_POSITIVE_MAX_DURATION 2013
#define LOAD_ERR_MIN_DURATION_GREATER_THAN_MAX_DURATION 2014
#define LOAD_ERR_TOO_SMALL_DURATION 2015
#define LOAD_ERR_TOO_BIG_DURATION 2016
#define LOAD_ERR_LIGHTS_GROUP_ALREADY_EXISTS 2017
#define LOAD_ERR_LIGHTS_GROUP_DOES_NOT_COVER_ALL_LIGHTS 2018
#define LOAD_ERR_LIGHTS_GROUP_UNKNOWN_ERROR 2019
#define LOAD_ERR_LIGHTS_GROUP_LIGHTS_IN_GREEN_AND_RED_AT_THE_SAME_TIME 2020
#define LOAD_ERR_LIGHTS_GROUP_LIGHTS_FROM_DIFFERENT_SYSTEM 2021
#define LOAD_ERR_UNKNOWN_GATE 2022
#define LOAD_ERR_GATE_NO_SUCH_INPUT 2023
#define LOAD_ERR_GATE_ALREADY_EXISTS 2024
#define LOAD_ERR_GATE_UNKNOWN_ERROR 2025
#define LOAD_ERR_GATE_NO_SUCH_GATE 2026
#define LOAD_ERR_GATE_NO_COMPONENTS 2027
#define LOAD_ERR_GATE_COMPONENT_IN_USE 2028
#define LOAD_ERR_CONFLICT_ROAD_MULTIPLE_OCCURRENCES 2029
#define LOAD_ERR_NO_CONFLICTS 2030
#define LOAD_ERR_GATE_NO_SUCH_LIGHTS 2031
#define LOAD_ERR_GATE_NO_ID 2032
#define LOAD_ERR_ROUNDABOUT_TOO_SHORT 2033
#define LOAD_ERR_ROUNDABOUT_INCORRECT_SPEED 2034
#define LOAD_ERR_ROUNDABOUT_UNKNOWN_ERROR 2035
#define LOAD_ERR_ADD_ROUNDABOUT_CANT_ADD_OUTPUT 2036
#define LOAD_ERR_ADD_ROUNDABOUT_OUTPUT_WRONG_NUMBER_OF_GATES 2037
#define LOAD_ERR_ADD_ROUNDABOUT_OUTPUT_INPUT_TAKEN 2038
#define LOAD_ERR_ADD_ROUNDABOUT_GATE_NO_SUCH_INPUT 2039
#define LOAD_ERR_ADD_ROUNDABOUT_OUTPUT_NO_SUCH_ROUNDABOUT 2040
#define LOAD_ERR_ADD_ROUNDABOUT_OUTPUT_UNKNOWN_ERROR 2041
#define LOAD_ERR_INVALID_ID 2042
#define LOAD_ERR_NO_ROUNDABOUT_GATE 2043
#define LOAD_ERR_ROUNDABOUT_GATE_NO_SUCH_ROUNDABOUT 2044
#define LOAD_ERR_ROUNDABOUT_GATE_CANT_CONNECT 2045
#define LOAD_ERR_NO_GENERATORS_SECTION 2046
#define LOAD_ERR_UNKNOWN_GENERATOR 2047
#define LOAD_ERR_GENERATOR_NEGATIVE_CARS_PER_PERIOD 2048
#define LOAD_ERR_GENERATOR_TOO_SMALL_PERIOD 2049
#define LOAD_ERR_GENERATOR_PARSING_ERROR 2050
#define LOAD_ERR_GENERATOR_ALREADY_EXISTS 2051
#define LOAD_ERR_GENERATOR_UNKNOWN_ERROR 2052
#define LOAD_ERR_ATTACH_GENERATOR_NO_SUCH_GENERATOR 2053
#define LOAD_ERR_ATTACH_GENERATOR_UNKNOWN_ERROR 2054
#define LOAD_ERR_NO_ROADS_SECTION 2055
#define LOAD_ERR_UNKNOWN_ROAD_TYPE 2056
#define LOAD_ERR_ROAD_ALREADY_EXISTS 2057
#define LOAD_ERR_ROAD_NOT_CONTINUOUS_SPEED_LIMITS 2058
#define LOAD_ERR_ROAD_WITH_NO_SPEED_LIMITS 2059
#define LOAD_ERR_ROAD_NOT_WHOLE_ROAD_COVERED_BY_SPEED_LIMITS 2060
#define LOAD_ERR_ROAD_NO_SUCH_GATE 2061
#define LOAD_ERR_ROAD_GATE_TAKEN 2062
#define LOAD_ERR_ROAD_TOO_SHORT 2063
#define LOAD_ERR_ROAD_NO_SUCH_INPUT 2064
#define LOAD_ERR_ROAD_INPUT_TAKEN 2065
#define LOAD_ERR_ROAD_UNKNOWN_ERR 2066
#define LOAD_ERR_ROAD_NO_SUCH_GENERATOR 2067
#define LOAD_ERR_ROAD_GENERATOR_TAKEN 2068
#define LOAD_ERR_MULTILANE_ROAD_SIZES_MISMATCH 2069
#define LOAD_ERR_ROAD_NO_SPEED_LIMITS_SECTION 2070
#define LOAD_ERR_MULTILANE_ROAD_NO_LANES_SECTION 2071
#define LOAD_ERR_SLIP_ROAD_NO_SUCH_JOINED_ROAD 2072
#define LOAD_ERR_SLIP_ROAD_NEGATIVE_JOIN_POSITION 2073
#define LOAD_ERR_SLIP_ROAD_TOO_BIG_JOIN_POS 2074
#define LOAD_ERR_SLIP_ROAD_TOO_SHORT 2075
#define LOAD_ERR_CONFLICT_GATE_NEGATIVE_CONFLICT_POSITION 2076
#define LOAD_ERR_CONFLICT_GATE_NEGATIVE_DURATION 2077
#define LOAD_ERR_CONFLICT_GATE_NEGATIVE_START 2078
#define LOAD_ERR_CONFLICT_GATE_CONFLICT_ROAD_NOT_FOUND 2079
#define LOAD_ERR_NO_ROUTES_SECTION 2080
#define LOAD_ERR_ROUTE_NON_UNIQUE_ID 2081
#define LOAD_ERR_ROUTE_WRONG_ROUTE 2082
#define LOAD_ERR_ROUTE_UNKNOWN_ERR 2083
#define LOAD_ERR_CAR_NO_CARS_SECTION 2084
#define LOAD_ERR_CAR_NO_SUCH_ROUTE 2085
#define LOAD_ERR_CAR_NON_UNIQUE_ID 2086
#define LOAD_ERR_GENERATOR_NO_TEMPLATE_CAR 2087
#define LOAD_ERR_GENERATOR_NO_SUCH_TEMPLATE_CAR 2088
#define LOAD_ERR_GENERATOR_CANT_SET_TEMPLATE_CAR 2089
#define LOAD_ERR_CAR_TOO_SHORT 2090
#define LOAD_ERR_CAR_SPEED_TOO_SMALL 2091
#define LOAD_ERR_CAR_AVG_GAS_TOO_SMALL 2092
#define LOAD_ERR_CAR_MAX_GAS_TOO_SMALL 2093
#define LOAD_ERR_CAR_AVG_BRAKE_NON_NEGATIVE 2094
#define LOAD_ERR_CAR_STRONG_BRAKE_TOO_WEAK 2095
#define LOAD_ERR_CAR_MAX_BRAKE_TOO_WEAK 2096
#define LOAD_ERR_CAR_NEGATIVE_MIN_GAP 2097
#define LOAD_ERR_CAR_NEGATIVE_REACTION_TIME 2098
#define LOAD_ERR_ROAD_NO_GATES 2099
#define LOAD_ERR_ROAD_GATE_NO_ID 2100
#define LOAD_ERR_UNKNOWN_JUNCTION_TYPE 2101
#define LOAD_ERR_ADD_COMPONENT_GATE_ALREADY_IN_USE 2102
#define LOAD_ERR_ADD_COMPONENT_GATE_NO_SUCH_COMPOUND_GATE 2103
#define LOAD_ERR_ADD_COMPONENT_GATE_NOT_COMPOUND_GATE 2104
#define LOAD_ERR_ADD_COMPONENT_GATE_NO_SUCH_COMPONENT 2105
#define LOAD_ERR_ADD_COMPONENT_GATE_UNKNOWN_ERROR 2106
#define LOAD_ERR_UNDEFINED_RESULT_CODE 7000

class LoadResult;
class ConflictGateXML;
class ConflictXML;
class RoadXML;
class MultilaneRoadXML;



class ModelLoader
{
public:
	// constructors ///////////////////////////////////////////////////////////////////////////////
	ModelLoader(std::string& path);
	~ModelLoader();
	// methods ////////////////////////////////////////////////////////////////////////////////////
	LoadResult load();
private:
	// fields /////////////////////////////////////////////////////////////////////////////////////
	std::string path;
	Model* pModel;
	/*
		Conflict gate requires knowledge about roads and roads are created after gates, so after
		loading conflict gates will be stored here and once all relevant roads are loaded the gates
		will be created.
	*/
	std::vector<ConflictGateXML> conflictGates;
	/*
		Roads which start in conflict gates. Must be created after all other roads are created.
	*/
	std::vector<RoadXML> roadsFromConflictGates;
	std::vector<MultilaneRoadXML> multilaneRoadsFromConflictGates;
	/*
		If a compound gate contained a conflict gate then if it was added normally to the model
		then the no such gate error would be returned, because conflict gates are added at the
		end. To prevent that if a compound gate contains a conflict gate, all the conflict gates
		are initially removed from the compound gate and the compund gate is added to a model
		without them. But these conflict gates are not forgotten, they are stored here and after
		all conflit gates are finally created the conflict component gates are added to the model.
	*/
	std::unordered_map<int, std::vector<int>> compoundGatesWithConflicts;
	std::unordered_map<int, Car*> cars;
	/*
		After successful generator creation its id is added here so that later it can have its 
		template car set. <generator id, template car id>
	*/
	std::unordered_map<int, int> generatorsTemplateCars;

	// methods ////////////////////////////////////////////////////////////////////////////////////
	void processLoadErrorToString(tinyxml2::XMLDocument& doc, LoadResult& res);
	void processXMLError(const tinyxml2::XMLError& error, int errorLine, const char* errorName, 
		LoadResult& res);
	void setUnknownCodeError(int errorLine, LoadResult& res);
	bool isIdValid(int id);
	/*
		Reads all junctions from the model file and if success then updates the model. Otherwise
		sets an error code in res.
		
		Returns:
			true	-> success
			false	-> fail, error will be set in res
	*/
	bool loadJunctions(tinyxml2::XMLDocument& doc, LoadResult& res);
	bool loadJunction(int id, tinyxml2::XMLElement* pJunctionXML, LoadResult& res);
	/*
		Loads a junction <junction> which is not a roundabout
	*/
	bool loadClassicalJunction(tinyxml2::XMLElement* pJunctionXML, LoadResult& res);
	bool loadRoundabout(tinyxml2::XMLElement* pRoundaboutXML, LoadResult& res);
	bool readRoundaboutAttributes(tinyxml2::XMLElement* pRoundaboutXML, LoadResult& res, int* pId, 
		double* pLength, double* pSpeed);
	bool processCreateRoundaboutResult(int result, LoadResult& res, int line);
	bool loadRoundaboutOutputs(tinyxml2::XMLElement* pRoundaboutXML, LoadResult& res, 
		int roundaboutId);
	bool loadRoundaboutOutput(tinyxml2::XMLElement* pOutputXML, LoadResult& res, int roundaboutId);
	bool createRoundaboutOutputInternalInput(int junctionInputId, int roundaboutId, 
		tinyxml2::XMLElement* pOutputXML, LoadResult& res);
	bool processAddRoundaboutOutputResult(int result, LoadResult& res, int line);
	bool loadRoundaboutGate(tinyxml2::XMLElement* pRoundaboutXML, LoadResult& res, int roundaboutId);

	// lights systems

	/*
		Loads lights systems <lights_systems> and their components: lights and lights groups
	*/
	bool loadLightsSystems(tinyxml2::XMLElement* pLightsSystemsXML, LoadResult& res, 
		int junctionId);
	/*
		Loads a lights system <lights_system> and its components: lights and lights groups
	*/
	bool loadLightsSystem(tinyxml2::XMLElement* pLightsSystemXML, LoadResult& res, int junctionId);
	/*
		Loads lights <lighst>
	*/
	bool loadLights(tinyxml2::XMLElement* pLightsXML, LoadResult& res, int lightsSystemId);

	// lights groups

	/*
		Loads a lights group <lights_group>
	*/
	bool loadLightsGroup(tinyxml2::XMLElement* pLightsGroupXML, LoadResult& res, int lightsSystemId);
	int readLightsGroupId(tinyxml2::XMLElement* pLightsGroupXML, LoadResult& res, int line);
	double readLightsGroupDuration(tinyxml2::XMLElement* pLightsGroupXML, LoadResult& res, int line,
		double defaultDuration);
	double readLightsGroupMinDuration(tinyxml2::XMLElement* pLightsGroupXML, LoadResult& res, int line);
	double readLightsGroupMaxDuration(tinyxml2::XMLElement* pLightsGroupXML, LoadResult& res, int line);
	bool areLightsGroupDurationsValid(double duration, double minDuration, double maxDuration, 
		LoadResult& res, int line);
	void readLightsGroupGreenLights(tinyxml2::XMLElement* pLightsGroupXML, LoadResult& res, int line,
		std::vector<int>& lightsIds);
	void readLightsGroupRedLights(tinyxml2::XMLElement* pLightsGroupXML, LoadResult& res, int line,
		std::vector<int>& lightsIds);
	bool createLightsGroup(int lightsGroupId, int lightsSystemId, std::vector<int>& greenLightsIds,
		std::vector<int>& redLightsIds, double duration, double minDuration, double maxDuration,
		LoadResult& res, int line);

	// junction inputs
	bool loadJunctionInputs(tinyxml2::XMLElement* pInputsXML, LoadResult& res, int junctionId);
	bool loadJunctionInput(tinyxml2::XMLElement* pInputXML, LoadResult& res, int junctionId);

	// gates

	bool loadGates(tinyxml2::XMLElement* pInputXML, LoadResult& res, int inputId);
	bool loadGate(tinyxml2::XMLElement* pGateXML, LoadResult& res, int inputId);
	bool processGateError(int err, LoadResult& res, int line);
	bool createAlwaysOpenGate(int id, LoadResult& res, int inputId, int line);
	bool createAlwaysClosedGate(int id, LoadResult& res, int inputId, int line);
	bool createCompoundGate(int id, LoadResult& res, int inputId, 
		tinyxml2::XMLElement* pCompoundGateXML);
	bool createConflictGate(int id, LoadResult& res, int inputId, 
		tinyxml2::XMLElement* pConflictGateXML);
	/*
		Called after roads are created. Checks whether the road ids are correct and if so then
		the previously loaded conflict gates are added to the model.
	*/
	bool validateRoadsInConflictGates(LoadResult& res);
	bool loadConflictGatesWithRoads(LoadResult& res);
	bool isInConflictGates(int id);
	bool loadConflict(tinyxml2::XMLElement* pConflictXML, std::vector<int>& roadsIds, double* pStart, 
		double* pDuration, double* pPosition, LoadResult& res);
	bool readConflictRoads(tinyxml2::XMLElement* pConflictXML, std::vector<int>& conflictRoadsIds,
		LoadResult& res);
	bool readConflictAttributes(tinyxml2::XMLElement* pConflictXML, double* pStart, 
		double* pConflictPos, double* pDuration, bool multipleConflictRoads, LoadResult& res);
	bool createLightsGate(int id, LoadResult& res, int inputId,	
		tinyxml2::XMLElement* pLightsGateXML);
	bool addConflictGatesToCompoundGates(LoadResult& res);

	// generators

	bool loadGenerators(tinyxml2::XMLDocument& doc, LoadResult& res);
	bool loadGenerator(tinyxml2::XMLElement* pGeneratorXML, LoadResult& res);
	bool loadGeneratorByType(tinyxml2::XMLElement* pGeneratorXML, LoadResult& res, 
		int generatorId, int templateCarId);
	bool loadUniformGenerator(tinyxml2::XMLElement* pUniformGeneratorXML, LoadResult& res,
		int generatorId, int templateCarId);
	bool loadInnerUniformGenerator(tinyxml2::XMLElement* pUniformGeneratorXML, LoadResult& res,
		int generatorId, int templateCarId);
	bool readUniformGeneratorAttributes(tinyxml2::XMLElement* pUniformGeneratorXML, 
		LoadResult& res, int* pCarsPerPeriod, double* pPeriod);
	bool createGenerator(int id, string& type, std::vector<string>& params, LoadResult& res, 
		int line, int templateCarId);
	bool attachGenerator(int id, string& type, std::vector<string>& params, LoadResult& res,
		int line, int templateCarId);
	bool loadInnerGenerators(tinyxml2::XMLElement* pGeneratorXML, LoadResult& res, 
		int generatorId);
	bool loadInnerGenerator(tinyxml2::XMLElement* pInnerGeneratorXML, LoadResult& res,
		int generatorId);

	// roads

	bool loadRoads(tinyxml2::XMLDocument& doc, LoadResult& res);
	bool loadRoad(tinyxml2::XMLElement* pRoadXML, LoadResult& res);
	/*
		Pass nullptr to the attributes which you don't want
	*/
	bool readRoadAttributes(tinyxml2::XMLElement* pRoadXML, LoadResult& res, int* pId, 
		int* pGeneratorId, int* pJunctionInputId, double* pLength, 
		int* pJoinedRoad = nullptr, double* pJoinPosition = nullptr);
	bool readRoadGates(tinyxml2::XMLElement* pRoadXML, LoadResult& res, std::vector<int>& gates);
	bool readRoadGate(tinyxml2::XMLElement* pGateXML, LoadResult& res, std::vector<int>& gates);
	bool readSpeedLimits(tinyxml2::XMLElement* pRoadXML, LoadResult& res, 
		std::vector<SpeedLimit>& speedLimits);
	bool readSpeedLimit(tinyxml2::XMLElement* pSpeedLimitXML, LoadResult& res, 
		std::vector<SpeedLimit>& speedLimits);
	bool loadClassicalRoad(tinyxml2::XMLElement* pRoadXML, LoadResult& res);
	bool loadInitialRoad(tinyxml2::XMLElement* pRoadXML, LoadResult& res);
	bool loadFinalRoad(tinyxml2::XMLElement* pRoadXML, LoadResult& res);
	bool loadClassicalMultilaneRoad(tinyxml2::XMLElement* pRoadXML, LoadResult& res);
	bool loadInitialMultilaneRoad(tinyxml2::XMLElement* pRoadXML, LoadResult& res);
	bool loadFinalMultilaneRoad(tinyxml2::XMLElement* pRoadXML, LoadResult& res);
	bool loadSlipRoad(tinyxml2::XMLElement* pRoadXML, LoadResult& res);
	/*
		Pass nullptr to the attributes which you don't want
	*/
	bool readLanes(tinyxml2::XMLElement* pRoadXML, LoadResult& res, std::vector<int>& ids,
		std::vector<int>* pGenerators, std::vector<std::vector<int>>* pGates, std::vector<int>* pJunctionInputs);
	bool readLane(tinyxml2::XMLElement* pLaneXML, LoadResult& res, std::vector<int>& ids,
		std::vector<int>* pGenerators, std::vector<std::vector<int>>* pGates, std::vector<int>* pJunctionInputs);
	bool processCreateRoadResult(int result, LoadResult& res, int line);
	bool addClassicalRoadsFromConflictGates(LoadResult& res);
	bool addFinalRoadsFromConflictGates(LoadResult& res);
	bool addClassicalMultilaneRoadsFromConflictGates(LoadResult& res);
	bool addFinalMultilaneRoadsFromConflictGates(LoadResult& res);
	bool hasConflictGate(std::vector<std::vector<int>>& gatesIds);

	// cars

	bool loadRoutes(tinyxml2::XMLDocument& doc, LoadResult& res);
	bool loadRoute(tinyxml2::XMLElement* pRouteXML, LoadResult& res);
	bool loadCars(tinyxml2::XMLDocument& doc, LoadResult& res);
	bool loadCar(tinyxml2::XMLElement* pCarXML, LoadResult& res);
	bool readCarAttributes(tinyxml2::XMLElement* pCarXML, LoadResult& res, int* pId, int* pRouteId,
		double* pLength, double* pSpeed, double* pAvgGas, double* pMaxGas, double* pAvgBrake, 
		double* pStrongBrake, double* pMaxBrake, double* pMinGap, double* pReactionTime);
	bool setTemplateCars(LoadResult& res);
};



class LoadResult
{
public:
	// constructors
	LoadResult() : pModel{ nullptr }, errCode{ LOAD_SUCCESS }, errMsg{ "" }, errLine{ -1 } {};
	// methods
	bool success() { return errCode == LOAD_SUCCESS; }
	bool fail() { return errCode != LOAD_SUCCESS; }
	// getters & setters
	void setModel(Model* pModel) { this->pModel = pModel; }
	Model* getModel() { return pModel; }
	void setErrCode(int errCode) { this->errCode = errCode; }
	int getErrCode() { return errCode; }
	void setErrMsg(std::string errMsg) { this->errMsg = errMsg; }
	std::string getErrMsg() { return errMsg; }
	void setErrLine(int line) { this->errLine = line; }
	int getErrLine() { return errLine; }
	void setWarnings(std::vector<ModelWarning>& warnings) { this->warnings = warnings; }
	std::vector<ModelWarning>& getWarnings() { return warnings; }
private:
	Model* pModel;
	int errCode;
	std::string errMsg;
	int errLine;
	std::vector<ModelWarning> warnings;
};



/*
	Adds the number of the line in which the conflict gate was defined and conflicts with just ids
	of roads, not the roads themselves. Also, junctionId was added. It is used in case some	error 
	occurred during creation of the conflict gate. It is different than other gates because	
	conflict gate is finally created only after relevant roads are created and at this moment the 
	XMLElement is not accessible anymore.
*/
class ConflictGateXML
{
public:
	// constructors
	ConflictGateXML(int id, int line, int inputId) : id{ id }, line{ line },
		inputId{ inputId } {};
	// methods
	void addConflict(std::vector<int> roadsIds, double start, double duration, double position)
	{ 
		this->roadsIds.push_back(roadsIds);
		conflictsStarts.push_back(start);
		conflictsDurations.push_back(duration);
		conflictsPositions.push_back(position);
	}
	int getLine() { return line; }
	int getId() { return id; }
	int getInputId() { return inputId; }
	std::vector<std::vector<int>>& getConflictRoadsIds() { return roadsIds; }
	std::vector<double>& getConflictsStarts() { return conflictsStarts; }
	std::vector<double>& getConflictsDurations() { return conflictsDurations; }
	std::vector<double>& getConflictsPositions() { return conflictsPositions; }
private:
	int id;
	int line;
	int inputId;
	std::vector<std::vector<int>> roadsIds;
	std::vector<double> conflictsStarts;
	std::vector<double> conflictsDurations;
	std::vector<double> conflictsPositions;
};



class RoadXML
{
public:
	// constructors
	RoadXML(int id, std::vector<int> gatesIds, int inputId, double length, std::vector<SpeedLimit> speedLimits, 
		int line, bool isFinal) 
		: id{ id }, gatesIds{ gatesIds }, inputId{ inputId }, length{ length }, 
		speedLimits{ speedLimits }, line{ line }, isFinalRoad{ isFinal } {}
	// methods
	int getId() { return id; }
	vector<int>& getGatesIds() { return gatesIds; }
	int getInputId() { return inputId; }
	double getLength() { return length; }
	std::vector<SpeedLimit>& getSpeedLimits() { return speedLimits; }
	int getLine() { return line; }
	bool isFinal() { return isFinalRoad; }
private:
	int id;
	std::vector<int> gatesIds;
	int inputId;
	double length;
	std::vector<SpeedLimit> speedLimits;
	int line;
	bool isFinalRoad;
};



class MultilaneRoadXML
{
public:
	// constructors
	MultilaneRoadXML(std::vector<std::vector<int>> gatesIds, std::vector<int> inputsIds, double length, 
		std::vector<SpeedLimit> speedLimits, int line, std::vector<int> lanesIds)
		: gatesIds{ gatesIds }, inputsIds{ inputsIds }, length{ length },
		speedLimits{ speedLimits }, line{ line }, isFinalRoad{ false }, lanesIds{ lanesIds } {}
	MultilaneRoadXML(std::vector<std::vector<int>> gatesIds, double length,
		std::vector<SpeedLimit> speedLimits, int line, std::vector<int> lanesIds)
		: gatesIds{ gatesIds }, inputsIds{ inputsIds }, length{ length },
		speedLimits{ speedLimits }, line{ line }, isFinalRoad{ true }, lanesIds{ lanesIds } {}
	// methods
	std::vector<int>& getLanesIds() { return lanesIds; }
	std::vector<std::vector<int>>& getGatesIds() { return gatesIds; }
	std::vector<int>& getInputsIds() { return inputsIds; }
	double getLength() { return length; }
	std::vector<SpeedLimit>& getSpeedLimits() { return speedLimits; }
	int getLine() { return line; }
	bool isFinal() { return isFinalRoad; }
private:
	std::vector<std::vector<int>> gatesIds;
	std::vector<int> inputsIds;
	double length;
	std::vector<SpeedLimit> speedLimits;
	std::vector<int> lanesIds;
	int line;
	bool isFinalRoad;
};
