#pragma once




namespace mov {

	/*
		Calculates distance required to stop with given initial speed and acceleration.
		@speed	- initial speed
		@acc	- acceleration (braking), must be != 0
	*/
	double brakingDistance(double initialSpeed, double finalSpeed, double acc);
	/*
		Calculates distance which will be covered in specified time, but ONLY FORWARD!
		This means, that if the speed is very small and the acc is big, then from the
		pure equation the distance should be negative, but in this case it will be 0
	*/
	double distance(double speed, double acc, double time);

	/*
		Calculates speed after given time, with given acceleration and initial speed. Returns
		ONLY NON NEGATIVE values, if the speed should be < 0 then it will be 0.0
	*/
	double speed(double currSpeed, double acc, double time);

	double time(double currSpeed, double dist, double acc);
}
