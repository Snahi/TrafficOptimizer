#pragma once
#include "SimplifiedGenerator.h"
#include "SimplifiedCar.h"
#include "SimplifiedRoad.h"
#include "SimplifiedLightsSystem.h"
#include "SimplifiedLights.h"
#include "../optimizer/ModelInterface.h"
#include <vector>
#include <unordered_set>



// constants //////////////////////////////////////////////////////////////////////////////////////
#define CARS_PER_ROAD_TO_FINISH 50



class SimplifiedGenerator;



/*
	Model written with big focus on efficiency. It is based on time a car spends in eaach place
	instead of its position.
*/
class SimplifiedModel : public ModelInterface
{
public:
	// constructors ///////////////////////////////////////////////////////////////////////////////
	SimplifiedModel(std::vector<SimplifiedGenerator*>& generators, std::vector<SimplifiedRoad*>& roads,
		std::vector<SimplifiedLightsSystem*>& lightsSystems, 
		std::vector<SimplifiedLightsGroup*>& lightsGroups, 
		std::vector<std::vector<SimplifiedRoad*>*>& routes, SimplifiedRoad* pFinalRoad);
	~SimplifiedModel();

	// methods ////////////////////////////////////////////////////////////////////////////////////
	virtual int run(double tickDuration, double timeoutSeconds) override;
	virtual double getAvgTime() override;
	virtual double getAdditionalTime() override;
	virtual int getNumOfFinishedCars() override;
	virtual int getNumOfReadyCars() override;
	virtual bool update(double* newValues) override;
	virtual void getRanges(std::pair<double, double>* ranges) override;
	virtual int getProblemSize() override;
	virtual void translate(double* individual, std::unordered_map<int, double>& groups,
		std::unordered_map<int, double>& systems) override;
	/*
		Returns instance of a car from a car pool. Attributes of the returned car are set to the
		values of attributes of pTemplate.
	*/
	SimplifiedCar* getCarInstance(SimplifiedCar* pTemplate);
	/*
		For adding lights which are not part of user design, but were created automatically at the
		end and have some different purpose. They are added here just so that model deletes them
		when it is deleted itself.
	*/
	void addFreeLights(SimplifiedLights* pLights);

private:
	// fields /////////////////////////////////////////////////////////////////////////////////////
	std::vector<SimplifiedLightsSystem*> lightsSystems;
	std::vector<SimplifiedLightsGroup*> lightsGroups;
	std::vector<SimplifiedRoad*> roads;
	std::vector<SimplifiedGenerator*> generators;
	std::vector<std::vector<SimplifiedRoad*>*> routes;
	/*
		To make the model faster cars are not created and deleted whenever the model is reset.
		Instead, when a new car is necessary it must be acquired through getCarInstance(...).
		That method (getCarInstance) takes a car from this pool if there is an available car
		or creates a new one and adds it to this pool if there weren't available cars.
	*/
	std::vector<SimplifiedCar*> carPool;
	/*
		Index of available car in carPool. If it is equal to the size of carPool then there are
		no available cars and a new one must be created.
	*/
	int idxOfFreeCar;
	/*
		Artificial road on which finished cars are put.
	*/
	SimplifiedRoad* pFinalRoad;
	int problemSize;
	bool isRunning;
	/*
		Lights which will be deleted together with this model, but nothing else is done with them.
		It's just to make the deletion safer - all model connected things are deleted when a model
		is deleted.
	*/
	std::vector<SimplifiedLights*> freeLights;

	// methods ////////////////////////////////////////////////////////////////////////////////////
	/*
		Restore initial state of this model.
	*/
	void reset();
};
