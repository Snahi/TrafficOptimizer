#include "../../pch.h"
#include "MultiLaneRoad.h"



int MultiLaneRoad::addLane(Road* pLane)
{
	int res = validateLane(pLane);

	if (res == ADD_LANE_SUCCESS)
	{
		if (!lanes.empty()) // if there are some lanes
		{
			// update right neighbour pointer of the current rightmost lane
			lanes.back()->setRightNeighbour(pLane);
			// update left neighbour pointer of the new lane
			pLane->setLeftNeighbour(lanes.back());
		}

		lanes.push_back(pLane);
	}

	return res;
}



int MultiLaneRoad::validateLane(Road* pLane)
{
	if (pLane == nullptr)
		return ADD_LANE_ERR_NULL_LANE;

	if (pLane->isLane())
		return ADD_LANE_ERR_LANE_IS_PART_OF_OTHER_MULTI_LANE_ROAD;

	if (laneExists(pLane))
		return ADD_LANE_ERR_LANE_ALREADY_EXISTS;

	return ADD_LANE_SUCCESS;
}



bool MultiLaneRoad::laneExists(Road* pLane)
{
	for (Road* l : lanes)
	{
		if (l == pLane)
			return true;
	}

	return false;
}
