#ifndef SHARE_RING_BUFFER_
#define SHARE_RING_BUFFER_

//ring buffer
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
	bool push(const T& data)
	{
		if (Full())
			return false;
		mData[WritePos()] = data;
		++mWriteCnt;
		return true;
	}
	bool push(T&& data)
	{
		if (Full())
			return false;
		mData[WritePos()] = std::move(data);
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
private:
	int ReadPos()
	{
		return mReadCnt & (mCapacity - 1);
	}
	int WritePos()
	{
		return mWriteCnt & (mCapacity - 1);
	}

	T* mData;
	int mCapacity;
	volatile unsigned int mWriteCnt;
	volatile unsigned int mReadCnt;
};

//lock free ring buffer 
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
	bool push(const T& data);
	bool push(T&& data);
	bool pop(T& data);
private:
	T* mData;
	int mWriteCnt;
	int mReadCnt;
	int mCapacity;
};

template <class T>
bool RingBuffer2<T>::push(const T& data)
{
	int in = 0;
	int out = 0;

	do {
		in = mWriteCnt;
		out = mReadCnt;

		if (in - out == mCapacity) {
			return false;
		}

	} while (!InterlockedCompareExchange(&mWriteCnt, in + 1, in))

		mData[in&(mCapacity - 1)] = data;

	return true;
}

template <class T>
bool RingBuffer2<T>::push(T&& data)
{
	int in = 0;
	int out = 0;

	do {
		in = mWriteCnt;
		out = mReadCnt;

		if (in - out == mCapacity) {
			return false;
		}

	} while (!InterlockedCompareExchange(&mWriteCnt, in + 1, in))

		mData[in&(mCapacity - 1)] = std::move(data);

	return true;
}

template <class T>
bool RingBuffer2<T>::pop(T& data)
{
	int in = 0;
	int out = 0;

	do {
		in = mWriteCnt;
		out = mReadCnt;

		if (out == in) {
			return false;
		}
	} while (!InterlockedCompareExchange(&mReadCnt, out + 1, out))
		data = mData[out&(mCapacity - 1)];
	return true;
}

#endif // SHARE_RING_BUFFER_
