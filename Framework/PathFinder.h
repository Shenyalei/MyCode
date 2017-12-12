#pragma once
#include "Map.h"

class CellPos;
class MapPos;
class Map;

class PathFinder
{
public:
	enum PointState
	{
		PS_NONE = 0,
		PS_OPEN = 1,
		PS_CLOSE = 2,
	};
	struct PointInfo
	{
		PointInfo()
		{
			f = 0.f;
			g = 0.f;
			h = 0.f;
			parent.x = 0;
			parent.y = 0;
			state = PS_NONE;
		}
		float f = 0;
		float g = 0;
		float h = 0;
		CellPos parent;
		PointState state = PS_NONE;
	};
	PathFinder();
	bool GetPath(Map* map, const MapPos& src, const MapPos& dst, std::vector<MapPos>& path);
private:
	bool InitMap(Map* map, const MapPos& src, const MapPos& dst);
	void AddOpenList(const CellPos& pos, float g, const CellPos& parent);
	CellPos PopOpenList();
	int CalcH(const CellPos& src, const CellPos& dst);
	bool GetAstarPath(Map* map, const MapPos& src, const MapPos& dst, std::vector<MapPos>& path);
	std::vector<MapPos> GetPathVec();
private:
	std::vector<std::vector<PointInfo>> mMapInfo;
	std::vector<CellPos> mOpenlist;
	MapPos mDst;
	MapPos mSrc;
	CellPos mSrcCell;
	CellPos mDstCell;
	Map* mMap;
	std::function<bool(CellPos&, CellPos&)> mMinHeap;
	std::vector<PointInfo*> mModify;
};