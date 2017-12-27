#ifndef SHARE_SINGLETON_
#define SHARE_SINGLETON_

//֧�ֶ��̵߳ĵ���ģʽ

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
