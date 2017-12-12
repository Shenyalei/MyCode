#pragma once
class Object
{
public:
	int GetAoiRadius() { return 0; }
	void OnEnterAoi(Object* obj);
	void OnLeaveAoi(Object* obj);
private:
	std::set<Object*> mObserverList;
	std::set<Object*> mVisibleList;
};