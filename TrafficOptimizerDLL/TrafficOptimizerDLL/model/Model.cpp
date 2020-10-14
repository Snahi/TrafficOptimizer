#include "../pch.h"
#include "Model.h"
#include <regex>
#include "junction/gate/AlwaysClosedGate.h"
#include "junction/gate/AlwaysOpenGate.h"
#include "junction/gate/CompoundGate.h"
#include "junction/gate/ConflictGate.h"
#include "junction/gate/GradeSeparatedLightsGate.h"
#include <iostream>
#include "ModelLoader.h"
#include <chrono>


using namespace std;



// constructors ///////////////////////////////////////////////////////////////////////////////////



Model::Model(int minCarsInGoalPerType)
{
	FinalGate* pFinalGate = new FinalGate(this);
	pFinalJunctionInput = new FinalJunctionInput(pFinalGate);
	numOfUnfinishedGenerators = 0;
	evaluationStarted = false;
	this->minCarsInGoalPerType = minCarsInGoalPerType;
}



// destructor /////////////////////////////////////////////////////////////////////////////////////



Model::~Model()
{
	for (auto& pairJunction : junctions)
		delete pairJunction.second;

	for (auto& pairRoad : roads)
		delete pairRoad.second;

	for (auto& pairGenerator : generators)
		delete pairGenerator.second;

	for (Car* pCar : activeCars)
		delete pCar;

	for (Car* pCar : finishedCars)
		delete pCar;

	if (pFinalJunctionInput != nullptr)
		delete pFinalJunctionInput;

	for (auto pairRoute : routes)
		delete pairRoute.second;
}



// methods ////////////////////////////////////////////////////////////////////////////////////////



int Model::load(string path, Model** ppModel, vector<ModelWarning>& warnings)
{
	ModelLoader loader(path);
	Model* pModel = nullptr;
	LoadResult res = loader.load();
	if (res.success())
		pModel = res.getModel();

	*ppModel = pModel;
	warnings = res.getWarnings();

	return res.getErrCode();
}



int Model::run(double tickDuration, double timeoutSeconds)
{
	if (tickDuration > MAX_TICK_DURATION)
		tickDuration = MAX_TICK_DURATION;

	if (timeoutSeconds > 0)
		return runWithTimeout(tickDuration, timeoutSeconds);
	else
		return runWithNoTimeout(tickDuration);
}



int Model::runWithTimeout(double tickDuration, double timeoutSeconds)
{
	// convert timeout to chrono::milliseconds
	chrono::milliseconds timeout((long)(timeoutSeconds * 1000));

	int tickRes = 0;
	auto start = chrono::steady_clock::now();
	while (numOfUnfinishedGenerators > 0)
	{
		if ((tickRes = tickElements(tickDuration)) != TICK_ELEMENTS_SUCCESS)
			return tickRes;

		if (chrono::steady_clock::now() - start > timeout)
			return RUN_TIME_OUT;
	}

	return RUN_SUCCESS;
}



int Model::runWithNoTimeout(double tickDuration)
{
	int tickRes = 0;
	while (numOfUnfinishedGenerators > 0)
	{
		if ((tickRes = tickElements(tickDuration)) != TICK_ELEMENTS_SUCCESS)
			return tickRes;
	}

	return RUN_SUCCESS;
}



void Model::translate(double* individual, std::unordered_map<int, double>& groups,
	std::unordered_map<int, double>& systems)
{
	// translate lights groups
	for (auto& idIdx : lightsGroupsMapping)
		groups[idIdx.first] = individual[idIdx.second];

	// translate lights sytems
	int numOfGroups = (int)groups.size();
	for (auto& idIdx : lightsSystemsMapping)
		systems[idIdx.first] = individual[numOfGroups + idIdx.second];
}



bool Model::update(double* individual) 
{
	reset();

	// update lights groups durations
	int lightsGroupId;
	LightsGroup* pLightsGroup;
	double newDuration;

	for (auto itLightsGroups = lightsGroups.begin(); itLightsGroups != lightsGroups.end();
		++itLightsGroups)
	{
		lightsGroupId = itLightsGroups->first;
		pLightsGroup = itLightsGroups->second;
		newDuration = individual[lightsGroupsMapping[lightsGroupId]];

		if (pLightsGroup->setDuration(newDuration) != SET_DURATION_SUCCESS)
			return false;
	}

	reloadLightsDurations();

	// update lights systems offsets
	int mappingOffset = lightsGroups.size();
	int mappedIdx;
	int systemId;
	LightsSystem* pSystem;
	for (auto& pairSystem : lightsSystems)
	{
		systemId = pairSystem.first;
		pSystem = pairSystem.second;
		mappedIdx = mappingOffset + lightsSystemsMapping[systemId];
		pSystem->setOffset(individual[mappedIdx]);
	}

	return true;
}



int Model::tickElements(double tickDuration)
{
	for (LightsSystem* pLightsSystem : vecLightsSystems)
	{
		if (pLightsSystem->tick(tickDuration) != TICK_LIGHTS_SYSTEM_SUCCESS)
			return TICK_ELEMENTS_ERR_LIGHTS_SYSTEM;
	}

	for (Generator* pGenerator : vecGenerators)
	{
		if (pGenerator->tick(tickDuration) != TICK_GENERATOR_SUCCESS)
			return TICK_ELEMENTS_ERR_GENERATOR;
	}

	for (Car* pActiveCar : activeCars)
	{
		if (pActiveCar->tick(tickDuration) != TICK_CAR_SUCCESS)
			return TICK_ELEMENTS_ERR_CAR;
	}

	for (Roundabout* pRoundabout : roundabouts)
	{
		if (pRoundabout->tick(tickDuration) != TICK_SUCCESS)
			return TICK_ELEMENTS_ERR_ROUNDABOUT;
	}

	// remove finished cars from activeCars
	list<Car*>::iterator itActiveCars = activeCars.begin();
	while (itActiveCars != activeCars.end())
	{
		if ((*itActiveCars)->isFinished())
			itActiveCars = activeCars.erase(itActiveCars);
		else
			itActiveCars++;
	}

	return TICK_ELEMENTS_SUCCESS;
}



void Model::reset()
{
	for (Roundabout* pRoundabout : roundabouts)
		pRoundabout->reset();

	for (auto itRoads = roads.begin(); itRoads != roads.end(); itRoads++)
		itRoads->second->reset();

	unfinishedGenerators.clear();
	for (Generator* pGenerator : vecGenerators)
	{
		pGenerator->reset();
		unfinishedGenerators.push_back(pGenerator);
	}

	for (Car* pCar : activeCars)
		delete pCar;

	activeCars.clear();
	firstCars.clear();

	for (Car* pCar : finishedCars)
		delete pCar;

	finishedCars.clear();

	generatorsWithCarsInGoal.clear();

	evaluationStarted = false;
	numOfUnfinishedGenerators = generators.size();
}



void Model::reloadLightsDurations()
{
	for (LightsSystem* pLightsSystem : vecLightsSystems)
		pLightsSystem->reset();
}



void Model::carFinished(Car* pCar)
{
	pCar->finish();

	finishedCars.insert(pCar);

	if (!evaluationStarted)
	{
		auto it = firstCars.find(pCar);
		if (it != firstCars.end()) // if is a first car
		{
			firstCars.erase(it);

			if (firstCars.empty())
			{
				evaluationStarted = true;
				orderGeneratorsToStartGeneratingEvaluatedCars();

				return;
			}
		}
	}
	else
	{
		// for stopping evaluation
		if (pCar->isEvaluated())
		{
			Generator* pSourceGenerator = pCar->getSourceGenerator();
			generatorsWithCarsInGoal[pSourceGenerator]++;
			auto itUnfinishedGenerator = find(unfinishedGenerators.begin(), 
				unfinishedGenerators.end(), pSourceGenerator);

			if (generatorsWithCarsInGoal[pSourceGenerator] >= minCarsInGoalPerType && 
				itUnfinishedGenerator != unfinishedGenerators.end())
			{
				numOfUnfinishedGenerators--;
				unfinishedGenerators.erase(itUnfinishedGenerator);
			}
		}
	}
}



void Model::orderGeneratorsToStartGeneratingEvaluatedCars()
{
	for (auto& pairGenerator : generators)
		pairGenerator.second->startGeneratingCarsForEvaluation();
}



void Model::firstCarGenerated(Car* pCar)
{
	firstCars.insert(pCar);
}



int Model::createJunction(int junctionId)
{
	if (junctions.find(junctionId) == junctions.end()) // if id is unique
	{
		Junction* pJunction = new Junction();
		junctions[junctionId] = pJunction;

		return CREATE_JUNCTION_SUCCESS;
	}
	else // junction with such id already exists
		return CREATE_JUNCTION_ALREADY_EXISTS;
}



int Model::createRoundabout(int id, double length, double speedLimit)
{
	int res = CREATE_ROUNDABOUT_SUCCESS;

	if (junctions.find(id) == junctions.end()) // if id is unique
	{
		if (length >= MIN_ROUNDABOUT_LENGTH)
		{
			if (speedLimit > 0.0 && speedLimit <= MAX_ROUNDABOUT_SPEED)
			{
				Roundabout* roundabout = new Roundabout(length, speedLimit);
				junctions[id] = roundabout;
				roundabouts.push_back(roundabout);
			}
			else
				res = CREATE_ROUNDABOUT_INCORRECT_SPEED;
		}
		else
			res = CREATE_ROUNDABOUT_TOO_SHORT;
	}
	else
		res = CREATE_ROUNDABOUT_ALREADY_EXISTS;

	return res;
}



int Model::addRoundaboutOutput(int roundaboutId, double position, int junctionInputId)
{
	int res = ADD_ROUNDABOUT_OUTPUT_SUCCESS;

	Roundabout* pRoundabout = getRoundabout(roundaboutId);
	if (pRoundabout != nullptr) // roundabout exists
	{
		auto itJunctionInput = junctionInputs.find(junctionInputId);
		if (itJunctionInput != junctionInputs.end()) // gate exists
		{
			JunctionInput* pOutJInput = itJunctionInput->second;
			if (pOutJInput->getInputRoad() == nullptr) // if input is free
			{
				/*
					Proper input of a roundabout has exactly one gate
				*/
				vector<Gate*> allGates = pOutJInput->getAllGates();
				if (allGates.size() == 1)
				{
					Gate* pOutGate = allGates[0];
					if (pRoundabout->addOutput(position, pOutGate) == ADD_OUTPUT_SUCCESS)
						pOutJInput->setInputRoad(pRoundabout->getMockRoad());
					else
						res = ADD_ROUNDABOUT_OUTPUT_CANT_ADD_TO_ROUNDABOUT;
				}
				else
					res = ADD_ROUNDABOUT_OUTPUT_JUNCTION_HAS_WRONG_NUMBER_OF_GATES;
			}
			else
				res = ADD_ROUNDABOUT_OUTPUT_JUNCTION_INPUT_TAKEN;
		}
		else
			res = ADD_ROUNDABOUT_OUTPUT_NO_SUCH_JUNCTION_INPUT;
	}
	else
		res = ADD_ROUNDABOUT_OUTPUT_NO_SUCH_ROUNDABOUT;

	return res;
}



int Model::createJunctionInput(int inputId, int junctionId)
{
	if (junctionInputs.find(inputId) != junctionInputs.end()) // if input already exists
		return CREATE_JUNCTION_INPUT_ALREADY_EXISTS;

	auto itJunctions = junctions.find(junctionId);
	if (itJunctions == junctions.end()) // there is no such junction
		return CREATE_JUNCTION_INPUT_JUNCTION_NOT_FOUND;

	Junction* pJunction = itJunctions->second;
	JunctionInput* pInput = new JunctionInput();
	junctionInputs[inputId] = pInput;
	pJunction->addJunctionInput(pInput);

	return CREATE_JUNCTION_INPUT_SUCCESS;
}



int Model::commonGateCheck(int gateId, int inputId, JunctionInput** ppInput)
{
	if (gates.find(gateId) != gates.end()) // gate already exists
		return CREATE_GATE_ALREADY_EXISTS;

	auto itInputs = junctionInputs.find(inputId);
	if (itInputs == junctionInputs.end()) // junction input doesn't exist
		return CREATE_GATE_NO_SUCH_INPUT;

	*ppInput = itInputs->second;

	return CREATE_GATE_SUCCESS;
}



int Model::createAlwaysClosedGate(int gateId, int inputId)
{
	JunctionInput* pInput = nullptr;
	int res = commonGateCheck(gateId, inputId, &pInput);

	if (res == CREATE_GATE_SUCCESS)
	{
		AlwaysClosedGate* pGate = new AlwaysClosedGate();
		gates[gateId] = pGate;
		pInput->addGate(pGate);
	}

	return res;
}



int Model::createAlwaysOpenGate(int gateId, int inputId)
{
	JunctionInput* pInput = nullptr;
	int res = commonGateCheck(gateId, inputId, &pInput);
	
	if (res == CREATE_GATE_SUCCESS)
	{
		AlwaysOpenGate* pGate = new AlwaysOpenGate();
		gates[gateId] = pGate;
		pInput->addGate(pGate);
	}

	return res;
}



int Model::createCompoundGate(int gateId, int inputId,
	vector<int>& componentGatesIds)
{
	JunctionInput* pInput = nullptr;
	int res = commonGateCheck(gateId, inputId, &pInput);

	if (res == CREATE_GATE_SUCCESS)
	{
		vector<Gate*> gatesList;
		for (int componentId : componentGatesIds)
		{
			auto it = gates.find(componentId);
			if (it == gates.end()) // the component gate doesn't exist
				return CREATE_GATE_NO_SUCH_GATE;
			else
				gatesList.push_back(it->second);
		}

		if (gatesList.empty())
			return CREATE_GATE_NO_COMPONENTS;

		// check if components are not used
		for (Gate* pComponent : gatesList)
		{
			if (pComponent->getRoad() != nullptr)
				return CREATE_GATE_COMPONENT_ALREADY_IN_USE;
		}

		CompoundGate* pGate = new CompoundGate();
		for (Gate* pComponent : gatesList)
			pGate->addGate(pComponent);

		gates[gateId] = pGate;
		pInput->addGate(pGate);
	}

	return res;
}



int Model::addComponentGate(int compoundGateId, int componentGateId)
{
	int res = ADD_COMPONENT_GATE_SUCCESS;

	auto itCompoundGate = gates.find(compoundGateId);
	if (itCompoundGate != gates.end())
	{
		CompoundGate* pGate = dynamic_cast<CompoundGate*>(itCompoundGate->second);
		if (pGate != nullptr)
		{
			auto itComponentGate = gates.find(componentGateId);
			if (itComponentGate != gates.end())
			{
				Gate* pComponentGate = itComponentGate->second;
				if (pComponentGate->getRoad() == nullptr)
				{
					pComponentGate->setRoad(pGate->getRoad());
					int addRes = pGate->addGate(pComponentGate);
					if (addRes != CREATE_GATE_SUCCESS)
						res = ADD_COMPONENT_GATE_UNKNOWN_ERROR_DURING_ADDING;
				}
				else
					res = ADD_COMPONENT_GATE_ALREADY_IN_USE;
			}
			else // no component gate
				res = ADD_COMPONENT_GATE_NO_SUCH_COMPONENT;
		}
		else // the gate is not a compound gate
			res = ADD_COMPONENT_GATE_NOT_COMPOUND_GATE;
	}
	else // no such gate
		res = ADD_COMPONENT_GATE_NO_SUCH_GATE;

	return res;
}



int Model::createConflictGate(int gateId, int inputId,
	vector<vector<int>>& conflictRoadsIds, vector<double>& conflictsStarts,
	vector<double>& conflictsDurations, vector<double>& conflictsPositions)
{
	int numOfConflicts = conflictRoadsIds.size();
	int numOfStarts = conflictsStarts.size();
	int numOfDurations = conflictsDurations.size();
	int numOfPositions = conflictsPositions.size();

	if (numOfConflicts != numOfStarts || numOfStarts != numOfDurations ||
		numOfDurations != numOfPositions)
		return CREATE_GATE_CONFLICT_VECTORS_SIZES_DIFFER;

	JunctionInput* pInput = nullptr;
	int res = commonGateCheck(gateId, inputId, &pInput);

	if (res == CREATE_GATE_SUCCESS)
	{
		vector<vector<Road*>> conflictRoads;
		bool areConflictRoadsValid = getRoads(conflictRoadsIds, &conflictRoads);

		if (areConflictRoadsValid)
		{
			ConflictGate* pGate = new ConflictGate();
			int numOfConflictSets = conflictRoads.size();
			int conflictCreRes = -1;
			for (int i = 0; i < numOfConflictSets; i++)
			{
				conflictCreRes = pGate->addConflict(conflictRoads[i], conflictsStarts[i],
					conflictsDurations[i], conflictsPositions[i]);

				switch (conflictCreRes)
				{
				case ADD_CONFLICT_EMPTY_CONFLICT_ROADS: 
					res = CREATE_GATE_NO_CONFLICT_ROADS; break;
				case ADD_CONFLICT_NEGATIVE_START_POSITION: 
					res = CREATE_GATE_NEGATIVE_CONFLICT_START; break;
				case ADD_CONFLICT_NEGATIVE_REQUIRED_TIME: 
					res = CREATE_GATE_NEGATIVE_REQUIRED_DURATION; break;
				case ADD_CONFLICT_NEGATIVE_CONFLICT_POSITION: 
					res = CREATE_GATE_NEGATIVE_CONFLICT_POSITION; break;
				}

				if (res != CREATE_GATE_SUCCESS)
				{
					delete pGate;
					return res;	
				}	
			}

			gates[gateId] = pGate;
			pInput->addGate(pGate);
		}
		else
			return CREATE_GATE_CONFLICT_ROAD_NOT_FOUND;
	}

	return res;
}



int Model::createGradeSeparatedLightsGate(int gateId, int inputId, int lightsId)
{
	JunctionInput* pInput = nullptr;
	int res = commonGateCheck(gateId, inputId, &pInput);

	if (res == CREATE_GATE_SUCCESS)
	{
		auto itLights = lights.find(lightsId);
		if (itLights == lights.end()) // lisghts don't exist
			return CREATE_GATE_NO_SUCH_LIGHTS;

		Lights* pLights = itLights->second;
		GradeSeparatedLightsGate* pGate = new GradeSeparatedLightsGate(pLights);
		gates[gateId] = pGate;
		pInput->addGate(pGate);
	}

	return res;
}



int Model::createRoundaboutGate(int gateId, int roundaboutId, int inputId, double position)
{
	JunctionInput* pInput = nullptr;
	int res = commonGateCheck(gateId, inputId, &pInput);

	if (res == CREATE_GATE_SUCCESS)
	{
		Roundabout* pRoundabout = getRoundabout(roundaboutId);
		if (pRoundabout != nullptr)
		{
			RoundaboutGate* pGate = new RoundaboutGate(pRoundabout, position);
			if (pGate->connectToRoundabout())
			{
				gates[gateId] = pGate;
				pInput->addGate(pGate);
				roundaboutJunctionInputs.push_back(pInput);
			}
			else
			{
				delete pGate;
				res = CREATE_ROUNDABOUT_GATE_CANT_CONNECT;
			}
		}
		else
			res = CREATE_ROUNDABOUT_GATE_NO_SUCH_ROUNDABOUT;
	}

	return res;
}



int Model::createRoad(int roadId, vector<int>& gatesIds, int junctionInputId,
	double length, std::vector<SpeedLimit> speedLimits)
{
	Road* pRoad = nullptr;
	int res = createRoadWithNoStart(roadId, junctionInputId, length, speedLimits, &pRoad);

	if (res == CREATE_ROAD_SUCCESS)
	{
		int gateId;
		for (int i = 0; i < (int) gatesIds.size() && res == CREATE_ROAD_SUCCESS; ++i)
		{
			gateId = gatesIds[i];

			auto itGates = gates.find(gateId);
			if (itGates == gates.end()) // gate doesn't exist
				res = CREATE_ROAD_NO_SUCH_GATE;
			else
			{
				Gate* pGate = itGates->second;
				if (pGate->getRoad() != nullptr)
					res = CREATE_ROAD_GATE_ALREADY_TAKEN;
				else
					pGate->setRoad(pRoad);
			}
		}

		if (res == CREATE_ROAD_SUCCESS)
			roads[roadId] = pRoad;
		else
		{
			delete pRoad;

			auto itJInputs = junctionInputs.find(junctionInputId);
			if (itJInputs != junctionInputs.end())
				itJInputs->second->setInputRoad(nullptr);
		}
	}

	return res;
}



int Model::createRoad(int roadId, int gateId, int junctionInputId, double length,
	vector<SpeedLimit> speedLimits)
{
	vector<int> gatesIds;
	gatesIds.push_back(gateId);

	return createRoad(roadId, gatesIds, junctionInputId, length, speedLimits);
}



int Model::createExternalRoad(int roadId, int generatorId, int junctionInputId, double length, 
	vector<SpeedLimit> speedLimits)
{
	auto itGenerators = generators.find(generatorId);
	if (itGenerators == generators.end())
		return CREATE_ROAD_NO_SUCH_GENERATOR;

	Generator* pGenerator = itGenerators->second;
	if (pGenerator->getRoad() != nullptr)
		return CREATE_ROAD_GENERATOR_ALREADY_TAKEN;

	Road* pRoad = nullptr;
	int res = createRoadWithNoStart(roadId, junctionInputId, length, speedLimits, &pRoad);

	if (res == CREATE_ROAD_SUCCESS)
	{
		roads[roadId] = pRoad;
		pGenerator->setRoad(pRoad);
	}

	return res;
}



int Model::createRoadWithNoStart(int roadId, int junctionInputId, 
	double length, vector<SpeedLimit> speedLimits, Road** pCreatedRoad)
{
	if (length < MIN_ROAD_LEN)
		return CREATE_ROAD_TOO_SHORT_ROAD;

	if (roads.find(roadId) != roads.end()) // road already exists
		return CREATE_ROAD_ALREADY_EXISTS;

	auto itInputs = junctionInputs.find(junctionInputId);
	if (itInputs == junctionInputs.end())
		return CREATE_ROAD_NO_SUCH_INPUT;

	JunctionInput* pInput = itInputs->second;
	if (pInput->getInputRoad() != nullptr)
		return CREATE_ROAD_INPUT_ALREADY_TAKEN;

	RoadCreationResult creationRes = Road::getInstance(length, pInput, speedLimits);

	if (creationRes.isOk())
	{
		*pCreatedRoad = creationRes.getRoad();
		return CREATE_ROAD_SUCCESS;
	}
	else
		return processRoadCreationError(creationRes.getError());
}



int Model::createFinalRoad(int roadId, vector<int>& gatesIds, double length,
	std::vector<SpeedLimit> speedLimits)
{
	if (length < MIN_ROAD_LEN)
		return CREATE_ROAD_TOO_SHORT_ROAD;

	if (roads.find(roadId) != roads.end()) // road alerady exists
		return CREATE_ROAD_ALREADY_EXISTS;

	RoadCreationResult creationRes = Road::getInstance(length, pFinalJunctionInput, speedLimits);
	int res = CREATE_ROAD_SUCCESS;

	if (creationRes.isOk())
	{
		Road* pRoad = creationRes.getRoad();

		int gateId;
		for (int i = 0; i < (int) gatesIds.size() && res == CREATE_ROAD_SUCCESS; ++i)
		{
			gateId = gatesIds[i];

			auto itGates = gates.find(gateId);
			if (itGates == gates.end()) // gate doesn't exist
				res = CREATE_ROAD_NO_SUCH_GATE;
			else
			{
				Gate* pGate = itGates->second;
				if (pGate->getRoad() != nullptr)
					res = CREATE_ROAD_GATE_ALREADY_TAKEN;
				else
					pGate->setRoad(pRoad);
			}
		}

		if (res == CREATE_ROAD_SUCCESS)
			roads[roadId] = pRoad;
		else
			delete pRoad;
	}
	else
		return processRoadCreationError(creationRes.getError());

	return res;
}



int Model::createFinalRoad(int roadId, int gateId, double length, 
	std::vector<SpeedLimit> speedLimits)
{
	vector<int> gatesIds;
	gatesIds.push_back(gateId);
	return createFinalRoad(roadId, gatesIds, length, speedLimits);
}



bool haveTheSameSizes(vector<vector<int>>& vectors)
{
	int numOfVectors = vectors.size();
	if (numOfVectors == 0)
		return false;

	int size = vectors[0].size();

	for (int i = 1; i < numOfVectors; ++i)
	{
		if (vectors[i].size() != size)
			return false;
	}

	return true;
}



int Model::createLane(Road* pLane, int roadId, vector<int>& gatesIds, int junctionInputId)
{
	int res = CREATE_ROAD_SUCCESS;

	int gateId;
	for (int i = 0; i < (int) gatesIds.size() && res == CREATE_ROAD_SUCCESS; ++i)
	{
		gateId = gatesIds[i];

		auto itGates = gates.find(gateId);
		if (itGates == gates.end()) // gate doesn't exist
			res = CREATE_ROAD_NO_SUCH_GATE;
		else
		{
			Gate* pGate = itGates->second;
			if (pGate->getRoad() != nullptr)
				res = CREATE_ROAD_GATE_ALREADY_TAKEN;
			else
				pGate->setRoad(pLane);
		}
	}

	if (res == CREATE_ROAD_SUCCESS)
		roads[roadId] = pLane;
	else
	{
		delete pLane;

		auto itJInputs = junctionInputs.find(junctionInputId);
		if (itJInputs != junctionInputs.end())
			itJInputs->second->setInputRoad(nullptr);
	}

	return res;
}



int Model::createMultiLaneRoad(std::vector<int> lanesIds, vector<vector<int>> gatesIds,
	vector<int> junctionInputsIds, double length, vector<SpeedLimit> speedLimits)
{
	int res = CREATE_ROAD_SUCCESS;

	int numOfLanes = lanesIds.size();
	vector<Road*> lanes;

	if (lanesIds.size() == gatesIds.size() && gatesIds.size() == junctionInputsIds.size())
	{
		for (int i = 0; i < numOfLanes && res == CREATE_ROAD_SUCCESS; ++i)
		{
			Road* pLane = nullptr;
			res = createRoadWithNoStart(lanesIds[i], junctionInputsIds[i], length, speedLimits,
				&pLane);

			if (res == CREATE_ROAD_SUCCESS)
			{
				res = createLane(pLane, lanesIds[i], gatesIds[i], junctionInputsIds[i]);
				lanes.push_back(pLane);
			}

			if (res != CREATE_ROAD_SUCCESS)
			{
				// rollback already created lanes
				for (int j = 0; j < i; ++j)
				{
					deleteRoad(lanesIds[j]);
				}
			}
		}
	}
	else // different sizes of input vectors
		res = CREATE_MULTILANE_ROAD_VECTORS_SIZES_MISMATCH;

	if (res == CREATE_ROAD_SUCCESS)
	{
		// connect lanes
		for (int i = 1; i < numOfLanes; ++i)
		{
			lanes[i - 1]->setRightNeighbour(lanes[i]);
			lanes[i]->setLeftNeighbour(lanes[i - 1]);
		}
	}

	return res;
}



int Model::createMultiLaneRoad(std::vector<int> lanesIds, vector<int> gatesIds,
	vector<int> junctionInputsIds, double length, vector<SpeedLimit> speedLimits)
{
	vector<vector<int>> gatesIdsAll;
	for (int gateId : gatesIds)
	{
		vector<int> idsInner;
		idsInner.push_back(gateId);
		gatesIdsAll.push_back(idsInner);
	}

	return createMultiLaneRoad(lanesIds, gatesIdsAll, junctionInputsIds, length, speedLimits);
}



int Model::createExternalMultiLaneRoad(std::vector<int> lanesIds, std::vector<int> generatorsIds,
	std::vector<int> junctionInputsIds, double length,
	std::vector<SpeedLimit> speedLimits)
{
	int res = CREATE_ROAD_SUCCESS;

	// check if sizes match
	vector<vector<int>> vectors;
	vectors.push_back(lanesIds);
	vectors.push_back(generatorsIds);
	vectors.push_back(junctionInputsIds);

	int numOfLanes = lanesIds.size();
	vector<Road*> lanes;

	if (haveTheSameSizes(vectors))
	{
		for (int i = 0; i < numOfLanes && res == CREATE_ROAD_SUCCESS; ++i)
		{
			Road* pLane = nullptr;
			res = createExternalLane(lanesIds[i], generatorsIds[i], junctionInputsIds[i], length,
				speedLimits, &pLane);

			if (res == CREATE_ROAD_SUCCESS)
				lanes.push_back(pLane);

			if (res != CREATE_ROAD_SUCCESS)
			{
				// rollback already created lanes
				for (int j = 0; j < i; ++j)
					deleteRoad(lanesIds[j]);
			}
		}
	}
	else // different sizes of input vectors
		res = CREATE_MULTILANE_ROAD_VECTORS_SIZES_MISMATCH;

	if (res == CREATE_ROAD_SUCCESS)
	{
		// connect lanes
		for (int i = 1; i < numOfLanes; ++i)
		{
			lanes[i - 1]->setRightNeighbour(lanes[i]);
			lanes[i]->setLeftNeighbour(lanes[i - 1]);
		}
	}

	return res;
}



int Model::createExternalLane(int laneId, int generatorId, int junctionInputId, double length, 
	vector<SpeedLimit>& speedLimits, Road** ppLane)
{
	auto itGenerators = generators.find(generatorId);
	if (itGenerators == generators.end())
		return CREATE_ROAD_NO_SUCH_GENERATOR;

	Generator* pGenerator = itGenerators->second;
	if (pGenerator->getRoad() != nullptr)
		return CREATE_ROAD_GENERATOR_ALREADY_TAKEN;

	Road* pRoad = nullptr;
	int res = createRoadWithNoStart(laneId, junctionInputId, length, speedLimits, &pRoad);

	if (res == CREATE_ROAD_SUCCESS)
	{
		roads[laneId] = pRoad;
		pGenerator->setRoad(pRoad);
		*ppLane = pRoad;
	}

	return res;
}



int Model::createFinalMultiLaneRoad(vector<int> lanesIds, vector<vector<int>>& gatesIds, double length,
	vector<SpeedLimit> speedLimits)
{
	int res = CREATE_ROAD_SUCCESS;

	int numOfLanes = lanesIds.size();
	vector<Road*> lanes;

	if (lanesIds.size() == gatesIds.size())
	{
		for (int i = 0; i < numOfLanes && res == CREATE_ROAD_SUCCESS; ++i)
		{
			Road* pLane = nullptr;
			res = createFinalLane(length, lanesIds[i], gatesIds[i], speedLimits, &pLane);

			if (res == CREATE_ROAD_SUCCESS)
				lanes.push_back(pLane);

			if (res != CREATE_ROAD_SUCCESS)
			{
				// rollback already created lanes
				for (int j = 0; j < i; ++j)
				{
					deleteRoad(lanesIds[j]);
				}
			}
		}
	}
	else // different sizes of input vectors
		res = CREATE_MULTILANE_ROAD_VECTORS_SIZES_MISMATCH;

	if (res == CREATE_ROAD_SUCCESS)
	{
		// connect lanes
		for (int i = 1; i < numOfLanes; ++i)
		{
			lanes[i - 1]->setRightNeighbour(lanes[i]);
			lanes[i]->setLeftNeighbour(lanes[i - 1]);
		}
	}

	return res;
}



int Model::createFinalMultiLaneRoad(vector<int> lanesIds, vector<int>& gatesIds,
	double length, vector<SpeedLimit> speedLimits)
{
	vector<vector<int>> gatesIdsAll;
	for (int gateId : gatesIds)
	{
		vector<int> idsInner;
		idsInner.push_back(gateId);
		gatesIdsAll.push_back(idsInner);
	}

	return createFinalMultiLaneRoad(lanesIds, gatesIdsAll, length, speedLimits);
}



int Model::createFinalLane(double length, int roadId, vector<int>& gatesIds, 
	vector<SpeedLimit>& speedLimits, Road** ppLane)
{
	if (length < MIN_ROAD_LEN)
		return CREATE_ROAD_TOO_SHORT_ROAD;

	if (roads.find(roadId) != roads.end()) // road alerady exists
		return CREATE_ROAD_ALREADY_EXISTS;

	RoadCreationResult creationRes = Road::getInstance(length, pFinalJunctionInput, speedLimits);
	int res = CREATE_ROAD_SUCCESS;

	if (creationRes.isOk())
	{
		Road* pRoad = creationRes.getRoad();

		int gateId;
		for (int i = 0; i < (int) gatesIds.size() && res == CREATE_ROAD_SUCCESS; ++i)
		{
			gateId = gatesIds[i];

			auto itGates = gates.find(gateId);
			if (itGates == gates.end()) // gate doesn't exist
				res = CREATE_ROAD_NO_SUCH_GATE;
			else
			{
				Gate* pGate = itGates->second;
				if (pGate->getRoad() != nullptr)
					res = CREATE_ROAD_GATE_ALREADY_TAKEN;
				else
					pGate->setRoad(pRoad);
			}
		}

		if (res == CREATE_ROAD_SUCCESS)
		{
			roads[roadId] = pRoad;
			*ppLane = pRoad;
		}
		else
			delete pRoad;
	}
	else
		return processRoadCreationError(creationRes.getError());

	return res;
}



bool Model::deleteRoad(int id)
{
	bool res = true;
	auto itMatch = roads.find(id);
	if (itMatch != roads.end()) // road found
	{
		Road* pRoad = itMatch->second;
		// release junction input
		pRoad->getJunctionInput()->setInputRoad(nullptr);
		// release gate
		for (auto& pGate : gates)
		{
			if (pGate.second->getRoad() == pRoad)
				pGate.second->setRoad(nullptr);
		}
		// release generator if any
		for (auto& genPair : generators)
		{
			if (genPair.second->getRoad() == pRoad)
				genPair.second->setRoad(nullptr);
		}

		roads.erase(itMatch);
		delete pRoad;
	}
	else // no such road
		res = false;

	return res;
}



int Model::createSlipRoad(int roadId, vector<int>& gatesIds, int joinedRoadId, double joinPosition, 
	double length)
{
	int res = validateSlipRoad(roadId, joinedRoadId, joinPosition, length);

	length /= 2; // divide by two, because all cars are changing road at the end, but in real
				 // life the position varies. By dividing length by two the actual time spent
				 // there will be closer to average.

	if (res == CREATE_ROAD_SUCCESS)
	{
		// get the joined road
		auto itJoinedRoad = roads.find(joinedRoadId);
		Road* pJoinedRoad = itJoinedRoad->second;

		// create SlipRoadGate
		SlipRoadGate* pSlipGate = new SlipRoadGate(pJoinedRoad, joinPosition, length);
		gates[generateUniqueIdForGate()] = pSlipGate;

		// create junction input and add the gate to it
		JunctionInput* pInput = new JunctionInput();
		pInput->addGate(pSlipGate);
		junctionInputs[generateUniqueIdForJunctionInput()] = pInput;

		// create the road
		vector<SpeedLimit> speedLimits;
		SpeedLimit sl(0, length, pSlipGate->getSpeedLimit());
		speedLimits.push_back(sl);
		RoadCreationResult roadCreRes = Road::getInstance(length, pInput, speedLimits);
		if (roadCreRes.isOk())
		{
			Road* pSlipRoad = roadCreRes.getRoad();
			roads[roadId] = pSlipRoad;

			// attach to the gates
			for (int i = 0; i < (int) gatesIds.size(); ++i)
			{
				auto itGate = gates.find(gatesIds[i]);
				if (itGate == gates.end())
					return CREATE_ROAD_NO_SUCH_GATE;

				Gate* pGate = itGate->second;
				if (pGate->getRoad() != nullptr)
					res = CREATE_ROAD_GATE_ALREADY_TAKEN;
				else
					pGate->setRoad(pSlipRoad);
			}
		}
		else // error during creating the road
			res = processRoadCreationError(roadCreRes.getError());
	}

	return res;
}



int Model::validateSlipRoad(int roadId, int joinedRoadId, double joinPosition, 
	double length)
{
	// check if road id is unique
	auto itRoad = roads.find(roadId);
	if (itRoad != roads.end()) // road already exists
		return CREATE_ROAD_ALREADY_EXISTS;

	// check if joined road exists
	auto itJoinedRoad = roads.find(joinedRoadId);
	if (itJoinedRoad == roads.end()) // road doesn't exist
		return CREATE_SLIP_ROAD_NO_SUCH_JOINED_ROAD;

	// check if join position is correct
	if (joinPosition < 0)
		return CREATE_SLIP_ROAD_JOIN_POSITION_NEGATIVE;

	Road* pJoinedRoad = itJoinedRoad->second;
	if (joinPosition + length > pJoinedRoad->getLength())
		return CREATE_SLIP_ROAD_JOIN_POSITION_TOO_BIG;

	// check if length correct
	if (length < MIN_SLIP_ROAD_LEN)
		return CREATE_SLIP_ROAD_TOO_SHORT;

	return CREATE_ROAD_SUCCESS;
}



int Model::generateUniqueIdForGate()
{
	int id = MAX_ALLOWED_USER_DEFINED_ID + 1;
	while (gates.find(id) != gates.end())
		++id;

	return id;
}



int Model::generateUniqueIdForJunctionInput()
{
	int id = MAX_ALLOWED_USER_DEFINED_ID;
	while (junctionInputs.find(id) != junctionInputs.end())
		++id;

	return id;
}



int Model::generateUniqueGlobalIdForGenerator()
{
	int id = 1;
	while (allGenerators.find(id) != allGenerators.end())
		++id;

	return id;
}



int Model::createGenerator(int generatorId, string& generatorType,
	vector<string>& params, int* pGlobalId)
{
	auto itGenerators = generators.find(generatorId);
	if (itGenerators != generators.end())
		return CREATE_GENERATOR_ALREADY_EXISTS;

	Generator* pGenerator = Generator::obtainFromString(generatorType, params);

	if (pGenerator != nullptr)
	{
		generators[generatorId] = pGenerator;
		vecGenerators.push_back(pGenerator);
		pGenerator->registerFirstGenerationListener(this);
		pGenerator->setActiveCarsInModel(&activeCars);
		generatorsWithCarsInGoal[pGenerator] = 0;
		unfinishedGenerators.push_back(pGenerator);
		numOfUnfinishedGenerators++;

		// add to all generators
		int globalId = generateUniqueGlobalIdForGenerator();
		allGenerators[globalId] = pGenerator;
		if (pGlobalId != nullptr)
			*pGlobalId = globalId;
	}
	else
		return CREATE_GENERATOR_PARSING_ERROR;
	
	return CREATE_GENERATOR_SUCCESS;
}



int Model::attachToGenerator(int generatorId, string& attachedGeneratorType,
	vector<string>& attachedGeneratorParams, int* pGlobalId)
{
	auto itGenerators = generators.find(generatorId);
	if (itGenerators == generators.end())
		return ATTACH_TO_GENERATOR_NO_SUCH_GENERATOR;

	Generator* pAttachedGenerator = Generator::obtainFromString(attachedGeneratorType,
		attachedGeneratorParams);

	if (pAttachedGenerator != nullptr)
	{
		Generator* pGenerator = itGenerators->second;
		Road* pRoad = pGenerator->getRoad();
		pAttachedGenerator->setRoad(pRoad);
		pAttachedGenerator->setNextGenerator(pGenerator);
		generators[generatorId] = pAttachedGenerator;
		pAttachedGenerator->registerFirstGenerationListener(this);
		pAttachedGenerator->setActiveCarsInModel(&activeCars);
		generatorsWithCarsInGoal[pAttachedGenerator] = 0;
		unfinishedGenerators.push_back(pAttachedGenerator);
		numOfUnfinishedGenerators++;

		// swap in vecGenerators
		auto itFoundGenerator = find(vecGenerators.begin(), vecGenerators.end(), pGenerator);
		vecGenerators.erase(itFoundGenerator);
		vecGenerators.push_back(pAttachedGenerator);

		// add to all generators
		int globalId = generateUniqueGlobalIdForGenerator();
		allGenerators[globalId] = pAttachedGenerator;
		if (pGlobalId != nullptr)
			*pGlobalId = globalId;

		return ATTACH_TO_GENERATOR_SUCCESS;
	}
	else
		return ATTACH_TO_GENERATOR_PARSING_ERROR;
}



int Model::createLightsSystem(int lightsSystemId, int junctionId)
{
	if (lightsSystems.find(lightsSystemId) != lightsSystems.end())
		return CREATE_LIGHTS_SYSTEM_ALREADY_EXISTS;

	auto itJunctions = junctions.find(junctionId);
	if (itJunctions == junctions.end())
		return CREATE_LIGHTS_SYSTEM_NO_SUCH_JUNCTION;

	LightsSystem* pSystem = new LightsSystem();
	lightsSystems[lightsSystemId] = pSystem;
	vecLightsSystems.push_back(pSystem);
	lightsSystemsMapping[lightsSystemId] = lightsSystems.size() - 1;

	Junction* pJunction = itJunctions->second;
	pJunction->setLightsSystem(pSystem);
	
	return CREATE_LIGHTS_SYSTEM_SUCCESS;
}



int Model::createLights(int lightsId, int lightsSystemId)
{
	if (lights.find(lightsId) != lights.end()) // lights aleardy exist
		return CREATE_LIGHTS_ALREADY_EXISTS;

	auto itLightsSystems = lightsSystems.find(lightsSystemId);
	if (itLightsSystems == lightsSystems.end()) // no such lights sytem
		return CREATE_LIGHTS_NO_SUCH_LIGHTS_SYSTEM;

	Lights* pLights = new Lights();
	LightsSystem* pSystem = itLightsSystems->second;
	pSystem->addLights(pLights);

	lights[lightsId] = pLights;

	return CREATE_LIGHTS_SUCCESS;
}



int Model::createLightsGroup(int lightsGroupId, int lightsSystemId,
	vector<int>& greenLightsIds, vector<int>& redLightsIds, double duration,
	double minDuration,	double maxDuration)
{
	if (duration < minDuration || duration < 0.0)
		return CREATE_LIGHTS_GROUP_TOO_SMALL_DURATION;

	if (lightsGroups.find(lightsGroupId) != lightsGroups.end())
		return CREATE_LIGHTS_GROUP_ALREADY_EXISTS;

	auto itLightsSystems = lightsSystems.find(lightsSystemId);
	if (itLightsSystems == lightsSystems.end())
		return CREATE_LIGHTS_GROUP_NO_SUCH_LIGHTS_SYSTEM;

	LightsSystem* pLightsSystem = itLightsSystems->second;

	// check if there is no light which is in green and red at the same time
	if (!areGreenAndRedLightsDisjoint(greenLightsIds, redLightsIds))
		return CREATE_LIGHTS_GROUP_LIGHT_IN_GREEN_AND_RED_AT_THE_SAME_TIME;

	removeDuplicates(greenLightsIds);
	removeDuplicates(redLightsIds);

	vector<Lights*> green;
	vector<Lights*> red;
	getLightsWithIds(greenLightsIds, &green);
	getLightsWithIds(redLightsIds, &red);

	// checks if all lights are from the lights system
	if (!areLightsFromTheLightsSystem(pLightsSystem, green))
		return CREATE_LIGHTS_GROUP_LIGHTS_FROM_MULTIPLE_SYSTEMS;

	if (!areLightsFromTheLightsSystem(pLightsSystem, red))
		return CREATE_LIGHTS_GROUP_LIGHTS_FROM_MULTIPLE_SYSTEMS;

	LightsGroup* pGroup = new LightsGroup(duration, minDuration, maxDuration);
	
	for (Lights* pGreen : green)
		pGroup->addGreenLights(pGreen);

	for (Lights* pRed : red)
		pGroup->addRedLights(pRed);

	if (pLightsSystem->addLightsGroup(pGroup))
	{
		lightsGroups[lightsGroupId] = pGroup;
		lightsGroupsMapping[lightsGroupId] = lightsGroups.size() - 1;
		return CREATE_LIGHTS_GROUP_SUCCESS;
	}
	else
	{
		delete pGroup;
		return CREATE_LIGHTS_GROUP_NOT_ALL_LIGHTS_COVERED;
	}
}



int Model::setTemplateCar(int generatorId, Car* pTemplateCar)
{
	return setTemplateCarGeneral(generatorId, pTemplateCar, generators);
}



int Model::setTemplateCarByGlobalId(int generatorGlobalId, Car* pTemplateCar)
{
	return setTemplateCarGeneral(generatorGlobalId, pTemplateCar, allGenerators);
}



int Model::setTemplateCarGeneral(int generatorId, Car* pTemplateCar,
	unordered_map<int, Generator*>& collection)
{
	if (pTemplateCar == nullptr)
		return SET_TEMPLATE_CAR_NULL_CAR;

	auto itFoundGenerator = collection.find(generatorId);
	if (itFoundGenerator == collection.end())
		return SET_TEMPLATE_CAR_NO_SUCH_GENERATOR;

	Generator* pGenerator = itFoundGenerator->second;
	bool setSuccess = pGenerator->setGeneratedCarTemplate(pTemplateCar);

	if (setSuccess)
		return SET_TEMPLATE_CAR_SUCCESS;
	else
		return SET_TEMPLATE_CAR_UNSPECIFIED_ROAD;
}



int Model::createRoute(int id, std::vector<int>& idsOfRoadsInRoute)
{
	// check if id is unique
	auto itRoute = routes.find(id);
	if (itRoute != routes.end())
		return CREATE_ROUTE_NON_UNIQUE_ID;

	vector<Road*>* pRoute = obtainRoute(idsOfRoadsInRoute);
	
	if (pRoute != nullptr)
	{
		routes[id] = pRoute;
		routesOptimalTimes[pRoute] = getRouteOptimalTime(pRoute);
		return CREATE_ROUTE_SUCCESS;
	}
	else
		return CREATE_ROUTE_WRONG_ROUTE;
}



vector<Road*>* Model::obtainRoute(vector<int>& idsOfRoadsInRoute)
{
	int routeLen = idsOfRoadsInRoute.size();
	if (routeLen < 1)
		return nullptr;

	vector<Road*>* pRoute = new vector<Road*>();
	for (int roadId : idsOfRoadsInRoute)
	{
		auto it = roads.find(roadId);
		if (it == roads.end())
		{
			delete pRoute;
			return nullptr;
		}
		else
			pRoute->push_back(it->second);
	}

	// check if the first road in the route is external (starts in a generator)
	if (!isExternalRoad(pRoute->front()))
	{
		delete pRoute;
		return nullptr;
	}

	// check if the last road is connected to the final junction input
	if (pRoute->back()->getJunctionInput() != pFinalJunctionInput)
	{
		delete pRoute;
		return nullptr;
	}

	// check if the roads are continuous and add roundabout roads
	Road* pPrevRoad = pRoute->front();
	Road* pCurrRoad = nullptr;
	JunctionInput* pInput = nullptr;
	RoundaboutGate* pRoundaboutGate = nullptr;
	Road* pRoundaboutRoad = nullptr;
	for (int i = 1; i < (int) pRoute->size(); i++)
	{
		pCurrRoad = pRoute->at(i);
		if (!pPrevRoad->isRoadAccessible(pCurrRoad))
		{
			/*
				If the road was not found, then check if there is no roundabout, because roundabout
				roads are not included int the roads ids. If so, then add that road and proceed
			*/
			pInput = pPrevRoad->getJunctionInput();
			if (find(roundaboutJunctionInputs.begin(), roundaboutJunctionInputs.end(), pInput) != 
				roundaboutJunctionInputs.end())
			{
				// roundabout encountered
				pRoundaboutGate = (RoundaboutGate*) pInput->getAllGates().front();
				pRoundaboutRoad = pRoundaboutGate->getRoad();
				if (pRoundaboutRoad != nullptr)
				{
					pRoute->insert(pRoute->begin() + i, pRoundaboutRoad);
					++i;
					// check if the next road is accessible from the roundabout
					Roundabout* pRoundabout = pRoundaboutGate->getRoundabout();
					if (pRoundabout->getOutputGate(pCurrRoad) == nullptr)
					{
						delete pRoute;
						return nullptr;
					}
				}
				else
				{
					delete pRoute;
					return nullptr;
				}
			}
			else // no connection
			{
				delete pRoute;
				return nullptr;
			}
		}

		pPrevRoad = pCurrRoad;
	}

	return pRoute;
}



vector<ModelWarning> Model::findWarnings()
{
	vector<ModelWarning> warnings;

	findWarningsInJunctions(warnings);
	findWarningsInRoundabouts(warnings);
	findWarningsInJunctionInputs(warnings);
	findWarningsInRoads(warnings);
	findWarningsInGenerators(warnings);
	findWarningsInLightsSystems(warnings);
	findWarningsInLights(warnings);
	findWarningsInLightsGroups(warnings);
	findWarningsInGates(warnings);

	return warnings;
}



void Model::findWarningsInJunctions(vector<ModelWarning>& warnings)
{
	for (auto& junctionPair : junctions)
	{
		if (!junctionPair.second->hasInputs())
			warnings.push_back(ModelWarning(WARNING_NO_JUNCTION_INPUT, ELEMENT_JUNCTION, 
				junctionPair.first));
	}
}



void Model::findWarningsInRoundabouts(vector<ModelWarning>& warnings)
{
	Road* pMockRoad;
	int id = -1;
	for (Roundabout* pRoundabout : roundabouts)
	{
		// only roundabout gate can have access to the roundabout road, so in order to check if
		// the roundabout input has a roundabout gate is checking if the input has access to the
		// road
		pMockRoad = pRoundabout->getMockRoad();
		bool hasInput = false;
		id = getJunctionId(pRoundabout);
		for (JunctionInput* pInput : roundaboutJunctionInputs)
		{
			// check if input has a roundabout gate
			hasInput = true;
			if (pInput->gateToRoad(pMockRoad) == nullptr)
			{
				warnings.push_back(ModelWarning(WARNING_ROUNDABOUT_WITH_NO_ROUNDABOUT_GATE, 
					ELEMENT_JUNCTION, id));
			}
		}

		// check if has an input in roundabouts inputs
		if (!hasInput)
		{
			warnings.push_back(ModelWarning(WARNING_ROUNDABOUT_WITH_NO_ROUNDABOUT_INPUT, 
				ELEMENT_JUNCTION, id));
		}

		// check if hast at least one output
		if (!pRoundabout->hasOutputs())
		{
			warnings.push_back(ModelWarning(WARNING_ROUNDABOUT_WITH_NO_OUTPUT, 
				ELEMENT_JUNCTION, id));
		}
	}
}



void Model::findWarningsInJunctionInputs(vector<ModelWarning>& warnings)
{
	for (auto& pairInput : junctionInputs)
	{
		if (!pairInput.second->hasGates())
		{
			warnings.push_back(ModelWarning(WARNING_JUNCTION_INPUT_WITH_NO_GATE, 
				ELEMENT_JUNCTION_INPUT, pairInput.first));
		}
	}
}



void Model::findWarningsInRoads(vector<ModelWarning>& warnings)
{
	unordered_set<Road*> notReferenced;
	// initially add all roads to notReferenced
	for (auto& pairRoad : roads)
		notReferenced.insert(pairRoad.second);

	// remove from notReferenced all roads which were referenced in at least one route
	Road* pRoad;
	vector<Road*>* pRoads;
	int size;
	for (auto& pairRoute : routes)
	{
		pRoads = pairRoute.second;
		size = pRoads->size();
		for (int i = 0; i < size; ++i)
		{
			pRoad = pRoads->at(i);
			notReferenced.erase(pRoad);
		}
	}

	// now in notReferenced there are only roads which are not covered by any route
	int id;
	for (Road* pNotRef : notReferenced)
	{
		id = getRoadId(pNotRef);
		warnings.push_back(ModelWarning(WARNING_ROAD_NOT_COVERED_BY_ROUTE, 
			ELEMENT_ROAD, id));
	}
}



void Model::findWarningsInGenerators(vector<ModelWarning>& warnings)
{
	for (auto& pairGenerator : generators)
	{
		if (pairGenerator.second->getRoad() == nullptr)
		{
			warnings.push_back(ModelWarning(WARNING_GENERATOR_WITH_NO_ROAD, 
				ELEMENT_GENERATOR, pairGenerator.first));
		}
	}
}



void Model::findWarningsInLightsSystems(vector<ModelWarning>& warnings)
{
	LightsSystem* pSystem;
	int id;
	for (auto& pairLightsSystem : lightsSystems)
	{
		pSystem = pairLightsSystem.second;
		id = pairLightsSystem.first;

		// lights
		if (!pSystem->hasLights())
			warnings.push_back(ModelWarning(WARNING_LIGHTS_SYSTEM_WITH_NO_LIGHTS, 
				ELEMENT_LIGHTS_SYSTEM, id));

		// lights groups
		if (!pSystem->hasLightsGroups())
			warnings.push_back(ModelWarning(WARNING_LIGHTS_SYSTEM_NO_LIGHTS_GROUPS, 
				ELEMENT_LIGHTS_SYSTEM, id));
	}
}



void Model::findWarningsInLights(vector<ModelWarning>& warnings)
{
	Lights* pLights;
	int id;
	for (auto& pairLights : lights)
	{
		id = pairLights.first;
		pLights = pairLights.second;

		if (!isInGreenLightsGroup(pLights))
			warnings.push_back(ModelWarning(WARNING_LIGHTS_NOT_IN_GREEN_GROUP, 
				ELEMENT_LIGHTS, id));
	}
}



bool Model::isInGreenLightsGroup(Lights* pLights)
{
	for (auto& pairLightsGroup : lightsGroups)
	{
		if (pairLightsGroup.second->hasGreenLights(pLights))
			return true;
	}

	return false;
}



void Model::findWarningsInLightsGroups(vector<ModelWarning>& warnings)
{
	for (auto& pairLightsGroup : lightsGroups)
	{
		if (!pairLightsGroup.second->hasLights())
		{
			warnings.push_back(ModelWarning(WARNING_LIGHTS_GROUP_WITH_NO_LIGHTS, 
				ELEMENT_LIGHTS_GROUP, pairLightsGroup.first));
		}
	}
}



void Model::findWarningsInGates(vector<ModelWarning>& warnings)
{
	for (auto& pairGate : gates)
	{
		if (pairGate.second->getRoad() == nullptr)
		{
			warnings.push_back(ModelWarning(WARNING_GATE_WITH_NO_ROAD, 
				ELEMENT_GATE, pairGate.first));
		}
	}
}



// utils //////////////////////////////////////////////////////////////////////////////////////////



void Model::removeDuplicates(std::vector<int> vec)
{
	vector<int>::iterator itr = vec.begin();
	unordered_set<int> passed;

	for (auto curr = vec.begin(); curr != vec.end(); ++curr) 
	{
		if (passed.insert(*curr).second)
			*itr++ = *curr;
	}

	vec.erase(itr, vec.end());
}



bool Model::areGreenAndRedLightsDisjoint(std::vector<int>& greenLightsIds,
	std::vector<int> redLightsIds)
{
	for (int green : greenLightsIds)
	{
		for (int red : redLightsIds)
		{
			if (green == red)
				return false;
		}
	}

	return true;
}



bool Model::areLightsFromTheLightsSystem(LightsSystem* pLightsSystem,
	std::vector<Lights*>& lights)
{
	for (Lights* l : lights)
	{
		if (!pLightsSystem->hasLights(l))
			return false;
	}

	return true;
}



bool Model::isExternalRoad(Road* pRoad)
{
	for (auto& pairGenerator : generators)
	{
		if (pairGenerator.second->getRoad() == pRoad)
			return true;
	}

	return false;
}



int Model::processRoadCreationError(int error)
{
	switch (error)
	{
	case RoadCreationResult::NOT_CONTINUOUS_SPEED_LIMITS:
		return CREATE_ROAD_NOT_CONTINUOUS_SPEED_LIMITS;
	case RoadCreationResult::NOT_ENOUGH_SPEED_LIMITS:
		return CREATE_ROAD_NOT_ENOUGH_SPEED_LIMITS;
	case RoadCreationResult::NOT_WHOLE_ROAD_COVERED:
		return CREATE_ROAD_NOT_WHOLE_ROAD_COVERED;
	default:
		return CREATE_ROAD_UNKNOWN_ERROR;
	}
}



// getters & setters //////////////////////////////////////////////////////////////////////////////
unordered_map<int, Junction*>& Model::getJunctions()
{
	return junctions;
}



unordered_map<int, JunctionInput*>& Model::getJunctionInputs()
{
	return junctionInputs;
}



unordered_map<int, Gate*>& Model::getGates()
{
	return gates;
}



unordered_map<int, LightsSystem*>& Model::getLightsSystems()
{
	return lightsSystems;
}



unordered_map<int, Lights*>& Model::getLights()
{
	return lights;
}



unordered_map<int, LightsGroup*>& Model::getLightsGroups()
{
	return lightsGroups;
}



unordered_map<int, Generator*>& Model::getGenerators()
{
	return generators;
}



unordered_map<int, Road*>& Model::getRoads()
{
	return roads;
}



unordered_map<int, std::vector<Road*>*>& Model::getRoutes()
{
	return routes;
}



double Model::getAvgTime()
{
	double totalTime = 0.0;
	int numOfCars = 0;

	for (Car* pCar : finishedCars)
	{
		if (pCar->isEvaluated())
		{
			totalTime += pCar->getTotalTime();
			numOfCars++;
		}
	}

	if (numOfCars > 0)
		return totalTime / numOfCars;
	else
		return DBL_MAX;
}



double Model::getAdditionalTime()
{
	double realTime = 0.0;
	double optimalTime = 0.0;
	for (Car* pFCar : finishedCars)
	{
		optimalTime += routesOptimalTimes[pFCar->getRoute()];
		realTime += pFCar->getTotalTime();
	}

	return realTime - optimalTime;
}



double Model::getRouteOptimalTime(vector<Road*>* pRoute)
{
	double optiTime = 0.0;
	if (pRoute != nullptr)
	{
		for (Road* pRoad : *pRoute)
			optiTime += pRoad->getOptimalTime();
	}

	return optiTime;
}



int Model::getNumOfReadyCars()
{
	int numOfReadyCars = 0;
	for (Generator* pGenerator : vecGenerators)
		numOfReadyCars += pGenerator->getNumOfReadyCars();

	return numOfReadyCars;
}



int Model::getNumOfFinishedCars()
{
	int numOfCars = 0;

	for (Car* pCar : finishedCars)
	{
		if (pCar->isEvaluated())
			numOfCars++;
	}

	return numOfCars;
}



void Model::getRanges(pair<double, double>* ranges)
{
	double min;
	double max;
	int mappedIdx; // position of a range in the ranges vector

	// first add lights groups minimal and maximal durations
	int groupIdx;
	for (auto& pairLightsGroup : lightsGroups)
	{
		groupIdx = pairLightsGroup.first;
		mappedIdx = lightsGroupsMapping[groupIdx];
		min = pairLightsGroup.second->getMinDuration();
		max = pairLightsGroup.second->getMaxDuration();
		ranges[mappedIdx] = pair<double, double>(min, max);
	}

	// second add offset ranges
	min = 0.0;
	int lightsSystemIdx;
	for (auto& pairLightsSystem : lightsSystems)
	{
		lightsSystemIdx = pairLightsSystem.first;
		mappedIdx = lightsSystemsMapping[lightsSystemIdx] + lightsGroups.size();
		max = pairLightsSystem.second->getTotalMaxLightsGroupsDuration();
		ranges[mappedIdx] = pair<double, double>(min, max);
	}
}



int Model::getProblemSize()
{
	return lightsGroups.size() + lightsSystems.size();;
}



Roundabout* Model::getRoundabout(int id)
{
	Junction* pJunction = junctions[id];
	for (Roundabout* pRoundabout : roundabouts)
	{
		if (pRoundabout == pJunction)
			return pRoundabout;
	}

	return nullptr;
}



bool Model::getRoads(vector<vector<int>>& conflictRoadsIds, vector<vector<Road*>>* pRes)
{
	for (auto conflictSet : conflictRoadsIds)
	{
		vector<Road*> conflictRoads;
		for (auto conflictRoad : conflictSet)
		{
			auto it = roads.find(conflictRoad);
			if (it == roads.end()) // if the road doesn't exist
				return false;
			else
				conflictRoads.push_back(it->second);
		}
		pRes->push_back(conflictRoads);
	}

	return true;
}



vector<Road*>* Model::getRoute(int id)
{
	auto itRoute = routes.find(id);

	if (itRoute != routes.end())
		return itRoute->second;
	else
		return nullptr;
}



int Model::getJunctionId(Junction* pJunction, bool* pFound)
{
	for (auto& pairJunction : junctions)
	{
		if (pairJunction.second == pJunction)
		{
			if (pFound != nullptr)
				*pFound = true;

			return pairJunction.first;
		}
	}

	if (pFound != nullptr)
		*pFound = false;

	return -1;
}



int Model::getRoadId(Road* pRoad, bool* pFound)
{
	for (auto& pairRoad : roads)
	{
		if (pairRoad.second == pRoad)
		{
			if (pFound != nullptr)
				*pFound = true;

			return pairRoad.first;
		}
	}

	if (pFound != nullptr)
		*pFound = false;

	return -1;
}



void Model::getLightsWithIds(vector<int>& lightsIds, vector<Lights*>* pRes)
{
	int i = 0;
	for (auto pairLights : lights)
	{
		for (int lightsId : lightsIds)
		{
			if (pairLights.first == lightsId)
				pRes->push_back(pairLights.second);
		}
	}
}



