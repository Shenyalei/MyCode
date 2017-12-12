#pragma once
#define LENGTH_OF_CELL 40.f

class MapPos
{
public:
	float x;
	float y;
	MapPos() :x(0), y(0) {}
	MapPos(float a, float b) :x(a), y(b) {}
	MapPos(const MapPos& other) :x(other.x), y(other.y) {}
	MapPos& operator = (const MapPos& other)
	{
		if (other == *this)
			return *this;
		x = other.x;
		y = other.y;
		return *this;
	}
	float operator - (const MapPos& other) const
	{
		return sqrt((x - other.x)*(x - other.x) + (y - other.y)*(y - other.y));
	}
	bool operator == (const MapPos& other) const
	{
		return FloatEqual(x, other.x) && FloatEqual(y, other.y);
	}
	int ToCellX() const { return int(x / LENGTH_OF_CELL); }
	int ToCellY() const { return int(y / LENGTH_OF_CELL); }
};

class CellPos
{
public:
	int x;
	int y;
	CellPos() :x(0), y(0) {}
	CellPos(int a, int b) :x(a), y(b) {}
	explicit CellPos(const MapPos& pos) :x(pos.ToCellX()), y(pos.ToCellY()) {}
	CellPos& operator = (const CellPos& other)
	{
		if (other == *this)
			return *this;
		x = other.x;
		y = other.y;
		return *this;
	}
	bool operator == (const CellPos& other) const
	{
		return x == other.x && y == other.y;
	}
	bool operator != (const CellPos& other) const
	{
		return !(*this == other);
	}
	float ToPixX() const { return x*LENGTH_OF_CELL + LENGTH_OF_CELL / 2; }
	float ToPixY() const { return y*LENGTH_OF_CELL + LENGTH_OF_CELL / 2; }
};

class Object;

struct Cell
{
	std::set<Object*> objectList;
	std::set<Object*> observerList;
};
class Map
{
public:
	int ID() { return 0; }
	int Width() { return 0; }
	int Height() { return 0; }
	Cell* GetCell(int x, int y) { return nullptr; }
	bool IsBlock(int x, int y) { return false; }
	bool CanGoStraight(const MapPos& src, const MapPos& dst) { return false; }
	//AOI SYSTEM
	void OnMove(Object* obj, int newx, int newy, int oldx, int oldy);
};