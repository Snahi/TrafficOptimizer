#pragma once
#include "StopCondition.h"
#include <chrono>



class TimeSC : public StopCondition
{
public:
	TimeSC(double durationSeconds) : StopCondition(nullptr)
	{
		long durationInMillis = (long) (durationSeconds * 1000);
		std::chrono::milliseconds duration = std::chrono::milliseconds(durationInMillis);
		std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch());
		
		startTS = now;
		endTS = now + duration;
	}

	virtual bool notMet() override
	{
		std::chrono::milliseconds currTime = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch());

		return currTime < endTS;
	}

	virtual float progress() override
	{
		std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch());

		return (float)(now.count() - startTS.count()) / (endTS.count() - startTS.count());
	}

private:
	std::chrono::milliseconds endTS;
	std::chrono::milliseconds startTS;
};
