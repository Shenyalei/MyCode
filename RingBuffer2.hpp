//lock free ring buffer 
//for multi producer and multi consumer

#include <windows.h>

template<class T>
class Buffer2 {
public:
	Buffer2(int size)
	{
		int s = 1;
		while (s < size)
		{
			s * = 2;
		}
		mCapacity = s;
		mData = new (*T)[mCapacity];
	}
	bool push(T* data);
	bool pop(T* data);
private:
	T** mData;
	int mWriteCnt;
	int mReadCnt;
	int mCapacity;
};

template <class T>
bool Buffer2<T>::push(T* data)
{
	int in = 0;
	int out = 0;

	do {
		in = mWriteCnt;
		out = mReadCnt;

		if (in - out == mCapacity) {
			return false;
		}

	} while (!InterlockedCompareExchange(&mWriteCnt, in +1, in))

	mData[in&(mCapacity - 1)] = data;

	return true;
}

template <class T>
bool Buffer2<T>::pop(T* data)
{
	int in = 0;
	int out = 0;

	do {
		in = mWriteCnt;
		out = mReadCnt;

		if (out == in) {
			return false;
		}
	} while (!InterlockedCompareExchange(&mReadCnt, out+1,out))
	data = mData[out&(mCapacity - 1)];
	return true;
}