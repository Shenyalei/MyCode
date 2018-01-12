#ifndef NETWORK_IOTHREADM_
#define NETWORK_IOTHREADM_
#include "RingBuffer.h"
class Connection;

class ActionBase : public OVERLAPPED
{
public:
	ActionBase();
	virtual ~ActionBase() {};
	virtual void Init();
	virtual bool OnComplete(DWORD num) = 0;
};

class AcceptAction :public ActionBase
{
public:
	AcceptAction();
	void Init() override;
	bool OnComplete(DWORD num) override;
	SOCKET acceptSocket;
};

class ConnectAction :public ActionBase
{
public:
	ConnectAction(SOCKET socket);
	~ConnectAction() override;
	bool OnComplete(DWORD num) override;
	SOCKET connSocket;
};

class RecvAction :public ActionBase
{
public:
	RecvAction(Connection* _conn) :conn(_conn) {}
	bool OnComplete(DWORD num) override;
	Connection* conn;
};

class SendAction :public ActionBase
{
public:
	SendAction(Connection* _conn) :conn(_conn) {}
	bool OnComplete(DWORD num) override;
	Connection* conn;
};


class IOThreadM : public Singleton<IOThreadM>
{
public:
	IOThreadM();
	~IOThreadM();
	bool Start();
	bool Listen(const std::string& ip,WORD port);
	bool Connect(const std::string& ip, WORD port);
	void AddSocket(SOCKET socket);
	void PostAccept(AcceptAction* action);
private:
	HANDLE	m_IOCP;
	std::vector<std::thread> m_workerThreads;
	SOCKET m_listenSocket;
};

#endif // NETWORK_IOTHREADM_