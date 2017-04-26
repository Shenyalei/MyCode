//ring buffer
//for one producer and one consumer
#pragma once
#include "stdafx.h"

template<class T>
class Buffer
{
public:
	Buffer(int size)
	{
		int s = 1;
		while (s < size)
		{
			s * = 2;
		}
		mCapacity = s;
		mData = new (*T)[mCapacity];
		mWriteCnt = 0;
		mReadCnt = 0;
	};
	bool push(T* data)
	{
		if (Full())
			return false;
		mData[WritePos()] = data;
		++mWriteCnt;
		return true;
	}
	T* pop()
	{
		if (Empty())
			return nullptr;
		T* ret = mData[ReadPos()];
		++mReadCnt;
		return ret;
	}
private:
	int ReadPos()
	{
		return mReadCnt & (mCapacity -1);
	}
	int WritePos()
	{
		return mWriteCnt & (mCapacity -1);
	}
	int Size()
	{
		return mWriteCnt - mReadCnt;
	}
	bool Empty()
	{
		return Size() == 0;
	}
	bool Full()
	{
		return Size() == mCapacity;
	}

	T* mData;
	int mCapacity;
	volatile unsigned int mWriteCnt;
	volatile unsigned int mReadCnt;
};

