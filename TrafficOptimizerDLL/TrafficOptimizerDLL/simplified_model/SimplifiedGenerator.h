#pragma once
#include "SimplifiedCar.h"
#include "SimplifiedRoad.h"
#include "SimplifiedModel.h"
#include <list>



class SimplifiedModel;



/*
	Generates cars. Generator is associated with a road and once generation time comes a new car
	(from a car pool) is generated and put on the road. If it wasn't possible to put the car on the
	road (because it was full) then the car is added to a queue and is put on the road as soon as
	possible. Frequency of generation is set by specifying period of generation. Also pahse may be
	specified.
*/
class SimplifiedGenerator
{
public:
	// constructors ///////////////////////////////////////////////////////////////////////////////
	SimplifiedGenerator(double period, SimplifiedCar* pTemplateCar, 
		SimplifiedRoad* pRoad, double phase = 0.0);
	~SimplifiedGenerator();

	// methods ////////////////////////////////////////////////////////////////////////////////////
	void tick(double currTime);
	void reset();
	void setModel(SimplifiedModel* pModel);
	std::list<SimplifiedCar*>* getReadyCars();
	int getNumOfReadyCars();

private:
	double period;
	double phase;
	double nextGenerationTime;
	SimplifiedCar* pTemplateCar;
	std::list<SimplifiedCar*> readyCars;
	SimplifiedRoad* pRoad;
	SimplifiedModel* pModel;
};
