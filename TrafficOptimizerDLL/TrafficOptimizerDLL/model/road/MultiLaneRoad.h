#pragma once

#include <vector>
#include "Road.h"

// constants //////////////////////////////////////////////////////////////////////////////////////
// addLane
#define ADD_LANE_SUCCESS 0
#define ADD_LANE_ERR_LANE_ALREADY_EXISTS 1
#define ADD_LANE_ERR_LANE_IS_PART_OF_OTHER_MULTI_LANE_ROAD 3
#define ADD_LANE_ERR_NULL_LANE 4

/*
	Class for representing roads which consists of more lanes. Like that:

	|     |     |     |    |
	|        _             |
	|     | | | |     |    |
	|       |_|    _       |
	|  _  |     | | | |    |
	| | |         |_|      |
	| |_| |     |     |    |

	The class itself does not act like a road, it just groups them and sets their neighbour
	road, so that cars which are on that road can change lane. In this system the class Road acts
	also as a lane.
*/
class MultiLaneRoad
{
public:
	/*
		Adds a new lane to the on the right side. The function updates neighbour lanes of the current
		lanes and of the added one.

		Returns:
		ADD_LANE_SUCCESS									- success
		ADD_LANE_ERR_LANE_ALREADY_EXISTS					- the lane is already pard of the road
		ADD_LANE_ERR_LANE_IS_PART_OF_OTHER_MULTI_LANE_ROAD	- the lane has at least one neighbour set
		ADD_LANE_ERR_NULL_LANE								- the lane is nullptr
	*/
	int addLane(Road* pRoad);
private:
	/*
		Holds lanes which are part of the multi lane road. The first lane is the leftmost lane,
		the last lane is the rightmost lane.
	*/
	std::vector<Road*> lanes;
	// methods
	/*
		Returns:
			true	- the lane is part of this multi-lane road
			false	- the lane is not part of this multi-lane road
	*/
	bool laneExists(Road* pLane);
	/*
		Checks if the lane can be added to this multilane road. Returns are compatible with 
		addLane() returns.

		Returns:
		ADD_LANE_SUCCESS									- success
		ADD_LANE_ERR_LANE_ALREADY_EXISTS					- the lane is already pard of the road
		ADD_LANE_ERR_LANE_IS_PART_OF_OTHER_MULTI_LANE_ROAD	- the lane has at least one neighbour set
		ADD_LANE_ERR_NULL_LANE								- the lane is nullptr
	*/
	int validateLane(Road* pLane);
};

