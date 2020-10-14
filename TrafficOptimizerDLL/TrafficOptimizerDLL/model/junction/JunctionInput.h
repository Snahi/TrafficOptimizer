#pragma once
#include <vector>
#include "gate/Gate.h"
#include "../road/Road.h"
#include "../Car.h"


class Gate;
class Road;


/*
	Junction input is the point where a road meets with a junction. Only one road can be
	associated with a junction, but it's not enforced through implementation of this class, it's
	just a rule to be obeyed. Junction contains gates which lead to roads which are accessible 
	from the junction input, because often we can choose many roads to go from a single road when
	we are at the junciton, e.g. straight, left.
*/
class JunctionInput 
{
public:
	// constructors ///////////////////////////////////////////////////////////////////////////////
	JunctionInput() : pInputRoad{ nullptr } {};
	~JunctionInput();
	// method /////////////////////////////////////////////////////////////////////////////////////
	void addGate(Gate* pGate);
	void setInputRoad(Road* pInputRoad);
	Road* getInputRoad();
	/*
		Finds gate which guards access to the specified road.
	*/
	virtual Gate* gateToRoad(Road* pRoad);
	std::vector<Gate*>& getAllGates();
	bool hasGates();
private:
	std::vector<Gate*> gates;
	Road* pInputRoad;
};



