#pragma once
#define FLT_EPSILON		1.192092896e-07F

inline bool FloatEqual(float f1, float f2)
{
	return (f1 < (f2 + FLT_EPSILON)) && (f1 > (f2 - FLT_EPSILON));
}

inline bool IsLine(float x1, float y1, float x2, float y2, float x3, float y3)
{
	return FloatEqual((y2 - y1) * (x3 - x2), (x2 - x1) * (y3 - y2));
}

template<typename T>
std::set<T> GetOutSet(std::set<T>& s1, std::set<T>& s2)
{
	if (s1.empty())
		return s2;
	std::set<T> ret;
	for (const auto& v : s2)
	{
		if (s1.find(v) == s1.end())
			ret.insert(v);
	}
	return ret;
}

inline std::vector<RECT> GetOutRect(RECT& r1, RECT& r2)
{
	std::vector<RECT> ret;
	RECT intersec;
	if (!IntersectRect(&intersec, &r1, &r2))
	{
		ret.push_back(r2);
		return ret;
	}
	RECT r;
	if (intersec.left == r2.left)
	{
		if (intersec.right != r2.right)
		{
			r.top = r2.top;
			r.bottom = r2.bottom;
			r.left = intersec.right + 1;
			r.right = r2.right;
			ret.push_back(r);
		}
	}
	else if (intersec.right == r2.right)
	{
		r.top = r2.top;
		r.bottom = r2.bottom;
		r.left = r2.left;
		r.right = intersec.left - 1;
		ret.push_back(r);
	}

	if (intersec.top == r2.top)
	{
		if (intersec.bottom != r2.bottom)
		{
			r.left = intersec.left;
			r.right = intersec.right;
			r.top = intersec.bottom + 1;
			r.bottom = r2.bottom;
			ret.push_back(r);
		}
	}
	else if (intersec.bottom == r2.bottom)
	{
		r.left = intersec.left;
		r.right = intersec.right;
		r.top = r2.top;
		r.bottom = intersec.top - 1;
		ret.push_back(r);
	}
	return ret;
}