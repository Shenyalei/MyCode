//ring buffer
//for one producer and one consumer
#pragma once
#include "stdafx.h"
#define MAX_BUFFER_SIZE 1024

template<class T>
class Buffer2
{
public:
	Buffer2()
	{
		mWriteCnt = 0;
		mReadCnt = 0;
	};
	void push(T* data)
	{
		if (Full())
			return;
		mData[WritePos()] = data;
		++mWriteCnt;
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
		return mReadCnt & (MAX_BUFFER_SIZE-1);
	}
	int WritePos()
	{
		return mWriteCnt & (MAX_BUFFER_SIZE-1);
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
		return Size() == MAX_BUFFER_SIZE;
	}

	T* mData[MAX_BUFFER_SIZE];
	volatile unsigned int mWriteCnt;
	volatile unsigned int mReadCnt;
};

