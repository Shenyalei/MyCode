#pragma once
#define FLT_EPSILON		1.192092896e-07F
#define LENGTH_OF_CELL 40.f

bool util_float_equal(float f1, float f2)
{
	return (f1 < (f2 + FLT_EPSILON)) && (f1 > (f2 - FLT_EPSILON));
}

bool util_is_line(float x1, float y1, float x2, float y2, float x3, float y3)
{
	return util_float_equal((y2 - y1) * (x3 - x2), (x2 - x1) * (y3 - y2));
}

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
		return util_float_equal(x, other.x) && util_float_equal(y, other.y);
	}
	int ToCellX() const { return int(x / LENGTH_OF_CELL); }
	int ToCellY() const { return int(y / LENGTH_OF_CELL); }
};

class MapCellPos
{
public:
	int x;
	int y;
	MapCellPos() :x(0), y(0) {}
	MapCellPos(int a, int b) :x(a), y(b) {}
	explicit MapCellPos(const MapPos& pos) :x(pos.ToCellX()), y(pos.ToCellY()) {}
	MapCellPos& operator = (const MapCellPos& other)
	{
		if (other == *this)
			return *this;
		x = other.x;
		y = other.y;
		return *this;
	}
	bool operator == (const MapCellPos& other) const
	{
		return x == other.x && y == other.y;
	}
	bool operator != (const MapCellPos& other) const
	{
		return !(*this == other);
	}
	float ToPixX() const { return x*LENGTH_OF_CELL + LENGTH_OF_CELL / 2; }
	float ToPixY() const { return y*LENGTH_OF_CELL + LENGTH_OF_CELL / 2; }
};

class CMap
{
public:
	int MapID() { return 0; }
	int Width() { return 0; }
	int Height() { return 0; }
	bool IsCellBlock(int x, int y) { return false; }
	bool CanGoStraight(const MapPos& src, const MapPos& dst) { return false; }
};