#include "../pch.h"
#include "SimplifiedModelLoader.h"
#include "SimplifiedLightsGroup.h"
#include "SimplifiedCar.h"
#include "Connector.h"
#include <unordered_set>



using namespace std;
using namespace tinyxml2;



// constants initialization ///////////////////////////////////////////////////////////////////////
const string SimplifiedModelLoader::ATTR_ID = "id";
const string SimplifiedModelLoader::ATTR_MIN_DURATION = "min_duration";
const string SimplifiedModelLoader::ATTR_MAX_DURATION = "max_duration";
const string SimplifiedModelLoader::ATTR_TRANSFER_TIME = "transfer_time";
const string SimplifiedModelLoader::ATTR_MAX_CARS = "max_cars";
const string SimplifiedModelLoader::ATTR_PERIOD = "period";
const string SimplifiedModelLoader::ATTR_ROUTE = "route";
const string SimplifiedModelLoader::ATTR_PHASE = "phase";
const string SimplifiedModelLoader::ATTR_DELAY = "delay";
const string SimplifiedModelLoader::ATTR_ROAD = "road";
const string SimplifiedModelLoader::ATTR_LIGHTS = "lights";
const string SimplifiedModelLoader::TAG_LIGHTS_SYSTEMS = "lights_systems";
const string SimplifiedModelLoader::TAG_LIGHTS_SYSTEM = "lights_system";
const string SimplifiedModelLoader::TAG_LIGHTS = "lights";
const string SimplifiedModelLoader::TAG_LIGHTS_GROUP = "lights_group";
const string SimplifiedModelLoader::TAG_GREEN = "green";
const string SimplifiedModelLoader::TAG_RED = "red";
const string SimplifiedModelLoader::TAG_ROADS = "roads";
const string SimplifiedModelLoader::TAG_ROAD = "road";
const string SimplifiedModelLoader::TAG_ROUTES = "routes";
const string SimplifiedModelLoader::TAG_ROUTE = "route";
const string SimplifiedModelLoader::TAG_GENERATORS = "generators";
const string SimplifiedModelLoader::TAG_GENERATOR = "generator";
const string SimplifiedModelLoader::TAG_CONNECTOR = "connector";
const string SimplifiedModelLoader::TAG_ROAD_LIGHTS = "road_lights";



// methods ////////////////////////////////////////////////////////////////////////////////////////



SimplifiedModel* SimplifiedModelLoader::load(string path, int* pResult, int* pErrLine)
{
	int res = SimplifiedModelLoader::LOAD_SUCCESS;
	SimplifiedModel* pModel = nullptr;
	XMLDocument doc;
	if (doc.LoadFile(path.c_str()) == XML_SUCCESS)
	{
		unordered_map<int, SimplifiedLights*> idLights;
		vector<SimplifiedLightsSystem*> lightsSystems;
		vector<SimplifiedLightsGroup*> lightsGroups;
		unordered_map<int, SimplifiedRoad*> idRoad;
		vector<SimplifiedRoad*> roads;
		unordered_map<int, vector<SimplifiedRoad*>*> idRoute;
		vector<vector<SimplifiedRoad*>*> routes;
		vector<int> routesLines;
		vector<SimplifiedGenerator*> generators;
		// finisher roads
		SimplifiedLights* pGreenLights = new SimplifiedLights(); // to protect access to finishers
		pGreenLights->setGreen();
		// final road
		SimplifiedRoad* pFinalRoad = new SimplifiedRoad(DBL_MAX / 2, INT_MAX);
		unordered_map<SimplifiedRoad*, SimplifiedLights*> roadLights;
		SimplifiedLights* pLights = new SimplifiedLights();
		pLights->setGreen();
		roadLights[pFinalRoad] = pLights;
		Connector* pFinalRoadConnector = new Connector(0.0, roadLights, pFinalRoad);
		pFinalRoad->setConnector(pFinalRoadConnector);

		if ((res = loadLightsSystems(doc, idLights, pErrLine, lightsSystems, lightsGroups)) ==
			SimplifiedModelLoader::LOAD_SUCCESS)
		{
			if ((res = loadRoads(doc, pErrLine, idRoad, roads)) == SimplifiedModelLoader::LOAD_SUCCESS)
			{
				if ((res = loadRoutes(doc, pErrLine, idRoad, idRoute, routes, routesLines)) ==
					SimplifiedModelLoader::LOAD_SUCCESS)
				{
					if ((res = loadConnectors(doc, pErrLine, idRoad, idLights, pFinalRoad)) ==
						SimplifiedModelLoader::LOAD_SUCCESS)
					{
						if (areRoutesValid(roads, routes, routesLines, pErrLine))
						{
							if (areAllRoadsCovered(roads, routes))
							{
								addRoutesFinishers(roads, routes, pFinalRoad, pGreenLights);

								if ((res = loadGenerators(doc, pErrLine, idRoute, generators)) ==
									SimplifiedModelLoader::LOAD_SUCCESS)
								{
									pModel = new SimplifiedModel(generators, roads, lightsSystems,
										lightsGroups, routes, pFinalRoad);
									pModel->addFreeLights(pGreenLights);
									pModel->addFreeLights(pLights);
								}
							}
							else // uncovered routes
								res = SimplifiedModelLoader::LOAD_ERR_UNCOVERED_ROADS;
						}
						else // invalid routes
							res = SimplifiedModelLoader::LOAD_ERR_INVALID_ROUTE;
					}
				}
			}
		}

		if (res != SimplifiedModelLoader::LOAD_SUCCESS)
		{
			// clean up
			for (auto& pairLights : idLights)
				delete pairLights.second;
			for (SimplifiedLightsSystem* pToDelete : lightsSystems)
				delete pToDelete;
			for (SimplifiedLightsGroup* pToDelete : lightsGroups)
				delete pToDelete;
			for (SimplifiedRoad* pToDelete : roads)
				delete pToDelete;
			for (auto toDelete : routes)
				delete toDelete;
			for (SimplifiedGenerator* pToDelete : generators)
				delete pToDelete;

			delete pLights;
			delete pGreenLights;
			delete pFinalRoad;
		}
	}
	else // invalid xml file
	{
		*pErrLine = doc.ErrorLineNum();
		res = SimplifiedModelLoader::LOAD_ERR_INVALID_FILE;
	}

	*pResult = res;
	return pModel;
}



// helpers ////////////////////////////////////////////////////////////////////////////////////////



void getChildren(XMLElement* pParent, vector<XMLElement*>& children,
	const char* childTag)
{
	XMLElement* pChild = pParent->FirstChildElement(childTag);
	while (pChild != nullptr)
	{
		children.push_back(pChild);
		pChild = pChild->NextSiblingElement(childTag);
	}
}



int getAttributes(XMLElement* pElement, vector<const char*>& attrNames,
	vector<double>& values, int* pErrLine)
{
	int numOfAttr = attrNames.size();
	double value;
	for (int i = 0; i < numOfAttr; ++i)
	{
		XMLError err = pElement->QueryDoubleAttribute(attrNames[i], &value);
		if (err == XML_SUCCESS)
		{
			values.push_back(value);
		}
		else
		{
			*pErrLine = pElement->GetLineNum();
			if (err == XMLError::XML_NO_ATTRIBUTE)
				return SimplifiedModelLoader::LOAD_ERR_MISSING_ATTRIBUTE;
			else
				return SimplifiedModelLoader::LOAD_ERR_WRONG_ATTRIBUTE;
		}
	}

	return SimplifiedModelLoader::LOAD_SUCCESS;
}



int getChildrenIds(XMLElement* pElement, const char* attrId, vector<int>& ids, int* pErrLine)
{
	XMLElement* pChild = pElement->FirstChildElement();
	int id;
	while (pChild != nullptr)
	{
		XMLError err = pChild->QueryIntAttribute(attrId, &id);
		if (err == XML_SUCCESS)
		{
			ids.push_back(id);
		}
		else
		{
			*pErrLine = pChild->GetLineNum();
			if (err == XMLError::XML_NO_ATTRIBUTE)
				return SimplifiedModelLoader::LOAD_ERR_MISSING_ATTRIBUTE;
			else
				return SimplifiedModelLoader::LOAD_ERR_WRONG_ATTRIBUTE;
		}

		pChild = pChild->NextSiblingElement();
	}

	return SimplifiedModelLoader::LOAD_SUCCESS;
}



int getIntAttr(XMLElement* pElement, const char* attrId, int* pId, int* pErrLine)
{
	XMLError err = pElement->QueryIntAttribute(attrId, pId);
	if (err == XML_SUCCESS)
		return SimplifiedModelLoader::LOAD_SUCCESS;
	else
	{
		*pErrLine = pElement->GetLineNum();
		if (err == XMLError::XML_NO_ATTRIBUTE)
			return SimplifiedModelLoader::LOAD_ERR_MISSING_ATTRIBUTE;
		else
			return SimplifiedModelLoader::LOAD_ERR_WRONG_ATTRIBUTE;
	}
}



// lights /////////////////////////////////////////////////////////////////////////////////////////



int loadLights(vector<XMLElement*>& lightsXML, unordered_map<int, SimplifiedLights*>& idLights, 
	vector<SimplifiedLights*>& lightsInSystem, int* pErrLine)
{
	const char* attrId = SimplifiedModelLoader::ATTR_ID.c_str();
	int id;
	int res;
	SimplifiedLights* pNewLights;
	for (XMLElement* pLights : lightsXML)
	{
		res = getIntAttr(pLights, attrId, &id, pErrLine);
		if (res == SimplifiedModelLoader::LOAD_SUCCESS)
		{
			if (idLights.find(id) == idLights.end()) // id unique
			{
				pNewLights = new SimplifiedLights();
				idLights[id] = pNewLights;
				lightsInSystem.push_back(pNewLights);
			}
			else // non-unique id
			{
				*pErrLine = pLights->GetLineNum();
				return SimplifiedModelLoader::LOAD_ERR_NON_UNQ_ID;
			}
		}
		else
			return res;
	}

	return SimplifiedModelLoader::LOAD_SUCCESS;
}



int transformLightsIdsToLights(vector<int>& greenIds, vector<int>& redIds,
	vector<SimplifiedLights*>& green, vector<SimplifiedLights*>& red, 
	vector<SimplifiedLights*>& lightsInSystem,
	unordered_map<int, SimplifiedLights*>& idLights)
{
	// check if green and red are disjoint
	for (int greenId : greenIds)
	{
		for (int redId : redIds)
		{
			if (greenId == redId)
				return SimplifiedModelLoader::LOAD_ERR_LIGHTS_IN_GREEN_AND_RED;
		}
	}

	SimplifiedLights* pLights;

	// convert green
	for (int greenId : greenIds)
	{
		pLights = idLights[greenId];
		if (pLights != nullptr)
		{
			if (find(lightsInSystem.begin(), lightsInSystem.end(), pLights) ==
				lightsInSystem.end()) // lights are from different lights system
			{
				return SimplifiedModelLoader::LOAD_ERR_LIGHTS_FROM_DIFFERENT_SYSTEM;
			}
			green.push_back(pLights);
		}
		else
			return SimplifiedModelLoader::LOAD_ERR_NO_SUCH_LIGHTS;
	}

	// convert red
	for (int redId : redIds)
	{
		pLights = idLights[redId];
		if (pLights != nullptr)
		{
			if (find(lightsInSystem.begin(), lightsInSystem.end(), pLights) ==
				lightsInSystem.end()) // lights are from different lights system
			{
				return SimplifiedModelLoader::LOAD_ERR_LIGHTS_FROM_DIFFERENT_SYSTEM;
			}
			red.push_back(pLights);
		}
		else
			return SimplifiedModelLoader::LOAD_ERR_NO_SUCH_LIGHTS;
	}

	return SimplifiedModelLoader::LOAD_SUCCESS;
}



int validateLightsGroupAttributes(unordered_set<int>& idsInUse, int id, double minDuration, 
	double maxDuration)
{
	if (idsInUse.find(id) != idsInUse.end())
		return SimplifiedModelLoader::LOAD_ERR_NON_UNQ_ID;

	if (minDuration < 0.0 || maxDuration < minDuration)
		return SimplifiedModelLoader::LOAD_ERR_WRONG_ATTRIBUTE;

	return SimplifiedModelLoader::LOAD_SUCCESS;
}



int loadLightsGroups(vector<XMLElement*>& groupsXML, vector<SimplifiedLightsGroup*>& groups,
	vector<SimplifiedLights*>& lightsInSystem,
	unordered_map<int, SimplifiedLights*>& idLights, int* pErrLine,
	vector<SimplifiedLightsGroup*>& allLightsGroups)
{
	int numOfLightsInSystem = lightsInSystem.size();
	const char* attrId = SimplifiedModelLoader::ATTR_ID.c_str();
	const char* tagGreen = SimplifiedModelLoader::TAG_GREEN.c_str();
	const char* tagRed = SimplifiedModelLoader::TAG_RED.c_str();
	/* names of a lights group attributes */
	vector<const char*> attrNames;
	attrNames.push_back(attrId);
	attrNames.push_back(SimplifiedModelLoader::ATTR_MIN_DURATION.c_str());
	attrNames.push_back(SimplifiedModelLoader::ATTR_MAX_DURATION.c_str());
	/* 
		values of attributes specified in attrNames
		[0] -> id
		[1] -> minDuration
		[2] -> maxDuration
	*/
	vector<double> attr;
	int id;
	double minDuration, maxDuration;

	/* ids of already created lights groups */
	unordered_set<int> idsInUse;

	XMLElement* pGreen; // <green>
	XMLElement* pRed;	// <red>
	vector<int> greenLightsIds;
	vector<int> redLightsIds;
	vector<SimplifiedLights*> greenLights;
	vector<SimplifiedLights*> redLights;
	int res;
	SimplifiedLightsGroup* pNewGroup;
	for (XMLElement* pGroupXML : groupsXML)
	{
		// get lights group attributes
		res = getAttributes(pGroupXML, attrNames, attr, pErrLine);
		if (res != SimplifiedModelLoader::LOAD_SUCCESS)
			return res;

		id = (int) attr[0];
		minDuration = attr[1];
		maxDuration = attr[2];
		res = validateLightsGroupAttributes(idsInUse, id, minDuration, maxDuration);
		if (res != SimplifiedModelLoader::LOAD_SUCCESS)
		{
			*pErrLine = pGroupXML->GetLineNum();
			return res;
		}
		
		// get green and red section
		pGreen = pGroupXML->FirstChildElement(tagGreen);
		pRed = pGroupXML->FirstChildElement(tagRed);
		if (pGreen == nullptr || pRed == nullptr)
		{
			*pErrLine = pGroupXML->GetLineNum();
			return SimplifiedModelLoader::LOAD_ERR_GREEN_AND_RED_NOT_SPECIFIED;
		}

		// get green lights ids
		res = getChildrenIds(pGreen, attrId, greenLightsIds, pErrLine);
		if (res != SimplifiedModelLoader::LOAD_SUCCESS)
			return res;

		// get red lights ids
		res = getChildrenIds(pRed, attrId, redLightsIds, pErrLine);
		if (res != SimplifiedModelLoader::LOAD_SUCCESS)
			return res;

		// check if all lights are covered (in terms of numbers, ids validity will be checked afterwards)
		if (greenLightsIds.size() + redLightsIds.size() != numOfLightsInSystem)
		{
			*pErrLine = pGroupXML->GetLineNum();
			return SimplifiedModelLoader::LOAD_ERR_NOT_ALL_LIGHTS_IN_LIGHTS_GROUP;
		}

		// transform ids to lights
		res = transformLightsIdsToLights(greenLightsIds, redLightsIds, greenLights, redLights, 
			lightsInSystem, idLights);
		if (res != SimplifiedModelLoader::LOAD_SUCCESS)
		{
			*pErrLine = pGroupXML->GetLineNum();
			return res;
		}

		// create a lights group
		pNewGroup = new SimplifiedLightsGroup(id, greenLights, redLights, minDuration, maxDuration);
		groups.push_back(pNewGroup);
		idsInUse.insert(id);
		allLightsGroups.push_back(pNewGroup);

		// clean up for the next lights group
		greenLightsIds.clear();
		redLightsIds.clear();
		attr.clear();
		greenLights.clear();
		redLights.clear();
	}

	return SimplifiedModelLoader::LOAD_SUCCESS;
}



int SimplifiedModelLoader::loadLightsSystems(XMLDocument& doc, 
	unordered_map<int, SimplifiedLights*>& idLights, int* pErrLine, 
	vector<SimplifiedLightsSystem*>& lightsSystems,
	vector<SimplifiedLightsGroup*>& lightsGroups)
{
	XMLElement* pLightsSystems = doc.FirstChildElement(
		SimplifiedModelLoader::TAG_LIGHTS_SYSTEMS.c_str());
	if (pLightsSystems == nullptr)
		return SimplifiedModelLoader::LOAD_ERR_SECTION_NOT_SPECIFIED;

	vector<XMLElement*> lightsSystemsXML;
	getChildren(pLightsSystems, lightsSystemsXML, SimplifiedModelLoader::TAG_LIGHTS_SYSTEM.c_str());

	vector<XMLElement*> lightsXML;
	vector<SimplifiedLights*> lightsInSystem;
	vector<XMLElement*> lightsGroupsXML;
	vector<SimplifiedLightsGroup*> lightsGroupsInSystem;
	/* ids of lights systems which are already in use */
	unordered_set<int> idsInUse;
	int res;
	const char* attrId = SimplifiedModelLoader::ATTR_ID.c_str();
	int id;
	const char* tagLights = SimplifiedModelLoader::TAG_LIGHTS.c_str();
	const char* tagLightsGroup = SimplifiedModelLoader::TAG_LIGHTS_GROUP.c_str();
	for (XMLElement* pLightsSystem : lightsSystemsXML)
	{
		// get lights system's id
		res = getIntAttr(pLightsSystem, attrId, &id, pErrLine);
		if (res != SimplifiedModelLoader::LOAD_SUCCESS)
			return res;

		if (idsInUse.find(id) != idsInUse.end())
		{
			*pErrLine = pLightsSystem->GetLineNum();
			return SimplifiedModelLoader::LOAD_ERR_NON_UNQ_ID;
		}

		// load lights
		getChildren(pLightsSystem, lightsXML, tagLights);
		res = loadLights(lightsXML, idLights, lightsInSystem, pErrLine);
		if (res != SimplifiedModelLoader::LOAD_SUCCESS)
			return res;

		// load lights groups
		getChildren(pLightsSystem, lightsGroupsXML, tagLightsGroup);
		res = loadLightsGroups(lightsGroupsXML, lightsGroupsInSystem, lightsInSystem, idLights, 
			pErrLine, lightsGroups);
		if (res != SimplifiedModelLoader::LOAD_SUCCESS)
			return res;

		// create the lights system
		lightsSystems.push_back(new SimplifiedLightsSystem(id, lightsGroupsInSystem, lightsInSystem));
		idsInUse.insert(id);

		lightsInSystem.clear();
		lightsXML.clear();
		lightsGroupsXML.clear();
		lightsGroupsInSystem.clear();
	}

	return SimplifiedModelLoader::LOAD_SUCCESS;
}



// roads //////////////////////////////////////////////////////////////////////////////////////////



int SimplifiedModelLoader::loadRoads(XMLDocument& doc, int* pErrLine, 
	unordered_map<int, SimplifiedRoad*>& idRoad,	vector<SimplifiedRoad*>& roads)
{
	XMLElement* pRoadsXML = doc.FirstChildElement(SimplifiedModelLoader::TAG_ROADS.c_str());
	if (pRoadsXML == nullptr)
		return SimplifiedModelLoader::LOAD_ERR_SECTION_NOT_SPECIFIED;

	// obtain road XMLElements
	vector<XMLElement*> roadsXML;
	getChildren(pRoadsXML, roadsXML, SimplifiedModelLoader::TAG_ROAD.c_str());

	// read roads attributes
	vector<const char*> attrNames;
	attrNames.push_back(SimplifiedModelLoader::ATTR_ID.c_str());
	attrNames.push_back(SimplifiedModelLoader::ATTR_TRANSFER_TIME.c_str());
	attrNames.push_back(SimplifiedModelLoader::ATTR_MAX_CARS.c_str());
	/*
		[0] -> id
		[1] -> transfer time
		[2] -> max cars
	*/
	vector<double> attr;
	int id, maxCars, res;
	double transferTime;
	SimplifiedRoad* pRoad;
	for (XMLElement* pRoadXML : roadsXML)
	{
		res = getAttributes(pRoadXML, attrNames, attr, pErrLine);
		if (res != XML_SUCCESS)
			return res;

		id = (int)attr[0];
		transferTime = attr[1];
		maxCars = (int)attr[2];

		// check if id is unique
		if (idRoad.find(id) != idRoad.end())
		{
			*pErrLine = pRoadXML->GetLineNum();
			return SimplifiedModelLoader::LOAD_ERR_NON_UNQ_ID;
		}

		// check if attributes are valid
		if (transferTime < 0.0 || maxCars < 1)
		{
			*pErrLine = pRoadXML->GetLineNum();
			return SimplifiedModelLoader::LOAD_ERR_WRONG_ATTRIBUTE;
		}

		pRoad = new SimplifiedRoad(transferTime, maxCars);
		idRoad[id] = pRoad;
		roads.push_back(pRoad);

		attr.clear();
	}

	return SimplifiedModelLoader::LOAD_SUCCESS;
}



// connectors /////////////////////////////////////////////////////////////////////////////////////



int getRoadLights(XMLElement* pConnectorXML, 
	unordered_map<SimplifiedRoad*, SimplifiedLights*>& roadsLights,
	unordered_map<int, SimplifiedRoad*>& idRoad,
	unordered_map<int, SimplifiedLights*>& idLights,
	int* pErrLine)
{
	vector<XMLElement*> roadsLightsXML;
	const char* tagRoadLights = SimplifiedModelLoader::TAG_ROAD_LIGHTS.c_str();
	getChildren(pConnectorXML, roadsLightsXML, tagRoadLights);
	vector<const char*> attrNames;
	attrNames.push_back(SimplifiedModelLoader::ATTR_ROAD.c_str());
	attrNames.push_back(SimplifiedModelLoader::ATTR_LIGHTS.c_str());
	/*
		[0] -> road id
		[1] -> lights id
	*/
	vector<double> attr;
	unordered_set<int> roadsIdsInUse;
	int roadId, lightsId;
	int res;
	for (XMLElement* pRoadLightsXML : roadsLightsXML)
	{
		// read attributes
		res = getAttributes(pRoadLightsXML, attrNames, attr, pErrLine);
		if (res != XML_SUCCESS)
			return res;
		roadId = (int)attr[0];
		lightsId = (int)attr[1];

		// check if the roads isn't already referenced
		if (roadsIdsInUse.find(roadId) != roadsIdsInUse.end())
		{
			*pErrLine = pRoadLightsXML->GetLineNum();
			return SimplifiedModelLoader::LOAD_ERR_THE_SAME_ROAD_REFERENCED_TWICE;
		}
		
		// check if road and lights exists
		if (idRoad.find(roadId) == idRoad.end() || idLights.find(lightsId) == idLights.end())
		{
			*pErrLine = pRoadLightsXML->GetLineNum();
			return SimplifiedModelLoader::LOAD_ERR_REFERENCED_ITEM_DOES_NOT_EXIST;
		}

		roadsLights[idRoad[roadId]] = idLights[lightsId];
		roadsIdsInUse.insert(roadId);
		attr.clear();
	}

	return SimplifiedModelLoader::LOAD_SUCCESS;
}



int SimplifiedModelLoader::loadConnectors(tinyxml2::XMLDocument& doc, int* pErrLine,
	std::unordered_map<int, SimplifiedRoad*>& idRoad,
	std::unordered_map<int, SimplifiedLights*>& idLights,
	SimplifiedRoad* pFinalRoad)
{
	XMLElement* pRoadsXML = doc.FirstChildElement(SimplifiedModelLoader::TAG_ROADS.c_str()); // null was checked in loadRoads
	XMLElement* pConnectorXML;
	int parentRoadId;
	const char* attrId = SimplifiedModelLoader::ATTR_ID.c_str();
	vector<XMLElement*> roadsXML;
	getChildren(pRoadsXML, roadsXML, SimplifiedModelLoader::TAG_ROAD.c_str());
	SimplifiedRoad* pRoad;
	Connector* pConnector;
	const char* tagConnector = SimplifiedModelLoader::TAG_CONNECTOR.c_str();
	vector<const char*> attrNames;
	attrNames.push_back(SimplifiedModelLoader::ATTR_DELAY.c_str());
	/* [0] -> delay */
	vector<double> attr;
	double delay = 0.0;
	unordered_map<SimplifiedRoad*, SimplifiedLights*> roadLights;
	int res;
	for (XMLElement* pRoadXML : roadsXML)
	{
		// read connector
		delay = 0.0;
		pConnectorXML = pRoadXML->FirstChildElement(tagConnector);
		if (pConnectorXML != nullptr)
		{
			// read connector's attributes
			res = getAttributes(pConnectorXML, attrNames, attr, pErrLine);
			if (res != XML_SUCCESS)
				return res;
			delay = attr[0];
			if (delay < 0.0)
				return SimplifiedModelLoader::LOAD_ERR_WRONG_ATTRIBUTE;

			// read roadLights
			res = getRoadLights(pConnectorXML, roadLights, idRoad, idLights, pErrLine);
			if (res != XML_SUCCESS)
				return res;
		}
		// if there is no connector specified then it is the last road, but create a connector anyway
		// to prevent null pointer and connect the road with the final road

		// get parent road
		res = getIntAttr(pRoadXML, attrId, &parentRoadId, pErrLine); // null check in loadRoads()
		pRoad = idRoad[parentRoadId];

		// create the connector and add it to the road
		pConnector = new Connector(delay, roadLights, pFinalRoad);
		
		pRoad->setConnector(pConnector);

		roadLights.clear();
		attr.clear();
	}

	return SimplifiedModelLoader::LOAD_SUCCESS;
}



// routes /////////////////////////////////////////////////////////////////////////////////////////



int SimplifiedModelLoader::loadRoutes(XMLDocument& doc, int* pErrLine,
	unordered_map<int, SimplifiedRoad*>& idRoad,
	unordered_map<int, std::vector<SimplifiedRoad*>*>& idRoute,
	vector<vector<SimplifiedRoad*>*>& routes,
	vector<int>& routesLines)
{
	XMLElement* pRoutesXML = doc.FirstChildElement(SimplifiedModelLoader::TAG_ROUTES.c_str());
	if (pRoutesXML == nullptr)
		return SimplifiedModelLoader::LOAD_ERR_SECTION_NOT_SPECIFIED;

	// read all routes
	vector<XMLElement*> routesXML;
	getChildren(pRoutesXML, routesXML, SimplifiedModelLoader::TAG_ROUTE.c_str());
	vector<int> idsOfRoadsInRoute;
	vector<SimplifiedRoad*>* pRoadsInRoute;
	const char* attrId = SimplifiedModelLoader::ATTR_ID.c_str();
	int res, id;
	for (XMLElement* pRouteXML : routesXML)
	{
		// read route id
		res = getIntAttr(pRouteXML, attrId, &id, pErrLine);
		if (res != XML_SUCCESS)
			return res;

		// check if route id is unique
		if (idRoute.find(id) != idRoute.end())
		{
			*pErrLine = pRouteXML->GetLineNum();
			return SimplifiedModelLoader::LOAD_ERR_NON_UNQ_ID;
		}

		res = getChildrenIds(pRouteXML, attrId, idsOfRoadsInRoute, pErrLine);
		if (res != XML_SUCCESS)
			return res;

		// check if there is at least one road
		if (idsOfRoadsInRoute.size() == 0)
		{
			*pErrLine = pRouteXML->GetLineNum();
			return SimplifiedModelLoader::LOAD_ERR_EMPTY_LIST;
		}

		pRoadsInRoute = new vector<SimplifiedRoad*>();

		// check if such roads exist
		for (int roadId : idsOfRoadsInRoute)
		{
			if (idRoad.find(roadId) == idRoad.end())
			{
				*pErrLine = pRouteXML->GetLineNum();
				delete pRoadsInRoute;
				return SimplifiedModelLoader::LOAD_ERR_REFERENCED_ITEM_DOES_NOT_EXIST;
			}

			pRoadsInRoute->push_back(idRoad[roadId]);
		}

		idRoute[id] = pRoadsInRoute;
		routes.push_back(pRoadsInRoute);
		routesLines.push_back(pRouteXML->GetLineNum());

		idsOfRoadsInRoute.clear();
	}

	return SimplifiedModelLoader::LOAD_SUCCESS;
}



int SimplifiedModelLoader::loadGenerators(XMLDocument& doc, int* pErrLine,
	unordered_map<int, std::vector<SimplifiedRoad*>*>& idRoute,
	vector<SimplifiedGenerator*>& generators)
{
	XMLElement* pGensXML = doc.FirstChildElement(SimplifiedModelLoader::TAG_GENERATORS.c_str());
	if (pGensXML == nullptr)
		return SimplifiedModelLoader::LOAD_ERR_SECTION_NOT_SPECIFIED;

	const char* tagGenerator = SimplifiedModelLoader::TAG_GENERATOR.c_str();
	XMLElement* pGeneratorXML = pGensXML->FirstChildElement(tagGenerator);
	
	if (pGeneratorXML == nullptr)
	{
		*pErrLine = pGensXML->GetLineNum();
		return SimplifiedModelLoader::LOAD_ERR_EMPTY_LIST;
	}

	double period;
	int routeId, res;
	SimplifiedCar* pTemplateCar;
	vector<SimplifiedRoad*>* pRoute;
	vector<const char*> attrNames;
	/*
		[0] -> period
		[1] -> route
	*/
	attrNames.push_back(SimplifiedModelLoader::ATTR_PERIOD.c_str());
	attrNames.push_back(SimplifiedModelLoader::ATTR_ROUTE.c_str());
	vector<double> attr;
	double phase;
	/* routes which don't have a generator assigned */
	unordered_set<int> uncoveredRoutes;
	for (auto& pairIdRoute : idRoute)
		uncoveredRoutes.insert(pairIdRoute.first);
	while (pGeneratorXML != nullptr)
	{
		// read attributes
		res = getAttributes(pGeneratorXML, attrNames, attr, pErrLine);
		if (res != XML_SUCCESS)
			return res;
		phase = pGeneratorXML->DoubleAttribute(SimplifiedModelLoader::ATTR_PHASE.c_str(), 0.0);
		period = attr[0];
		routeId = (int) attr[1];

		// check if period is valid
		if (period < 0.0)
		{
			*pErrLine = pGeneratorXML->GetLineNum();
			return SimplifiedModelLoader::LOAD_ERR_WRONG_ATTRIBUTE;
		}

		// check if route exists
		if (idRoute.find(routeId) == idRoute.end())
		{
			*pErrLine = pGeneratorXML->GetLineNum();
			return SimplifiedModelLoader::LOAD_ERR_REFERENCED_ITEM_DOES_NOT_EXIST;
		}

		uncoveredRoutes.erase(routeId);
		pRoute = idRoute[routeId];

		// create the generator
		pTemplateCar = new SimplifiedCar(pRoute);
		generators.push_back(new SimplifiedGenerator(period, pTemplateCar, pRoute->at(0), phase));

		pGeneratorXML = pGeneratorXML->NextSiblingElement(tagGenerator);
		attr.clear();
	}

	if (!uncoveredRoutes.empty())
	{
		*pErrLine = pGensXML->GetLineNum();
		return SimplifiedModelLoader::LOAD_ERR_NOT_ALL_ROUTES_ASSOCIATIED_WITH_GENERATOR;
	}

	return SimplifiedModelLoader::LOAD_SUCCESS;
}



// final checks ///////////////////////////////////////////////////////////////////////////////////



bool SimplifiedModelLoader::areRoutesValid(vector<SimplifiedRoad*>& roads, 
	vector<vector<SimplifiedRoad*>*>& routes, vector<int>& routesLines,
	int* pErrLine)
{
	SimplifiedRoad* pPrev, * pCurr;
	vector<SimplifiedRoad*>* pRoute;
	int numOfRoutes = routes.size();
	for (int routeIdx = 0; routeIdx < numOfRoutes; ++routeIdx)
	{
		pRoute = routes[routeIdx];
		for (int i = 1; i < (int) pRoute->size(); ++i)
		{
			pPrev = pRoute->at(i - 1);
			pCurr = pRoute->at(i);
			if (!pPrev->isConnected(pCurr))
			{
				*pErrLine = routesLines[routeIdx];
				return false;
			}
		}
	}

	return true;
}



bool SimplifiedModelLoader::areAllRoadsCovered(vector<SimplifiedRoad*>& roads,
	vector<vector<SimplifiedRoad*>*>& routes)
{
	unordered_set<SimplifiedRoad*> uncovered;
	for (SimplifiedRoad* pRoad : roads)
		uncovered.insert(pRoad);

	for (vector<SimplifiedRoad*>* pRoute : routes)
	{
		for (SimplifiedRoad* pRoad : *pRoute)
		{
			uncovered.erase(pRoad);
		}
	}

	return uncovered.empty();
}



// routes finishers ///////////////////////////////////////////////////////////////////////////////



void SimplifiedModelLoader::addRoutesFinishers(vector<SimplifiedRoad*>& roads,
	vector<vector<SimplifiedRoad*>*>& routes,
	SimplifiedRoad* pFinalRoad, SimplifiedLights* pGreenLights)
{
	SimplifiedRoad* pFinisher;
	Connector* pFinisherConnector;
	unordered_map<SimplifiedRoad*, SimplifiedLights*> roadLights;
	for (vector<SimplifiedRoad*>* pRoute : routes)
	{
		pFinisher = new SimplifiedRoad(0.0, INT_MAX);
		pFinisherConnector = new Connector(0.0, roadLights, pFinalRoad);
		pFinisher->setConnector(pFinisherConnector);
		roads.push_back(pFinisher);
		pRoute->back()->addRoadToConnector(pFinisher, pGreenLights);
		pRoute->push_back(pFinisher);
	}
}