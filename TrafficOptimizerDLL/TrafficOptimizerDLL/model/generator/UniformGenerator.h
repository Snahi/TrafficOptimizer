#pragma once
#include "Generator.h"



class UniformGenerator : public Generator
{
public:
	// constructors ///////////////////////////////////////////////////////////////////////////////
	UniformGenerator(int carsPerPeriod, double periodOfGeneration, 
		std::list<Car*>* pActiveCarsInModel, Car* pCarTemplate, Road* pRoad);
	UniformGenerator(int carsPerPeriod, double periodOfGeneration, 
		std::list<Car*>* pActiveCarsInModel, Car* pCarTemplate, Road* pRoad, Generator* pNextGenerator);
	// methods ////////////////////////////////////////////////////////////////////////////////////
	static Generator* obtainFromString(std::vector<std::string>& parameters);
protected:
	bool shouldGenerateNow();
	Car* generate();

private:
	// methods ////////////////////////////////////////////////////////////////////////////////////
	void commonInit(int carsPerPeriod);

	// fields /////////////////////////////////////////////////////////////////////////////////////
	double separationTime;
};
