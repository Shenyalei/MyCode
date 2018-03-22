#ifndef SHARE_RING_BUFFER_
#define SHARE_RING_BUFFER_
#include "Common.h"

//lock free ring buffer
//for one producer and one consumer
template<class T>
class RingBuffer
{
public:
	RingBuffer(int size)
	{
		int s = 1;
		while (s < size)
		{
			s *= 2;
		}
		mCapacity = s;
		mData = new T[mCapacity];
		mWriteCnt = 0;
		mReadCnt = 0;
	};
	~RingBuffer()
	{
		delete []mData;
		mData = nullptr;
	}
	bool push(const T& data)
	{
		if (Full())
			return false;
		mData[WritePos()] = data;
		++mWriteCnt;
		return true;
	}
	void pop()
	{
		if (Empty())
			return ;
		++mReadCnt;
	}
	T& back()
	{
		return mData[ReadPos()];
	}
	UINT Size()
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
private:
	UINT ReadPos()
	{
		return mReadCnt & (mCapacity - 1);
	}
	UINT WritePos()
	{
		return mWriteCnt & (mCapacity - 1);
	}

	T* mData;
	UINT mCapacity;
	std::atomic<UINT> mWriteCnt;
	std::atomic<UINT> mReadCnt;
};

template<>
class RingBuffer<char>
{
public:

	UINT Size()
	{
		return mWriteCnt - mReadCnt;
	}

	UINT get(char* buf, UINT len)
	{

		UINT l = 0;
		len = min(len, Size());

		//smp_rmb();

		l = min(len, mCapacity - ReadPos());
		memcpy(buf, mData + ReadPos(), l);
		memcpy(buf + l,mData, len - l);

		//smp_mb();

		mWriteCnt += len;
		return len;
	}
	UINT put(char* buf,UINT len)
	{
		UINT l = 0;
		len = min(len, mCapacity - Size());

		//smp_mb();

		l = min(len, mCapacity - WritePos());
		memcpy(mData + WritePos(), buf, l);
		memcpy(mData, buf + l, len - l);

		//smp_wmb();

		mReadCnt += len;
		return len;
	}
private:
	UINT ReadPos()
	{
		return mReadCnt & (mCapacity - 1);
	}
	UINT WritePos()
	{
		return mWriteCnt & (mCapacity - 1);
	}
	char* mData;
	UINT mCapacity;
	std::atomic<UINT> mWriteCnt;
	std::atomic<UINT> mReadCnt;
};

//spin lock ring buffer 
//for multi producer and multi consumer
template<class T>
class RingBuffer2 {
public:
	RingBuffer2(int size)
	{
		int s = 1;
		while (s < size)
		{
			s *= 2;
		}
		mCapacity = s;
		mData = new T[mCapacity];
	}
	~RingBuffer2()
	{
		delete[]mData;
		mData = nullptr;
	}
	bool push(const T& data);
	bool pop(T& data);
private:
	T* mData;
	UINT mCapacity;

	std::atomic<UINT> mWriteCnt;
	std::atomic<UINT> mReadCnt;

};

template <class T>
bool RingBuffer2<T>::push(const T& data)
{
	UINT in = 0;
	UINT out = 0;

	do {
		in = mWriteCnt;
		out = mReadCnt;

		if (in - out == mCapacity) {
			return false;
		}

	} while (!mWriteCnt.compare_exchange_strong(in,in + 1))

	mData[in&(mCapacity - 1)] = data;

	return true;
}

template <class T>
bool RingBuffer2<T>::pop(T& data)
{
	UINT in = 0;
	UINT out = 0;

	do {
		in = mWriteCnt;
		out = mReadCnt;

		if (out == in) {
			return false;
		}
	} while (!mReadCnt.compare_exchange_strong(out,out + 1))
		data = mData[out&(mCapacity - 1)];
	return true;
}


#endif // SHARE_RING_BUFFER_
