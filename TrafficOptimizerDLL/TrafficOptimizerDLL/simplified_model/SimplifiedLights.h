#pragma once



/*
	Represents traffic lights. It can be either green or red.
*/
class SimplifiedLights
{
public:
	// methods ////////////////////////////////////////////////////////////////////////////////////
	bool isGreen();
	void setGreen();
	void setRed();

private:
	bool green;
};
