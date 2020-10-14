#include "../../pch.h"
#include "Generator.h"
#include "UniformGenerator.h"
#include <algorithm>



// constructors ///////////////////////////////////////////////////////////////////////////////////



Generator::Generator(double periodOfGeneration, std::list<Car*>* pActiveCarsInModel, 
	Car* pCarTemplate, Road* pRoad)
{
	this->periodOfGeneration = periodOfGeneration;
	this->pActiveCarsInModel = pActiveCarsInModel;
	this->pGeneratedCarTemplate = pCarTemplate;
	this->timePassed = 0.0;
	this->pNextGenerator = nullptr;
	this->pRoad = pRoad;
	this->hasGeneratedCar = false;
	this->timeOfLastGeneration = 0.0;

	if (isInitializedProperly())
	{
		successfulInit = true;
	}
	else
	{
		successfulInit = false;
	}
}



Generator::Generator(double periodOfGeneration, std::list<Car*>* pActiveCarsInModel, Car* pCarTemplate,
	Road* pRoad, Generator* pNextGenerator)
{
	this->periodOfGeneration = periodOfGeneration;
	this->pActiveCarsInModel = pActiveCarsInModel;
	this->pGeneratedCarTemplate = pCarTemplate;
	this->timePassed = 0.0;
	this->pNextGenerator = pNextGenerator;
	this->pRoad = pRoad;
	this->hasGeneratedCar = false;
	this->timeOfLastGeneration = 0.0;

	if (isInitializedProperly())
	{
		successfulInit = true;
	}
	else
	{
		successfulInit = false;
	}
}



// destructor /////////////////////////////////////////////////////////////////////////////////////



Generator::~Generator()
{
	if (pNextGenerator != nullptr)
		delete pNextGenerator;

	// delete cars which have not been added to the simulation
	int numOfCarsInQueue = carsQueue.size();
	for (int i = 0; i < numOfCarsInQueue; i++)
		delete carsQueue[i];

	if (pGeneratedCarTemplate != nullptr)
		delete pGeneratedCarTemplate;
}



// methods ////////////////////////////////////////////////////////////////////////////////////////



void Generator::reset()
{
	timePassed = 0.0;
	timeOfLastGeneration = 0.0;
	for (Car* pCar : carsQueue)
		delete pCar;

	carsQueue.clear();
	hasGeneratedCar = false;

	pGeneratedCarTemplate->setEvaluated(false);
	if (pNextGenerator != nullptr)
		pNextGenerator->pGeneratedCarTemplate->setEvaluated(false);
}



Generator* Generator::obtainFromString(std::string& type, std::vector<std::string>& parameters)
{
	if (type.compare(UNIFORM_GENERATOR_STRING) == 0)
		return UniformGenerator::obtainFromString(parameters);

	return nullptr;
}



int Generator::tick(double duration)
{
	// check if pNextGenerator xor pRoad is initialized
	if (successfulInit)
	{
		pushCarFromQueue(); // if there are cars waiting in the queue
		if (shouldGenerateNow())
		{
			Car* pGeneratedCar = generate();
			timeOfLastGeneration = timePassed;
			pushCarFurther(pGeneratedCar);
			if (!hasGeneratedCar)
			{
				if (pFirstGenerationListener != nullptr)
					pFirstGenerationListener->firstCarGenerated(pGeneratedCar);
			}
			
			hasGeneratedCar = true;
		}

		if (pNextGenerator != nullptr)
			pNextGenerator->tick(duration);

		if (timePassed - timeOfLastGeneration > periodOfGeneration) // if period finished reset the generator
		{
			timePassed = 0.0;
			timeOfLastGeneration = 0.0;
		}
		else
			timePassed += duration;

		return TICK_GENERATOR_SUCCESS;
	}
	else
		return TICK_GENERATOR_WRONG_INIT;
}



bool Generator::isInitializedProperly()
{
	return periodOfGeneration > 0.0 && pActiveCarsInModel != nullptr &&
		pGeneratedCarTemplate != nullptr && pGeneratedCarTemplate->getPosition() < 0.0 &&
		pRoad != nullptr && pGeneratedCarTemplate->getFirstRoad() == pRoad;
}



void Generator::pushCarFurther(Car* pCar)
{
	if (pNextGenerator != nullptr) // is not the last generator
	{
		pNextGenerator->pushCarFurther(pCar); // pass the generated car to the next generator
	}
	else // is the last generator
	{
		if (!carsQueue.empty())
		{
			if (carsQueue.front()->putOnStart() == PUT_ON_START_OK)	// try putting the first car in the queue on the road
			{
				pActiveCarsInModel->push_back(carsQueue.front()); // give control over the car to the model
				carsQueue.erase(carsQueue.begin()); // remove the car from the queue
			}

			carsQueue.push_back(pCar);
		}
		else // no cars in queue -> try to put the car on the road
		{
			if (pCar->putOnStart() == PUT_ON_START_OK)
				pActiveCarsInModel->push_back(pCar); // give control over the car to the model
			else
				carsQueue.push_back(pCar); // couldn't put car on the road -> add it to the queue
		}
	}
}



void Generator::pushCarFromQueue()
{
	if (!carsQueue.empty())
	{
		Car* pCar = carsQueue.front();
		if (pNextGenerator == nullptr)
		{
			if (pCar->putOnStart() == PUT_ON_START_OK)	// try putting the first car in the queue on the road
			{
				pActiveCarsInModel->push_back(carsQueue.front()); // give control over the car to the model
				carsQueue.erase(carsQueue.begin()); // remove the car from the queue
			}
		}
		else
		{
			pNextGenerator->pushCarFurther(pCar);
		}
	}
}



void Generator::registerFirstGenerationListener(FirstCarGenerationListener* pListener)
{
	this->pFirstGenerationListener = pListener;
	if (pNextGenerator != nullptr)
		pNextGenerator->registerFirstGenerationListener(pListener);
}



void Generator::startGeneratingCarsForEvaluation()
{
	if (pGeneratedCarTemplate != nullptr)
		pGeneratedCarTemplate->setEvaluated(true);

	if (pNextGenerator != nullptr)
		pNextGenerator->startGeneratingCarsForEvaluation();
}



int Generator::getNumOfReadyCars()
{
	return (int) carsQueue.size();
}



// getters && setters /////////////////////////////////////////////////////////////////////////////



bool Generator::setGeneratedCarTemplate(Car* pCarTemplate)
{
	if (pRoad != nullptr && pCarTemplate->getFirstRoad() == pRoad)
	{
		// if the car has to big speed then decrease it to the road speed limit
		double speed = min(pCarTemplate->getSpeed(), pRoad->speedLimitAt(0.0));
		pCarTemplate->setSpeed(speed);
		pCarTemplate->setSourceGenerator(this);
		this->pGeneratedCarTemplate = pCarTemplate;

		if (isInitializedProperly())
			successfulInit = true;
		else
			successfulInit = false;

		return true;
	}
	else
		return false;
}



Car* Generator::getGeneratedCarTemplate()
{
	return pGeneratedCarTemplate;
}



Road* Generator::getGeneratedCarTemplateFirstRoad()
{
	if (pGeneratedCarTemplate != nullptr)
		return pGeneratedCarTemplate->getFirstRoad();
	else
		return nullptr;
}



bool Generator::setRoad(Road* pRoad)
{
	if (pGeneratedCarTemplate == nullptr)
	{
		this->pRoad = pRoad;

		if (pNextGenerator != nullptr)
			pNextGenerator->setRoad(pRoad);

		successfulInit = isInitializedProperly();

		return true;
	}
	else
		return false;
}



Road* Generator::getRoad()
{
	return pRoad;
}



void Generator::setActiveCarsInModel(std::list<Car*>* pActiveCarsInModel)
{
	this->pActiveCarsInModel = pActiveCarsInModel;
	
	this->successfulInit = isInitializedProperly();
}



void Generator::setNextGenerator(Generator* pNextGenerator)
{
	if (this->pNextGenerator != nullptr)
		delete this->pNextGenerator;

	this->pNextGenerator = pNextGenerator;
}