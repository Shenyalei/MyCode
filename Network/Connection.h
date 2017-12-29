#ifndef NETWORK_CONNECTION_
#define NETWORK_CONNECTION_

#include "Message.h"
#include "RingBuffer.h"

class SendAction;
class RecvAction;

class Connection
{
public:
	Connection(SOCKET _socket);
	virtual ~Connection();
	void SendMsg(const Message& msg);
	void RecvMsg(Message* msg);
	void ProcessMsg();

	void PostSend();
	void PostRecv();

	void OnSend(int num);
	void OnRecv(int num);
private:
	RingBuffer<Message> m_recvQueue;
	RingBuffer<Message> m_sendQueue;
	SendAction* m_sendAction;
	RecvAction* m_recvAction;
	WORD m_sendPos;
	Message* m_sendMsg;
	WORD m_recvPos;
	Message* m_recvMsg;

	SOCKET m_socket;
	std::string m_ip;
	WORD m_port;
};

#endif // NETWORK_CONNECTION_