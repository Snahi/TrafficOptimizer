#include "../../pch.h"
#include "UniformGenerator.h"



// constructors ///////////////////////////////////////////////////////////////////////////////////



UniformGenerator::UniformGenerator(int carsPerPeriod, double periodOfGeneration,
	std::list<Car*>* pActiveCarsInModel, Car* pCarTemplate, Road* pRoad)
	: Generator(periodOfGeneration, pActiveCarsInModel, pCarTemplate, pRoad)
{
	commonInit(carsPerPeriod);
}



UniformGenerator::UniformGenerator(int carsPerPeriod, double periodOfGeneration,
	std::list<Car*>* pActiveCarsInModel, Car* pCarTemplate, Road* pRoad, Generator* pNextGenerator)
	: Generator(periodOfGeneration, pActiveCarsInModel, pCarTemplate, pRoad, pNextGenerator)
{
	commonInit(carsPerPeriod);
}



// init methods ///////////////////////////////////////////////////////////////////////////////////



void UniformGenerator::commonInit(int carsPerPeriod)
{
	separationTime = periodOfGeneration / (double) carsPerPeriod;
}



// methods ////////////////////////////////////////////////////////////////////////////////////////



Generator* UniformGenerator::obtainFromString(std::vector<std::string>& parameters)
{
	if (parameters.size() == 2)
	{
		try
		{
			int carsPerPeriod = stoi(parameters[0]);
			double periodOfGeneration = stod(parameters[1]);
			
			return new UniformGenerator(carsPerPeriod, periodOfGeneration, nullptr, nullptr, nullptr);
		}
		catch (const exception e)
		{
			return nullptr;
		}
	}

	return nullptr;
}



bool UniformGenerator::shouldGenerateNow()
{
	if (timePassed - timeOfLastGeneration >= separationTime)
		return true;
	else
		return false;
}



Car* UniformGenerator::generate()
{
	return pGeneratedCarTemplate->clone();
}