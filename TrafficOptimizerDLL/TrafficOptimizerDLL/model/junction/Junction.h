#pragma once
#include "lights/LightsSystem.h"
#include "gate/Gate.h"
#include "../road/Road.h"
#include "JunctionInput.h"
#include <vector>



/*
	Class which represents a junction
*/
class Junction
{
public:
	Junction(LightsSystem* pLightsSystem = nullptr);
	~Junction();
	void addJunctionInput(JunctionInput* pInput);
	bool hasInputs();
	// getters && setters
	void setLightsSystem(LightsSystem* pLightsSystem);
private:
	std::vector<JunctionInput*> inputs;
	LightsSystem* pLightsSystem;
};

