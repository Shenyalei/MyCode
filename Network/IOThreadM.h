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
	virtual void OnFail() = 0;
};

class AcceptAction :public ActionBase
{
public:
	AcceptAction();
	void Init() override;
	bool OnComplete(DWORD num) override;
	void OnFail() override {};
	SOCKET acceptSocket;
	char buf[64];//for local address,remote address
};

class ConnectAction :public ActionBase
{
public:
	ConnectAction(SOCKET socket);
	~ConnectAction() override;
	bool OnComplete(DWORD num) override;
	void OnFail() override {};
	SOCKET connSocket;
};

class RecvAction :public ActionBase
{
public:
	RecvAction(Connection* _conn) :conn(_conn) {}
	bool OnComplete(DWORD num) override;
	void OnFail() override;
	Connection* conn;
};

class SendAction :public ActionBase
{
public:
	SendAction(Connection* _conn) :conn(_conn) {}
	bool OnComplete(DWORD num) override;
	void OnFail() override;
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
	bool ConnectSync(const std::string& ip, WORD port);
	void AddSocket(SOCKET socket);
	bool PostAccept(AcceptAction* action);

	SOCKET m_listenSocket;
private:
	HANDLE	m_IOCP;
	std::vector<std::thread> m_workerThreads;
};

#endif // NETWORK_IOTHREADM_