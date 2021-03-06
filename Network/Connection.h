#ifndef NETWORK_CONNECTION_
#define NETWORK_CONNECTION_

#include "Message.h"
#include "RingBuffer.h"

class SendAction;
class RecvAction;

class Connection : public std::enable_shared_from_this<Connection>
{
public:
	struct MsgEvent
	{
		std::shared_ptr<Connection> conn;
		Message* msg = nullptr;
	};
	Connection(const Connection&) = delete;
	Connection& operator=(const Connection&) = delete;
	Connection(Connection&&) = delete;
	Connection& operator=(Connection&&) = delete;

	Connection(SOCKET _socket);
	virtual ~Connection();

	void SendMsg(const Message& msg);
	void RecvMsg(Message* msg);
	static void ProcessMsg();

	void PostSend();
	void PostRecv();

	void OnSend(int num);
	void OnRecv(int num);

	void Close();

	std::string m_ip;
	WORD m_port;
	std::atomic<bool> m_close;
	std::atomic<bool> m_closeSend;
	std::atomic<bool> m_closeRecv;
private:
	static std::mutex m_recvQueueMutex;
	static RingBuffer<MsgEvent> m_recvQueue;
	RingBuffer<Message> m_sendQueue;
	SendAction* m_sendAction;
	RecvAction* m_recvAction;
	WORD m_sendPos;
	Message* m_sendMsg;
	WORD m_recvPos;
	Message* m_recvMsg;

	SOCKET m_socket;
};

#endif // NETWORK_CONNECTION_