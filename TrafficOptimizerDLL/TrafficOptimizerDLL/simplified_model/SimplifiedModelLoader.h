#pragma once
#include <string>
#include "../xml/tinyxml2.h"
#include <vector>
#include <unordered_map>
#include "SimplifiedLights.h"
#include "SimplifiedLightsSystem.h"
#include "SimplifiedRoad.h"
#include "SimplifiedGenerator.h"



class SimplifiedModelLoader
{
public:
	// const //////////////////////////////////////////////////////////////////////////////////////
	static const int LOAD_SUCCESS = 0;
	static const int LOAD_ERR_INVALID_FILE = 1;
	static const int LOAD_ERR_MISSING_ATTRIBUTE = 2;
	static const int LOAD_ERR_WRONG_ATTRIBUTE = 3;
	static const int LOAD_ERR_NON_UNQ_ID = 4;
	static const int LOAD_ERR_GREEN_AND_RED_NOT_SPECIFIED = 5;
	static const int LOAD_ERR_SECTION_NOT_SPECIFIED = 6;
	static const int LOAD_ERR_NOT_ALL_LIGHTS_IN_LIGHTS_GROUP = 7;
	static const int LOAD_ERR_LIGHTS_IN_GREEN_AND_RED = 8;
	static const int LOAD_ERR_NO_SUCH_LIGHTS = 9;
	static const int LOAD_ERR_REFERENCED_ITEM_DOES_NOT_EXIST = 10;
	static const int LOAD_ERR_EMPTY_LIST = 11;
	static const int LOAD_ERR_NOT_ALL_ROUTES_ASSOCIATIED_WITH_GENERATOR = 12;
	static const int LOAD_ERR_INVALID_ROUTE = 13;
	static const int LOAD_ERR_UNCOVERED_ROADS = 14;
	static const int LOAD_ERR_LIGHTS_FROM_DIFFERENT_SYSTEM = 15;
	static const int LOAD_ERR_THE_SAME_ROAD_REFERENCED_TWICE = 16;
	// naming
	static const std::string ATTR_ID;
	static const std::string ATTR_MIN_DURATION;
	static const std::string ATTR_MAX_DURATION;
	static const std::string ATTR_TRANSFER_TIME;
	static const std::string ATTR_MAX_CARS;
	static const std::string ATTR_PERIOD;
	static const std::string ATTR_ROUTE;
	static const std::string ATTR_PHASE;
	static const std::string ATTR_DELAY;
	static const std::string ATTR_ROAD;
	static const std::string ATTR_LIGHTS;
	static const std::string TAG_LIGHTS_SYSTEMS;
	static const std::string TAG_LIGHTS_SYSTEM;
	static const std::string TAG_LIGHTS;
	static const std::string TAG_LIGHTS_GROUP;
	static const std::string TAG_GREEN;
	static const std::string TAG_RED;
	static const std::string TAG_ROADS;
	static const std::string TAG_ROAD;
	static const std::string TAG_CONNECTOR;
	static const std::string TAG_ROAD_LIGHTS;
	static const std::string TAG_ROUTES;
	static const std::string TAG_ROUTE;
	static const std::string TAG_GENERATORS;
	static const std::string TAG_GENERATOR;

	// methods ////////////////////////////////////////////////////////////////////////////////////
	SimplifiedModel* load(std::string path, int* pResult, int* pErrLine);
	
private:
	int loadLightsSystems(tinyxml2::XMLDocument& doc, 
		std::unordered_map<int, SimplifiedLights*>& idLights, int* pErrLine, 
		std::vector<SimplifiedLightsSystem*>& lightsSystems,
		std::vector<SimplifiedLightsGroup*>& lightsGroups);
	int loadRoads(tinyxml2::XMLDocument& doc, int* pErrLine, 
		std::unordered_map<int, SimplifiedRoad*>& idRoad, std::vector<SimplifiedRoad*>& roads);
	int loadConnectors(tinyxml2::XMLDocument& doc, int* pErrLine,
		std::unordered_map<int, SimplifiedRoad*>& idRoad,
		std::unordered_map<int, SimplifiedLights*>& idLights,
		SimplifiedRoad* pFinalRoad);
	int loadRoutes(tinyxml2::XMLDocument& doc, int* pErrLine,
		std::unordered_map<int, SimplifiedRoad*>& idRoad,
		std::unordered_map<int, std::vector<SimplifiedRoad*>*>& idRoute,
		std::vector<std::vector<SimplifiedRoad*>*>& routes,
		std::vector<int>& routesLines);
	int loadGenerators(tinyxml2::XMLDocument& doc, int* pErrLine,
		std::unordered_map<int, std::vector<SimplifiedRoad*>*>& idRoute,
		std::vector<SimplifiedGenerator*>& generators);
	bool areRoutesValid(std::vector<SimplifiedRoad*>& roads,
		std::vector<std::vector<SimplifiedRoad*>*>& routes,
		std::vector<int>& routesLines, int* pErrLine);
	bool areAllRoadsCovered(std::vector<SimplifiedRoad*>& roads,
		std::vector<std::vector<SimplifiedRoad*>*>& routes);
	/*
		In order to assure that a car from every route reaches the goal for specified
		number of times for each route one final road will be added. The final road will have
		transfer time equal to 0 and unreacheble max number of cars and it will work as a counter
		for the associated route because only cars from that route will go throught it.
	*/
	void addRoutesFinishers(std::vector<SimplifiedRoad*>& roads,
		std::vector<std::vector<SimplifiedRoad*>*>& routes,	SimplifiedRoad* pFinalRoad,
		SimplifiedLights* pGreenLights);
};
