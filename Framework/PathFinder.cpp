#include "stdafx.h"
#include "PathFinder.h"

#define MAX_MAP_WIDTH 300
#define MAX_MAP_HEIGHT 300
#define MAX_LOOP_TIMES 1000 

PathFinder::PathFinder()
{
	mMap = nullptr;
	mMinHeap = [this](CellPos& first, CellPos& second) {
		return mMapInfo[first.x][first.y].f > mMapInfo[second.x][second.y].f;
	};
	//allocate space
	mMapInfo.resize(MAX_MAP_WIDTH);
	for (auto& row : mMapInfo)
	{
		row.resize(MAX_MAP_HEIGHT);
	}
	mOpenlist.reserve(MAX_LOOP_TIMES * 8);
	mModify.reserve(MAX_LOOP_TIMES * 8);
}

bool PathFinder::InitMap(Map* map, const MapPos& src, const MapPos& dst)
{
	int mapWidth = map->Width();
	int mapHeight = map->Height();
	if (mapWidth > MAX_MAP_WIDTH || mapHeight > MAX_MAP_HEIGHT)
	{
		printf("error : map too big , id:%d width:%d height:%d\n", map->ID(), mapWidth, mapHeight);
		return false;
	}
	if (map->IsBlock(src.x, src.y)|| map->IsBlock(dst.x, dst.y))
		return false;
	//clear old data
	for (auto m : mModify)
	{
		PointInfo& info = *m;
		info.state = PS_NONE;
	}
	mModify.clear();
	mOpenlist.clear();
	mSrc = src;
	mDst = dst;
	mSrcCell = (CellPos)src;
	mDstCell = (CellPos)dst;
	mMap = map;
	return true;
}

bool PathFinder::GetPath(Map* map, const MapPos& src, const MapPos& dst, std::vector<MapPos>& path)
{
	if (src == dst)
		return false;
	if (map->CanGoStraight(src, dst))
	{
		path.push_back(dst);
		return true;
	}
	return GetAstarPath(map, src, dst, path);
}

bool PathFinder::GetAstarPath(Map* map, const MapPos& src, const MapPos& dst, std::vector<MapPos>& path)
{
	if (!InitMap(map, src, dst))
		return false;
	AddOpenList(mSrcCell, 0, CellPos(0, 0));
	bool findPath = false;
	int loop = MAX_LOOP_TIMES;
	while (--loop)
	{
		if (mOpenlist.empty())//fail
			break;
		CellPos oldPos = PopOpenList();
		if (oldPos == mDstCell)//success
		{
			findPath = true;
			break;
		}
		mMapInfo[oldPos.x][oldPos.y].state = PS_CLOSE;
		for (int i = -1; i <= 1; ++i)
			for (int j = -1; j <= 1; ++j)
			{
				if (i == 0 && j == 0)
					continue;
				CellPos newPos(oldPos.x + i, oldPos.y + j);
				if (map->IsBlock(newPos.x, newPos.y))
					continue;
				PointInfo& oldInfo = mMapInfo[oldPos.x][oldPos.y];
				float g = 1.f;
				if (i != 0 && j != 0)
					g = 1.4f;
				AddOpenList(newPos, oldInfo.g + g, oldPos);
			}

	}
	if (findPath)
	{
		path = GetPathVec();
		return true;
	}
	else
	{
		printf("error:not find path , mapid:%d src(%f,%f) dst(%f,%f)\n", map->ID(), src.x, src.y, dst.x, dst.y);
		return false;
	}
}

void PathFinder::AddOpenList(const CellPos& pos, float g, const CellPos& parent)
{
	PointInfo& info = mMapInfo[pos.x][pos.y];
	if (info.state == PS_NONE)//new
	{
		info.state = PS_OPEN;
		info.g = g;
		info.h = CalcH(pos, mDstCell);
		info.f = info.g + info.h;
		info.parent = parent;
		mModify.push_back(&info);
		mOpenlist.push_back(pos);
		std::push_heap(mOpenlist.begin(), mOpenlist.end(), mMinHeap);
	}
	else if (info.state == PS_OPEN && g < info.g)//update
	{
		info.g = g;
		info.f = info.g + info.h;
		info.parent = parent;
		std::make_heap(mOpenlist.begin(), mOpenlist.end(), mMinHeap);
	}
}

CellPos PathFinder::PopOpenList()
{
	std::pop_heap(mOpenlist.begin(), mOpenlist.end(), mMinHeap);
	CellPos ret = mOpenlist.back();
	mOpenlist.pop_back();
	return ret;
}

int PathFinder::CalcH(const CellPos& src, const CellPos& dst)
{
	return std::abs(dst.x - src.x) + std::abs(dst.y - src.y);
}

std::vector<MapPos> PathFinder::GetPathVec()
{
	std::vector<MapPos> ret;

	//get path
	std::list<MapPos> path;
	path.push_front(mDst);
	CellPos cell = mMapInfo[mDstCell.x][mDstCell.y].parent;
	while (cell != mSrcCell)
	{
		path.emplace_front(cell.ToPixX(), cell.ToPixY());
		cell = mMapInfo[cell.x][cell.y].parent;
	}
	path.push_front(mSrc);

	//optimize path for pixel 
	if (path.size() > 2)
	{
		auto it1 = path.begin();
		auto it2 = ++it1;
		auto it3 = ++it1;
		it1 = path.begin();
		for (; it3 != path.end(); ++it3)
		{
			if (IsLine(it1->x, it1->y, it2->x, it2->y, it3->x, it3->y)
				|| mMap->CanGoStraight(*it1, *it3))
			{
				path.erase(it2);
				it2 = it3;
			}
			else
			{
				++it1;
				++it2;
			}
		}
	}
	path.pop_front();

	for (auto& pos : path)
	{
		ret.push_back(pos);
	}
	return ret;
}
