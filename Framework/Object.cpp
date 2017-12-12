#include "stdafx.h"
#include "Object.h"

void Object::OnEnterAoi(Object* obj)
{
	mVisibleList.insert(obj);
	obj->mObserverList.insert(this);
}

void Object::OnLeaveAoi(Object* obj)
{
	mVisibleList.erase(obj);
	obj->mObserverList.erase(this);
}
