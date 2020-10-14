#include "../pch.h"
#include "MovementFunctions.h"
#include <math.h>
#include <limits>


using namespace std;



double mov::brakingDistance(double initialSpeed, double finalSpeed, double acc)
{
	double t = (finalSpeed - initialSpeed) / acc;
	return initialSpeed * t + acc * t * t / 2;
}



double mov::distance(double speed, double acc, double time)
{
	double bareDistance = speed * time + acc * time * time / 2;
	return bareDistance > 0.0 ? bareDistance : 0.0;
}



double mov::speed(double currSpeed, double acc, double time)
{
	double bareSpeed = currSpeed + acc * time;
	return bareSpeed > 0 ? bareSpeed : 0.0;
}



double mov::time(double initSpeed, double dist, double acc)
{
	double delta = initSpeed * initSpeed + 2 * acc * dist;
	delta = delta >= 0 ? delta : 0.0;
	if (acc != 0)
		return (sqrt(delta) - initSpeed) / acc;
	else
	{
		if (initSpeed != 0)
			return dist / initSpeed;
		else
			return DBL_MAX;
	}
}