#pragma once
#include "Gate.h"
#include "../../road/Road.h"
#include <vector>


// const //////////////////////////////////////////////////////////////////////////////////////////
#define CREATE_GATE_SUCCESS 0
#define ADD_GATE_INCOMPATIBLE_ROAD 1



class CompoundGate : public Gate
{
public:
	// constructors ///////////////////////////////////////////////////////////////////////////////
	CompoundGate(Road* pRoad = nullptr) : Gate(pRoad) {};
	~CompoundGate();
	// methods ////////////////////////////////////////////////////////////////////////////////////
	bool isGateOpen(Car* pCar) override;
	int addGate(Gate* pComponentGate);
	// getters && setters
	void setRoad(Road* pRoad) override;
	std::vector<Gate*>& getComponents();
private:
	std::vector<Gate*> componentGates;
};
