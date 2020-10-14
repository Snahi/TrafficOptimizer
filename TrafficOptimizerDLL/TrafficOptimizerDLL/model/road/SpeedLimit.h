#pragma once
class SpeedLimit
{
public:
	// constructors ///////////////////////////////////////////////////////////////////////////////////
	SpeedLimit(double startPos, double endPos, double value);
	double getStartPos();
	double getEndPos();
	double getValue();

private:
	// fields /////////////////////////////////////////////////////////////////////////////////////////
	double startPos;	// position on a road from which the speed limit is in force.
	double endPos;		// position on a road to which the speed limit is in force.
	double value;		// the actual value of the speed limit, i.e. 10 m/s

};

