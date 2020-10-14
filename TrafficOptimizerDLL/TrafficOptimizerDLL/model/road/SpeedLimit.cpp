#include "../../pch.h"
#include "SpeedLimit.h"

/*
	Represents speed limit on a particular section of a road.
*/
SpeedLimit::SpeedLimit(double startPos, double endPos, double value)
	: startPos(startPos), endPos(endPos), value(value) {}



// getters && setters //////////////////////////////////////////////////////////////



double SpeedLimit::getStartPos()
{
	return startPos;
}



double SpeedLimit::getEndPos()
{
	return endPos;
}



double SpeedLimit::getValue()
{
	return value;
}
