#include "../pch.h"
#include "SimplifiedLights.h"



// methods ////////////////////////////////////////////////////////////////////////////////////



bool SimplifiedLights::isGreen()
{
	return green;
}



void SimplifiedLights::setGreen()
{
	green = true;
}



void SimplifiedLights::setRed()
{
	green = false;
}