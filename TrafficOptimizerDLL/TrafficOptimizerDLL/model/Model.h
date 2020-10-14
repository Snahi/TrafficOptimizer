#pragma once
#include "../optimizer/ModelInterface.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <string>
#include "road/Road.h"
#include "road/SpeedLimit.h"
#include "junction/Junction.h"
#include "junction/JunctionInput.h"
#include "junction/lights/LightsSystem.h"
#include "junction/lights/Lights.h"
#include "junction/gate/Gate.h"
#include "generator/Generator.h"
#include "Car.h"
#include "generator/Generator.h"
#include "junction/gate/FinalGate.h"
#include "junction/FinalJunctionInput.h"
#include "junction/gate/SlipRoadGate.h"
#include "junction/Roundabout.h"
#include "junction/gate/RoundaboutGate.h"


// const //////////////////////////////////////////////////////////////////////////////////////////
#define MAX_NAME_LEN 200
#define MIN_ROAD_LEN 5.0
#define MAX_TICK_DURATION 0.2	// in seconds, tick can't be longer than that
#define DEFALUT_MIN_CARS_IN_GOAL_PER_TYPE 10
#define DEFAULT_MAX_TOTAL_DURATION 3600
#define MAX_ALLOWED_USER_DEFINED_ID 1000000000
// create junction
#define CREATE_JUNCTION_SUCCESS 0
#define CREATE_JUNCTION_ALREADY_EXISTS 1
// create roundabout
#define CREATE_ROUNDABOUT_SUCCESS 0
#define CREATE_ROUNDABOUT_ALREADY_EXISTS 1
#define CREATE_ROUNDABOUT_TOO_SHORT 2
#define CREATE_ROUNDABOUT_INCORRECT_SPEED 3
// add roundabout output
#define ADD_ROUNDABOUT_OUTPUT_SUCCESS 0
#define ADD_ROUNDABOUT_OUTPUT_NO_SUCH_ROUNDABOUT 1
#define ADD_ROUNDABOUT_OUTPUT_NO_SUCH_JUNCTION_INPUT 2
#define ADD_ROUNDABOUT_OUTPUT_JUNCTION_INPUT_TAKEN 3
#define ADD_ROUNDABOUT_OUTPUT_JUNCTION_HAS_WRONG_NUMBER_OF_GATES 4
#define ADD_ROUNDABOUT_OUTPUT_CANT_ADD_TO_ROUNDABOUT 5
// add junction input
#define CREATE_JUNCTION_INPUT_SUCCESS 0
#define CREATE_JUNCTION_INPUT_JUNCTION_NOT_FOUND 1
#define CREATE_JUNCTION_INPUT_ALREADY_EXISTS 2
// create gate (all types of gates can use these codes)
#define CREATE_GATE_SUCCESS 0
#define CREATE_GATE_ALREADY_EXISTS 2
#define CREATE_GATE_NO_SUCH_INPUT 4
#define CREATE_GATE_NO_SUCH_GATE 8
#define CREATE_GATE_NO_COMPONENTS 9
#define CREATE_GATE_COMPONENT_ALREADY_IN_USE 10
#define CREATE_GATE_CONFLICT_ROAD_NOT_FOUND 11
#define CREATE_GATE_NO_CONFLICT_ROADS 12
#define CREATE_GATE_NEGATIVE_CONFLICT_POSITION 13
#define CREATE_GATE_NEGATIVE_REQUIRED_DURATION 14
#define CREATE_GATE_NEGATIVE_CONFLICT_START 15
#define CREATE_GATE_CONFLICT_VECTORS_SIZES_DIFFER 16
#define CREATE_GATE_NO_SUCH_LIGHTS 17
// create roundabout gate
#define CREATE_ROUNDABOUT_GATE_NO_SUCH_ROUNDABOUT 2931
#define CREATE_ROUNDABOUT_GATE_CANT_CONNECT 2932
// create road
#define CREATE_ROAD_SUCCESS 0
#define CREATE_ROAD_ALREADY_EXISTS 2
#define CREATE_ROAD_NO_SUCH_GATE 3
#define CREATE_ROAD_TOO_SHORT_ROAD 4
#define CREATE_ROAD_NO_SUCH_INPUT 5
#define CREATE_ROAD_GATE_ALREADY_TAKEN 6
#define CREATE_ROAD_INPUT_ALREADY_TAKEN 7
#define CREATE_ROAD_NOT_CONTINUOUS_SPEED_LIMITS 8
#define CREATE_ROAD_NOT_ENOUGH_SPEED_LIMITS 9
#define CREATE_ROAD_NOT_WHOLE_ROAD_COVERED 10
#define CREATE_ROAD_UNKNOWN_ERROR 11
#define CREATE_ROAD_NO_SUCH_GENERATOR 12
#define CREATE_ROAD_GENERATOR_ALREADY_TAKEN 13
// create multilane road
#define CREATE_MULTILANE_ROAD_VECTORS_SIZES_MISMATCH 300
// create slip road
#define CREATE_SLIP_ROAD_NO_SUCH_JOINED_ROAD 400
#define CREATE_SLIP_ROAD_TOO_SHORT 401
#define CREATE_SLIP_ROAD_JOIN_POSITION_NEGATIVE 402
#define CREATE_SLIP_ROAD_JOIN_POSITION_TOO_BIG 403
#define MIN_SLIP_ROAD_LEN 20.0
// add generator
#define CREATE_GENERATOR_SUCCESS 0
#define CREATE_GENERATOR_PARSING_ERROR 2
#define CREATE_GENERATOR_ALREADY_EXISTS 3
// attach to generator
#define ATTACH_TO_GENERATOR_SUCCESS 0
#define ATTACH_TO_GENERATOR_NO_SUCH_GENERATOR 1
#define ATTACH_TO_GENERATOR_PARSING_ERROR 2
// create lights system
#define CREATE_LIGHTS_SYSTEM_SUCCESS 0
#define CREATE_LIGHTS_SYSTEM_NO_SUCH_JUNCTION 2
#define CREATE_LIGHTS_SYSTEM_ALREADY_EXISTS 3
// create lights
#define CREATE_LIGHTS_SUCCESS 0
#define CREATE_LIGHTS_NO_SUCH_LIGHTS_SYSTEM 2
#define CREATE_LIGHTS_ALREADY_EXISTS 3
// create lights group
#define CREATE_LIGHTS_GROUP_SUCCESS 0
#define CREATE_LIGHTS_GROUP_ALREADY_EXISTS 2
#define CREATE_LIGHTS_GROUP_NO_SUCH_LIGHTS_SYSTEM 3
#define CREATE_LIGHTS_GROUP_TOO_SMALL_DURATION 4
#define CREATE_LIGHTS_GROUP_NOT_ALL_LIGHTS_COVERED 5
#define CREATE_LIGHTS_GROUP_LIGHT_IN_GREEN_AND_RED_AT_THE_SAME_TIME 6
#define CREATE_LIGHTS_GROUP_LIGHTS_FROM_MULTIPLE_SYSTEMS 7
// set template car
#define SET_TEMPLATE_CAR_SUCCESS 0
#define SET_TEMPLATE_CAR_NO_SUCH_GENERATOR 1
#define SET_TEMPLATE_CAR_NULL_CAR 2
#define SET_TEMPLATE_CAR_UNSPECIFIED_ROAD 3
// tick elements
#define TICK_ELEMENTS_SUCCESS 0
#define TICK_ELEMENTS_ERR_LIGHTS_SYSTEM 1
#define TICK_ELEMENTS_ERR_GENERATOR 2
#define TICK_ELEMENTS_ERR_CAR 3
#define TICK_ELEMENTS_ERR_ROUNDABOUT 4
// create route
#define CREATE_ROUTE_SUCCESS 0
#define CREATE_ROUTE_NON_UNIQUE_ID 1
#define CREATE_ROUTE_WRONG_ROUTE 2
// warnings
#define WARNING_NO_JUNCTION_INPUT 1
#define WARNING_ROUNDABOUT_WITH_NO_ROUNDABOUT_GATE 2
#define WARNING_ROUNDABOUT_WITH_NO_ROUNDABOUT_INPUT 3
#define WARNING_ROUNDABOUT_WITH_NO_OUTPUT 4
#define WARNING_JUNCTION_INPUT_WITH_NO_GATE 5
#define WARNING_ROAD_NOT_COVERED_BY_ROUTE 6
#define WARNING_GENERATOR_WITH_NO_ROAD 7
#define WARNING_LIGHTS_SYSTEM_WITH_NO_LIGHTS 8
#define WARNING_LIGHTS_SYSTEM_NO_LIGHTS_GROUPS 9
#define WARNING_LIGHTS_NOT_IN_GREEN_GROUP 10
#define WARNING_LIGHTS_GROUP_WITH_NO_LIGHTS 11
#define WARNING_GATE_WITH_NO_ROAD 12
// elements
#define ELEMENT_JUNCTION 0
#define ELEMENT_JUNCTION_INPUT 1
#define ELEMENT_GATE 2
#define ELEMENT_ROAD 3
#define ELEMENT_GENERATOR 4
#define ELEMENT_LIGHTS_SYSTEM 5
#define ELEMENT_LIGHTS 6
#define ELEMENT_LIGHTS_GROUP 7
// add component gate
#define ADD_COMPONENT_GATE_SUCCESS 0
#define ADD_COMPONENT_GATE_NO_SUCH_GATE 1
#define ADD_COMPONENT_GATE_NOT_COMPOUND_GATE 2
#define ADD_COMPONENT_GATE_NO_SUCH_COMPONENT 3
#define ADD_COMPONENT_GATE_ALREADY_IN_USE 4
#define ADD_COMPONENT_GATE_UNKNOWN_ERROR_DURING_ADDING 5


class ModelWarning;




class Model : public ModelInterface, public FinishedCarListener, public FirstCarGenerationListener
{
public:
	// constructors ///////////////////////////////////////////////////////////////////////////////
	Model(int minCarsInGoalPerType = DEFALUT_MIN_CARS_IN_GOAL_PER_TYPE);
	~Model();
	// methods ////////////////////////////////////////////////////////////////////////////////////
	static int load(std::string path, Model** ppModel, std::vector<ModelWarning>& warnings);
	/*
		Start the simulation
	*/
	virtual int run(double tickDuration, double timeoutSeconds) override;
	virtual double getAvgTime() override;
	virtual double getAdditionalTime() override;
	virtual int getNumOfReadyCars() override;
	virtual void translate(double* individual, std::unordered_map<int, double>& groups,
		std::unordered_map<int, double>& systems) override;
	virtual int getNumOfFinishedCars() override;
	virtual bool update(double* individual) override;
	virtual void getRanges(std::pair<double, double>* ranges) override;
	virtual int getProblemSize() override;
	/*
		Restores state before running the model, so that it can be run again (deletes all cars and 
		resets statistics and other elements which need to be reset, e.g. roads)
	*/
	void reset();
	/*
		Called by the final gate whenver a car arrives there
	*/
	void carFinished(Car* pCar) override;
	/*
		Called by a generator when the first car is generated. It is used to determine the time when
		evaluation should be called - only the cars which start after the first car from every 
		generator arrived will be evaluated.
	*/
	void firstCarGenerated(Car* pCar) override;
	/*
		Creates a new junction in the model. The junction will have the specified id.

		Returns:
			CREATE_JUNCTION_SUCCESS			- success
			CREATE_JUNCTION_ALREADY_EXISTS	- junction with that name already exists
	*/
	int createJunction(int id);
	/*
		Creates a new junction of type roundabout in the model.

		Returns:
			CREATE_ROUNDABOUT_SUCCESS			- success
			CREATE_ROUNDABOUT_ALREADY_EXISTS	- id is not unique
			CREATE_ROUNDABOUT_TOO_SHORT			- length is too short
			CREATE_ROUNDABOUT_INCORRECT_SPEED	- speed is too low or too big
	*/
	int createRoundabout(int id, double length, double speed);
	/*
		Returns:
			ADD_ROUNDABOUT_OUTPUT_SUCCESS;
			ADD_ROUNDABOUT_OUTPUT_CANT_ADD_TO_ROUNDABOUT;
			ADD_ROUNDABOUT_OUTPUT_JUNCTION_HAS_WRONG_NUMBER_OF_GATES;
			ADD_ROUNDABOUT_OUTPUT_JUNCTION_INPUT_TAKEN;
			ADD_ROUNDABOUT_OUTPUT_NO_SUCH_JUNCTION_INPUT;
			ADD_ROUNDABOUT_OUTPUT_NO_SUCH_ROUNDABOUT;
	*/
	int addRoundaboutOutput(int roundaboutId, double position, int junctionInputId);
	/*
		Creates a new junction input with the specified id and attaches it to the specified junction.

		Returns:
			CREATE_JUNCTION_INPUT_SUCCESS				- success
			CREATE_JUNCTION_INPUT_ALREADY_EXISTS		- junction input with that name already exists
			CREATE_JUNCTION_INPUT_JUNCTION_NOT_FOUND	- there is no junction with such name
	*/
	int createJunctionInput(int inputId, int junctionId);
	/*
		Creates an always closed gate and adds it to the specified input.
		Returns:
			CREATE_GATE_SUCCESS
			CREATE_GATE_NO_SUCH_INPUT
			CREATE_GATE_ALREADY_EXISTS
	*/
	int createAlwaysClosedGate(int gateId, int inputId);
	/*
		Creates an always open gate and adds it to the specified input.

		Returns:
			CREATE_GATE_SUCCESS
			CREATE_GATE_NO_SUCH_INPUT
			CREATE_GATE_ALREADY_EXISTS
	*/
	int createAlwaysOpenGate(int gateId, int inputId);
	/*
		Creates a compund gate and adds it to the specified input. Component gates are obtained
		from the passed ids.

		Returns:
			CREATE_GATE_SUCCESS
			CREATE_GATE_NO_SUCH_INPUT
			CREATE_GATE_ALREADY_EXISTS
			CREATE_GATE_NO_SUCH_GATE
			CREATE_GATE_NO_COMPONENTS
			CREATE_GATE_COMPONENT_ALREADY_IN_USE
	*/
	int createCompoundGate(int gateId, int inputId, 
		std::vector<int>& componentGatesIds);
	int addComponentGate(int compoundGateId, int componentGateId);
	/*
		Creates a conflict gate and adds it to the specified input. Conflict gate is a gate which
		goes through other roads.

		Arguments:
			@gateId - id with which the gate will be created
			@inputId - id of the junction input to which the road will be added
			@conflictRoadsIds - vector of vectors of conflict roads. The inner vector has to be
			created because often the conflict extends to other roads, e.g. it starts at the
			end of a road, the road ends in a junction and the conflict continues to the next road
			which is continuation of that ended road. And there may be several such conflicting
			roads sets.
			@conflictStarts - vector with postions on which the conflict starts. first element is
			associated with the first conflictRoad/s and so on
			@conflictsDurations - vector with durations (time) of the conflict (for how long the conflict
			road has to be free on the specified section). [0] -> conflictRoadsIds[0]
			@conflicsPositions - positons at which the conflict occurs. [0] -> conflictRoadsIds[0]

		Returns:
			CREATE_GATE_SUCCESS
			CREATE_GATE_NO_SUCH_INPUT
			CREATE_GATE_ALREADY_EXISTS
			CREATE_GATE_NO_CONFLICT_ROADS
			CREATE_GATE_NEGATIVE_CONFLICT_START
			CREATE_GATE_NEGATIVE_REQUIRED_DURATION
			CREATE_GATE_NEGATIVE_CONFLICT_POSITION
			CREATE_GATE_CONFLICT_ROAD_NOT_FOUND
			CREATE_GATE_CONFLICT_VECTORS_SIZES_DIFFER
	*/
	int createConflictGate(int gateId, int inputId,
		std::vector<std::vector<int>>& conflictRoadsIds, std::vector<double>& conflictStarts,
		std::vector<double>& conflictsDurations, std::vector<double>& conflictsPositions);
	/*
		Creates a grade separated lights gate and adds it to the specified junction input. The
		gate will be open or closed depending on the speficied lights.

		Returns:
			CREATE_GATE_SUCCESS
			CREATE_GATE_NO_SUCH_INPUT
			CREATE_GATE_ALREADY_EXISTS
			CREATE_GATE_NO_SUCH_LIGHTS
	*/
	int createGradeSeparatedLightsGate(int gateId, int inputId, int lightsId);
	/*
		Creates a gate to enter a roundabout.

		Returns:
			CREATE_GATE_SUCCESS							- success
			CREATE_GATE_NO_SUCH_INPUT					- such input doesn't exist
			CREATE_GATE_ALREADY_EXISTS					- gate with such id already exists
			CREATE_ROUNDABOUT_GATE_NO_SUCH_ROUNDABOUT	- there is no roundabout with such id
			CREATE_ROUNDABOUT_GATE_CANT_CONNECT			- wrong position or other error connected
														- with connecting to a roundbout
	*/
	int createRoundaboutGate(int gateId, int roundaboutId, int inputId, double position);
	/*
		Creates a road which starts in the specified gates and ends in the specified junction input.

		Returns:
			CREATE_ROAD_SUCCESS						- success
			CREATE_ROAD_ALREADY_EXISTS				- road with the name already exists
			CREATE_ROAD_NOT_CONTINUOUS_SPEED_LIMITS - there are gaps between speed limits
			CREATE_ROAD_NOT_ENOUGH_SPEED_LIMITS		- speed limits list is empty
			CREATE_ROAD_NOT_WHOLE_ROAD_COVERED		- the speed limits do not fit the road
			CREATE_ROAD_NO_SUCH_GATE				- gate with such name does not exist
			CREATE_ROAD_GATE_ALREADY_TAKEN			- the gate had already have associated road
			CREATE_ROAD_TOO_SHORT_ROAD				- the road is too short
			CREATE_ROAD_NO_SUCH_INPUT				- there is no input with such name
			CREATE_ROAD_INPUT_ALREADY_TAKEN			- the input has other associated road
			CREATE_ROAD_UNKNOWN_ERROR				- unknown error
	*/
	int createRoad(int roadId, std::vector<int>& gatesIds, int junctionInputId,
		double length, std::vector<SpeedLimit> speedLimits);
	/*
		Creates a road which starts in the specified gate and ends in the specified junction input.

		Returns:
			CREATE_ROAD_SUCCESS						- success
			CREATE_ROAD_ALREADY_EXISTS				- road with the name already exists
			CREATE_ROAD_NOT_CONTINUOUS_SPEED_LIMITS - there are gaps between speed limits
			CREATE_ROAD_NOT_ENOUGH_SPEED_LIMITS		- speed limits list is empty
			CREATE_ROAD_NOT_WHOLE_ROAD_COVERED		- the speed limits do not fit the road
			CREATE_ROAD_NO_SUCH_GATE				- gate with such name does not exist
			CREATE_ROAD_GATE_ALREADY_TAKEN			- the gate had already have associated road
			CREATE_ROAD_TOO_SHORT_ROAD				- the road is too short
			CREATE_ROAD_NO_SUCH_INPUT				- there is no input with such name
			CREATE_ROAD_INPUT_ALREADY_TAKEN			- the input has other associated road
			CREATE_ROAD_UNKNOWN_ERROR				- unknown error
	*/
	int createRoad(int roadId, int gateId, int junctionInputId,
		double length, std::vector<SpeedLimit> speedLimits);
	/*
		Creates a road which does not have any predecessor - its one end is directly connected to
		a generator. The second end of the road goes to the specified junction input.

		Returns:
			CREATE_ROAD_SUCCESS						- success
			CREATE_ROAD_ALREADY_EXISTS				- road with the name already exists
			CREATE_ROAD_NOT_CONTINUOUS_SPEED_LIMITS - there are gaps between speed limits
			CREATE_ROAD_NOT_ENOUGH_SPEED_LIMITS		- speed limits list is empty
			CREATE_ROAD_NOT_WHOLE_ROAD_COVERED		- the speed limits do not fit the road
			CREATE_ROAD_TOO_SHORT_ROAD				- the road is too short
			CREATE_ROAD_NO_SUCH_INPUT				- there is no input with such name
			CREATE_ROAD_INPUT_ALREADY_TAKEN			- the input has other associated road
			CREATE_ROAD_UNKNOWN_ERROR				- unknown error
			CREATE_ROAD_NO_SUCH_GENERATOR			- there is no generator with such name
			CREATE_ROAD_GENERATOR_ALREADY_TAKEN
	*/
	int createExternalRoad(int roadId, int generatorId,	int junctionInputId, double length, 
		std::vector<SpeedLimit> speedLimits);
	/*
		Create a road which will be the last road. Cars which go through it will end up in a
		final gate.

		Returns:
			CREATE_ROAD_GATE_ALREADY_TAKEN
			CREATE_ROAD_NO_SUCH_GATE
			CREATE_ROAD_SUCCESS
			CREATE_ROAD_ALREADY_EXISTS
			CREATE_ROAD_TOO_SHORT_ROAD
			CREATE_ROAD_NOT_WHOLE_ROAD_COVERED
			CREATE_ROAD_NOT_ENOUGH_SPEED_LIMITS
			CREATE_ROAD_NOT_CONTINUOUS_SPEED_LIMITS
			CREATE_ROAD_UNKNOWN_ERROR
	*/
	int createFinalRoad(int roadId, std::vector<int>& gatesIds, double length,
		std::vector<SpeedLimit> speedLimits);
	/*
		Create a road which will be the last road. Cars which go through it will end up in a 
		final gate.

		Returns:
			CREATE_ROAD_GATE_ALREADY_TAKEN
			CREATE_ROAD_NO_SUCH_GATE
			CREATE_ROAD_SUCCESS
			CREATE_ROAD_ALREADY_EXISTS
			CREATE_ROAD_TOO_SHORT_ROAD
			CREATE_ROAD_NOT_WHOLE_ROAD_COVERED
			CREATE_ROAD_NOT_ENOUGH_SPEED_LIMITS
			CREATE_ROAD_NOT_CONTINUOUS_SPEED_LIMITS
			CREATE_ROAD_UNKNOWN_ERROR
	*/
	int createFinalRoad(int roadId, int gateId, double length, 
		std::vector<SpeedLimit> speedLimits);
	/*
		Creates a road which consists of multiple lanes. In this case lanes are represented with
		single roads.

		Returns:
			CREATE_ROAD_SUCCESS
			CREATE_ROAD_ALREADY_EXISTS
			CREATE_ROAD_NO_SUCH_GATE
			CREATE_ROAD_TOO_SHORT_ROAD
			CREATE_ROAD_NO_SUCH_INPUT
			CREATE_ROAD_GATE_ALREADY_TAKEN
			CREATE_ROAD_INPUT_ALREADY_TAKEN
			CREATE_ROAD_NOT_CONTINUOUS_SPEED_LIMITS
			CREATE_ROAD_NOT_ENOUGH_SPEED_LIMITS
			CREATE_ROAD_NOT_WHOLE_ROAD_COVERED
			CREATE_ROAD_UNKNOWN_ERROR
			CREATE_ROAD_NO_SUCH_GENERATOR
			CREATE_ROAD_GENERATOR_ALREADY_TAKEN
			CREATE_MULTILANE_ROAD_VECTORS_SIZES_MISMATCH
	*/
	int createMultiLaneRoad(std::vector<int> lanesIds, std::vector<std::vector<int>> gatesIds,
		std::vector<int> junctionInputsIds, double length, std::vector<SpeedLimit> speedLimits);
	/*
		Creates a road which consists of multiple lanes. In this case lanes are represented with
		single roads.

		Returns:
			CREATE_ROAD_SUCCESS 
			CREATE_ROAD_ALREADY_EXISTS 
			CREATE_ROAD_NO_SUCH_GATE 
			CREATE_ROAD_TOO_SHORT_ROAD 
			CREATE_ROAD_NO_SUCH_INPUT 
			CREATE_ROAD_GATE_ALREADY_TAKEN 
			CREATE_ROAD_INPUT_ALREADY_TAKEN 
			CREATE_ROAD_NOT_CONTINUOUS_SPEED_LIMITS 
			CREATE_ROAD_NOT_ENOUGH_SPEED_LIMITS 
			CREATE_ROAD_NOT_WHOLE_ROAD_COVERED 
			CREATE_ROAD_UNKNOWN_ERROR 
			CREATE_ROAD_NO_SUCH_GENERATOR 
			CREATE_ROAD_GENERATOR_ALREADY_TAKEN 
			CREATE_MULTILANE_ROAD_VECTORS_SIZES_MISMATCH 
	*/
	int createMultiLaneRoad(std::vector<int> lanesIds, std::vector<int> gatesIds, 
		std::vector<int> junctionInputsIds, double length, std::vector<SpeedLimit> speedLimits);

	int createExternalMultiLaneRoad(std::vector<int> lanesIds, std::vector<int> generatorsIds, 
		std::vector<int> junctionInputsIds, double length,
		std::vector<SpeedLimit> speedLimits);

	int createFinalMultiLaneRoad(std::vector<int> lanesIds, std::vector<std::vector<int>>& gatesIds, 
		double length, std::vector<SpeedLimit> speedLimits);
	int createFinalMultiLaneRoad(std::vector<int> lanesIds, std::vector<int>& gatesIds,
		double length, std::vector<SpeedLimit> speedLimits);
	/*
		Creates a slip road. Speed limit will be the same as on the joined road.

		@roadId			- id of the slip road
		@gateId			- gate which guards this road
		@joinedRoadId	- road to which the cars will change through the slip road
		@joinPosition	- positon on joined road at which the slip road will start
		@length			- length of the road
	*/
	int createSlipRoad(int roadId, std::vector<int>& gatesIds, int joinedRoadId, double joinPosition, 
		double length);
	/*
		Creates a new generator of the specified type.

		Returns:
			CREATE_GENERATOR_SUCCESS		- success
			CREATE_GENERATOR_PARSING_ERROR	- could not parse a generator from the provided strings
			CREATE_GENERATOR_ALREADY_EXISTS - generator with such name already exists
	*/
	int createGenerator(int generatorId, std::string& generatorType, 
		std::vector<string>& params, int* pGlobalId = nullptr);
	/*
		Creates a new generator of the specified type with the passed parameters and combines it with
		another generator which already exists.

		Returns:
			ATTACH_TO_GENERATOR_SUCCESS				- success
			ATTACH_TO_GENERATOR_PARSING_ERROR		- could not parse a generator from the string
			ATTACH_TO_GENERATOR_NO_SUCH_GENERATOR	- there is no generator to attach to with such name
	*/
	int attachToGenerator(int generatorId, std::string& attachedGeneratorType,
		std::vector<string>& attachedGeneratorParams, int* pGlobalId = nullptr);
	/*
		Creates a new lights sytem for the specified junction.
		Returns:
			CREATE_LIGHTS_SYSTEM_SUCCESS
			CREATE_LIGHTS_SYSTEM_NO_SUCH_JUNCTION
			CREATE_LIGHTS_SYSTEM_ALREADY_EXISTS
	*/
	int createLightsSystem(int lightsSystemId, int junctionId);
	/*
		Creates new lights which are part of the specified lights system.

		Returns:
			CREATE_LIGHTS_SUCCESS
			CREATE_LIGHTS_NO_SUCH_LIGHTS_SYSTEM
			CREATE_LIGHTS_ALREADY_EXISTS
	*/
	int createLights(int lightsId, int lightsSystemId);
	/*
		Creates a new lights group. LightsGroup combines lights which should be greeen with lights
		which should be red during a specified duration. Groups are used to set proper timing for
		lights.

		Returns:
			CREATE_LIGHTS_GROUP_SUCCESS
			CREATE_LIGHTS_GROUP_ALREADY_EXISTS
			CREATE_LIGHTS_GROUP_NOT_ALL_LIGHTS_COVERED
			CREATE_LIGHTS_GROUP_NO_SUCH_LIGHTS_SYSTEM
			CREATE_LIGHTS_GROUP_TOO_SMALL_DURATION
			CREATE_LIGHTS_GROUP_LIGHT_IN_GREEN_AND_RED_AT_THE_SAME_TIME
			CREATE_LIGHTS_GROUP_LIGHTS_FROM_MULTIPLE_SYSTEMS
	*/
	int createLightsGroup(int lightsGroupId, int lightsSystemId, std::vector<int>& greenLightsIds,
		std::vector<int>& redLightsIds, double duration, 
		double minDuration = DEFAULT_MIN_LIGHTS_GROUP_DURATION, 
		double maxDuration = DEFAULT_MAX_LIGHTS_GROUP_DURATION);
	/*
		Creates a route for cars so that it can be reused.
	*/
	int createRoute(int id, std::vector<int>& idsOfRoadsInRoute);
	std::vector<Road*>* getRoute(int id);
	/*
		Returns a route for a car by roads ids. It also checks if the road is valid, e.g. if
		it ends on a final road, if the route is sequential etc.
	*/
	std::vector<Road*>* obtainRoute(std::vector<int>& idsOfRoadsInRoute);
	/*
		Sets template car for a generator. In order to set a car for a generator whic is part of
		another generator add the template immediatly after attaching the generator.
	*/
	int setTemplateCar(int generatorId, Car* pTemplateCar);
	int setTemplateCarByGlobalId(int generatorGlobalId, Car* pTemplateCar);
	int generateUniqueIdForJunctionInput();
	int generateUniqueGlobalIdForGenerator();
	std::vector<ModelWarning> findWarnings();

	// getters & setters //////////////////////////////////////////////////////////////////////////
	// getters and setters are not supposed to be used anywhere beside tests
	std::unordered_map<int, Junction*>& getJunctions();
	std::unordered_map<int, JunctionInput*>& getJunctionInputs();
	std::unordered_map<int, Gate*>& getGates();
	std::unordered_map<int, LightsSystem*>& getLightsSystems();
	std::unordered_map<int, Lights*>& getLights();
	std::unordered_map<int, LightsGroup*>& getLightsGroups();
	std::unordered_map<int, Generator*>& getGenerators();
	std::unordered_map<int, Road*>& getRoads();
	std::unordered_map<int, std::vector<Road*>*>& getRoutes();

private:
	// fields /////////////////////////////////////////////////////////////////////////////////////
	std::unordered_map<int, Junction*> junctions;
	std::vector<Roundabout*> roundabouts; // just for tick, they are also stored in junctions
	std::unordered_map<int, JunctionInput*> junctionInputs;
	std::vector<JunctionInput*> roundaboutJunctionInputs; // for adding rondabout road to route
	std::unordered_map<int, Gate*> gates;
	std::unordered_map<int, LightsSystem*> lightsSystems;
	std::vector<LightsSystem*> vecLightsSystems; // just for faster access -> tick
	std::unordered_map<int, LightsGroup*> lightsGroups;
	/*
		duration of each lights group will be passed in a vector, so a mapping between ids and
		positions in that vector is required. the first element is an id of a lightsGroup and the
		second is the position (index) of the corresponding group.
	*/
	std::unordered_map<int, int> lightsGroupsMapping;
	/*
		As in lightsGroupsMapping, with one difference - the mapping is not direct, 0 is the first
		position after the last lights group in an individual
	*/
	std::unordered_map<int, int> lightsSystemsMapping;
	std::unordered_map<int, Lights*> lights;
	std::unordered_map<int, Road*> roads;
	std::unordered_map<int, std::vector<Road*>*> routes;
	std::unordered_map<std::vector<Road*>*, double> routesOptimalTimes;
	std::unordered_map<int, Generator*> generators;
	/*
		For storing all generators separately (inner generators also have ids here). Ids are
		generated automatically and are not known to the user. It is for assigning template
		cars, because otherwise it's hard to assign template cars for inner generators.
	*/
	std::unordered_map<int, Generator*> allGenerators;
	std::vector<Generator*> vecGenerators;
	std::vector<Generator*> unfinishedGenerators;
	std::list<Car*> activeCars;
	FinalJunctionInput* pFinalJunctionInput;
	std::unordered_set<Car*> firstCars;
	std::unordered_set<Car*> finishedCars;
	/*
		When an evaluated cars from a generator reaches the goal then it is added for that 
		generator. It is used for automatically stopping the simulation when certain number of
		cars in goal
	*/
	std::unordered_map<Generator*, int> generatorsWithCarsInGoal;
	bool evaluationStarted;
	int numOfUnfinishedGenerators; // runs until it reaches 0 (if no time specified)
	int minCarsInGoalPerType; // from each generator at least that many evaluated cars must arrive
							  // if no time is specified

	// methods ////////////////////////////////////////////////////////////////////////////////////
	/*
		Creates a road which beginning is not associated with an other road or a generator. It is
		common part of creating specific kinds of roads.
	*/
	int createRoadWithNoStart(int roadId, int junctionInputId, double length,
		std::vector<SpeedLimit> speedLimits, Road** pCreatedRoad);
	bool areGreenAndRedLightsDisjoint(std::vector<int>& greenLightsIds, 
		std::vector<int> redLightsIds);
	bool areLightsFromTheLightsSystem(LightsSystem* pLightsSystem, 
		std::vector<Lights*>& lights);
	void removeDuplicates(std::vector<int> vec);
	void getLightsWithIds(std::vector<int>& lightsIds, std::vector<Lights*>* pRes);
	/*
		Checks if the gate does not exist and if there is a junction input with the specified id. 
		If both of these conditions are met, then *ppInput is set to point to the input with the
		specified id.
	*/
	int commonGateCheck(int gateId, int inputId, JunctionInput** ppInput);
	/*
		Transforms roads ids list into roads pointers list.
		Returns:
			true -> success
			false -> one of the roads does not exist
	*/
	bool getRoads(std::vector<std::vector<int>>& conflictRoadsIds,
		std::vector<std::vector<Road*>>* pRes);
	bool isExternalRoad(Road* pRoad);
	int processRoadCreationError(int error);
	void orderGeneratorsToStartGeneratingEvaluatedCars();
	int tickElements(double tickDuration);
	int createLane(Road* pLane, int roadId, std::vector<int>& gatesIds, int junctionInputId);
	int createExternalLane(int laneId, int generatorId, int junctionInputId, double length, 
		vector<SpeedLimit>& speedLimits, Road** ppLane);
	int createFinalLane(double length, int laneId, std::vector<int>& gatesIds, vector<SpeedLimit>& speedLimits,
		Road** ppLane);
	/*
		IMPORTANT: if road is a lane, then it is not deleting its connections!
	*/
	bool deleteRoad(int id);
	int validateSlipRoad(int roadId, int joinedRoadId, double joinPosition,
		double length);
	int generateUniqueIdForGate();
	Roundabout* getRoundabout(int id);
	void reloadLightsDurations();
	int setTemplateCarGeneral(int generatorId, Car* pTemplateCar, 
		unordered_map<int, Generator*>& collection);
	int getJunctionId(Junction* pJunction, bool* pFound = nullptr);
	int getRoadId(Road* pRoad, bool* pFound = nullptr);

	// warnings
	/*
		Checks if every junction has at least one junction input.
	*/
	void findWarningsInJunctions(std::vector<ModelWarning>& warnings);
	/*
		Checks if every roundabout has at least one input with a roundabout gate and at least
		one output.
	*/
	void findWarningsInRoundabouts(std::vector<ModelWarning>& warnings);
	/*
		Checks if every junction input has at least one gate.
	*/
	void findWarningsInJunctionInputs(std::vector<ModelWarning>& warnings);
	/*
		Checks if every road is referenced in at least one route.
	*/
	void findWarningsInRoads(std::vector<ModelWarning>& warnings);
	/*
		Checks if every generator is connected with a road.
	*/
	void findWarningsInGenerators(std::vector<ModelWarning>& warnings);
	/*
		Checks if every lights system has at least one lights and at least one lights group.
	*/
	void findWarningsInLightsSystems(std::vector<ModelWarning>& warnings);
	/*
		Checks if every lights are covered by at least one "green" lights group.
	*/
	void findWarningsInLights(std::vector<ModelWarning>& warnings);
	bool isInGreenLightsGroup(Lights* pLights);
	/*
		Checks if every lights group contains at least one lights.
	*/
	void findWarningsInLightsGroups(std::vector<ModelWarning>& warnings);
	/*
		Checks if every gate has has an associated road.
	*/
	void findWarningsInGates(std::vector<ModelWarning>& warnings);
	double getRouteOptimalTime(vector<Road*>* pRoute);
	int runWithTimeout(double tickDuration, double timeoutSeconds);
	int runWithNoTimeout(double tickDuration);
};



class ModelWarning
{
public:
	// constructors
	ModelWarning(int code, int elementType, int elementId) : code{ code }, 
		elementType{ elementType }, elementId{ elementId } {}
	// getters & setters
	int getCode() { return code; }
	int getElementId() { return elementId; }
	int getElementType() { return elementType; }
private:
	int code;
	int elementType;
	int elementId;
};