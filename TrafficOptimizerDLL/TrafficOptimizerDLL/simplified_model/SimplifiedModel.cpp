#include "../pch.h"
#include "SimplifiedModel.h"
#include <list>
#include "SimplifiedModelLoader.h"
#include <chrono>



using namespace std;



// constructors ///////////////////////////////////////////////////////////////////////////////////



SimplifiedModel::SimplifiedModel(vector<SimplifiedGenerator*>& generators, vector<SimplifiedRoad*>& roads,
	vector<SimplifiedLightsSystem*>& lightsSystems, vector<SimplifiedLightsGroup*>& lightsGroups,
	vector<vector<SimplifiedRoad*>*>& routes, SimplifiedRoad* pFinalRoad)
{
	this->generators = generators;
	for (SimplifiedGenerator* pGenerator : generators)
		pGenerator->setModel(this);

	this->lightsSystems = lightsSystems;
	this->lightsGroups = lightsGroups;
	this->roads = roads;
	this->routes = routes;
	this->idxOfFreeCar = 0;
	this->problemSize = lightsGroups.size() + lightsSystems.size();
	this->isRunning = false;
	this->pFinalRoad = pFinalRoad;
}



SimplifiedModel::~SimplifiedModel()
{
	for (SimplifiedLightsSystem* pSystem : lightsSystems)
	{
		/*
			it's like that because otherwise it would be hard to make a proper clean up 
			after failed load
		*/
		pSystem->deleteLightsAndLightsGroups();
		delete pSystem;
	}

	for (SimplifiedRoad* pRoad : roads)
		delete pRoad;

	for (SimplifiedGenerator* pGenerator : generators)
		delete pGenerator;

	for (SimplifiedCar* pCar : carPool)
		delete pCar;

	for (vector<SimplifiedRoad*>* pRoute : routes)
		delete pRoute;

	if (pFinalRoad != nullptr)
		delete pFinalRoad;

	for (SimplifiedLights* pFreeLights : freeLights)
		delete pFreeLights;
}



// methods ////////////////////////////////////////////////////////////////////////////////////////



int SimplifiedModel::run(double tickDuration, double timeoutSeconds)
{
	// add all roads to unfinished
	int numOfRoads = roads.size();
	/*
		Contains roads which didn't have any car yet. Once it is empty all cars from pFinalRoad
		will be erased and from now on cars for statistics will be stored in pFinalRoad. It is like
		that, because otherwise statistics wouldn't be calculated only on cars from the period when 
		the	model is under full load.
	*/
	unordered_set<SimplifiedRoad*> unreachedRoads;
	/*
		Initially it contains all roads. Once road's car counter reaches CARS_PER_ROAD_TO_FINISH
		the road is deleted from this set. If the set is empty the function finishes.
	*/
	unordered_set<SimplifiedRoad*> unfinishedRoads;
	for (int i = 0; i < numOfRoads; ++i)
	{
		unreachedRoads.insert(roads[i]);
		unfinishedRoads.insert(roads[i]);
	}

	int numOfGenerators = generators.size();
	int numOfLightsSystems = lightsSystems.size();
	double currTime = 0.0;
	isRunning = true;
	auto start = chrono::steady_clock::now(); // for timeout
	chrono::milliseconds timeout(0);
	if (timeoutSeconds > 0) // if timeout <= 0 then no timeout is used
		timeout = chrono::milliseconds((long)(timeoutSeconds * 1000));
	// run until certain number of cars went through all roads or until stop() is called
	while (isRunning)
	{
		// tick elements
		for (int i = 0; i < numOfLightsSystems; ++i)
			lightsSystems[i]->tick(currTime);

		for (int i = 0; i < numOfGenerators; ++i)
			generators[i]->tick(currTime);

		for (int i = 0; i < numOfRoads; ++i)
		{
			roads[i]->tick(currTime);
			// check if road was reached and if so then delete it from unreachedRoads
			if (!unreachedRoads.empty()) // if there is unreached road
			{
				if (roads[i]->getCarsCounter() > 0) // if current road was reached
					unreachedRoads.erase(roads[i]); // if it was already reached nothing will hapen
				if (unreachedRoads.empty())
				{
					pFinalRoad->reset();
					for (int i = 0; i < numOfRoads; ++i) // reset counters
						roads[i]->resetCarsCounter();
				}
			} // all roads reached -> check if road "finished"
			else if (roads[i]->getCarsCounter() >= CARS_PER_ROAD_TO_FINISH)
			{
				unfinishedRoads.erase(roads[i]);
			}
		}

		if (unfinishedRoads.empty())
			isRunning = false;

		currTime += tickDuration;

		if (timeoutSeconds > 0) // consider timeout only if it is greater than 0
		{
			if (chrono::steady_clock::now() - start > timeout)
				return RUN_TIME_OUT;
		}
	}

	return RUN_SUCCESS;
}



double SimplifiedModel::getAvgTime()
{
	double totalTime = 0.0;
	list<SimplifiedCar*>& finishedCars = pFinalRoad->getCars(); // all finished cars are stored on pFinalRoad
	for (SimplifiedCar* pCar : finishedCars)
		totalTime += pCar->getTotalTime();

	return totalTime / (double)finishedCars.size();
}



double SimplifiedModel::getAdditionalTime()
{
	double additionalTime = 0.0;
	for (SimplifiedRoad* pRoad : roads)
		additionalTime += pRoad->getAdditionalTime();

	return additionalTime;
}



int SimplifiedModel::getNumOfFinishedCars()
{
	return pFinalRoad->getNumOfCars(); // all finished cars are on pFinalRoad
}



int SimplifiedModel::getNumOfReadyCars()
{
	int totReadyCars = 0;
	for (SimplifiedGenerator* pGenerator : generators)
		totReadyCars += pGenerator->getNumOfReadyCars();

	return totReadyCars;
}



void SimplifiedModel::getRanges(pair<double, double>* ranges)
{
	int numOfLightsGroups = lightsGroups.size();
	for (int i = 0; i < numOfLightsGroups; ++i)
	{
		ranges[i] = pair<double, double>(lightsGroups[i]->getMinDuration(), 
			lightsGroups[i]->getMaxDuration());
	}

	int numOfLightsSystems = lightsSystems.size();
	for (int i = 0; i < numOfLightsSystems; ++i)
	{
		ranges[numOfLightsGroups + i] = pair<double, double>(0.0, 
			lightsSystems[i]->getMaxTotalDuration());
	}
}



int SimplifiedModel::getProblemSize()
{
	return lightsGroups.size() + lightsSystems.size();
}



void SimplifiedModel::translate(double* individual, std::unordered_map<int, double>& groups,
	std::unordered_map<int, double>& systems)
{
	int idx = 0;
	for (SimplifiedLightsGroup* pGroup : lightsGroups)
	{
		groups[pGroup->getId()] = individual[idx];
		++idx;
	}

	for (SimplifiedLightsSystem* pSystem : lightsSystems)
	{
		systems[pSystem->getId()] = individual[idx];
		++idx;
	}
}



SimplifiedCar* SimplifiedModel::getCarInstance(SimplifiedCar* pTemplate)
{
	SimplifiedCar* pCar;
	if (idxOfFreeCar < (int) carPool.size()) // if there are available cars in carPool
	{
		// assign a car from carPool
		pCar = carPool[idxOfFreeCar];
		pCar->reset();
		pCar->setRoute(pTemplate->getRoute());
	}
	else // no available cars in carPool
	{
		// create a new car and add it to the carPool
		pCar = new SimplifiedCar(pTemplate->getRoute());
		carPool.push_back(pCar);
	}

	++idxOfFreeCar;
	return pCar;
}



bool SimplifiedModel::update(double* newValues)
{
	reset();

	int lightsGroupsSize = lightsGroups.size();
	for (int i = 0; i < lightsGroupsSize; ++i)
		lightsGroups[i]->setDuration(newValues[i]);

	int lightsSystemsSize = lightsSystems.size();
	for (int i = 0; i < lightsSystemsSize; ++i)
		lightsSystems[i]->setPhase(newValues[lightsGroupsSize + i]);

	return true;
}



void SimplifiedModel::reset()
{
	int numOfLightsSystems = lightsSystems.size();
	for (int i = 0; i < numOfLightsSystems; ++i)
		lightsSystems[i]->reset();

	int numOfRoads = roads.size();
	for (int i = 0; i < numOfRoads; ++i)
		roads[i]->reset();

	int numOfGenerators = generators.size();
	for (int i = 0; i < numOfGenerators; ++i)
		generators[i]->reset();

	idxOfFreeCar = 0;

	pFinalRoad->reset();

	isRunning = false;
}



void SimplifiedModel::addFreeLights(SimplifiedLights* pLights)
{
	freeLights.push_back(pLights);
}