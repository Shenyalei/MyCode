#pragma once
#include "Map.h"

class MapCellPos;
class MapPos;
class CMap;

class PathFinding
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
		MapCellPos parent;
		PointState state = PS_NONE;
	};
	PathFinding();
	bool GetPath(CMap* map, const MapPos& src, const MapPos& dst, std::vector<MapPos>& path);
private:
	bool InitMap(CMap* map, const MapPos& src, const MapPos& dst);
	void AddOpenList(const MapCellPos& pos, float g, const MapCellPos& parent);
	MapCellPos PopOpenList();
	int CalcH(const MapCellPos& src, const MapCellPos& dst);
	bool GetAstarPath(CMap* map, const MapPos& src, const MapPos& dst, std::vector<MapPos>& path);
	std::vector<MapPos> GetPathVec();
private:
	std::vector<std::vector<PointInfo>> mMapInfo;
	std::vector<MapCellPos> mOpenlist;
	MapPos mDst;
	MapPos mSrc;
	MapCellPos mSrcCell;
	MapCellPos mDstCell;
	CMap* mMap;
	std::function<bool(MapCellPos&, MapCellPos&)> mMinHeap;
	std::vector<PointInfo*> mModify;
};