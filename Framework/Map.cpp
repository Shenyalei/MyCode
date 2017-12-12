#include "stdafx.h"
#include "Map.h"
#include "Object.h"

void Map::OnMove(Object* obj, int newx, int newy, int oldx, int oldy)
{
	int viewDistance = obj->GetAoiRadius();
	int mapWidth = Width();
	int mapHeight = Height();
	RECT oldRect = { max(oldx - viewDistance, 0),max(oldy - viewDistance, 0), min(oldx + viewDistance, mapWidth),min(oldy + viewDistance, mapHeight) };
	RECT newRect = { max(newx - viewDistance, 0), max(newy - viewDistance, 0),min(newx + viewDistance, mapWidth),min(newy + viewDistance, mapHeight) };
	auto inRectList = GetOutRect(oldRect, newRect);
	for (const auto& r : inRectList)
	{
		for (int y = r.top; y <= r.bottom; ++y)
		{
			for (int x = r.left; x <= r.right; ++x)
			{
				Cell* cell = GetCell(x, y);
				if (cell)
				{
					for (auto enterObj: cell->objectList)
					{
						obj->OnEnterAoi(enterObj);
					}
					cell->observerList.insert(obj);
				}
			}
		}
	}
	auto outRectList = GetOutRect(newRect, oldRect);
	for (const auto& r:outRectList)
	{
		for (int y = r.top; y <= r.bottom; ++y)
		{
			for (int x = r.left; x <= r.right; ++x)
			{
				Cell* cell = GetCell(x, y);
				if (cell)
				{
					for (auto leaveObj:cell->objectList)
					{
						if (leaveObj == obj) continue;
						obj->OnLeaveAoi(leaveObj);
					}
					cell->observerList.erase(obj);
				}
			}
		}
	}

	std::set<Object*> oldObserver;
	std::set<Object*> newObserver;
	Cell* oldCell = GetCell(oldx, oldy);
	if (oldCell)
		oldObserver = oldCell->observerList;
	Cell* newCell = GetCell(newx, newy);
	if (newCell)
		newObserver = newCell->observerList;
	auto inObserver = GetOutSet(oldObserver, newObserver);
	for (auto other : inObserver)
	{
		if (other == obj) continue;
		other->OnEnterAoi(obj);
	}
	auto outObserver = GetOutSet(newObserver, oldObserver);
	for (auto other : outObserver)
	{
		other->OnLeaveAoi(obj);
	}
}