#ifndef SHARE_SINGLETON_
#define SHARE_SINGLETON_

//支持多线程的单例模式

template<class T>
class Singleton
{
public:
	static T& GetInstance()
	{
		static T instance;
		return instance;
	}
protected:
	Singleton() {}
	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;
};








#endif // SHARE_SINGLETON_
