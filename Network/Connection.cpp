#include <WinSock2.h>
#include <mswsock.h>
#include "Common.h"
#include "Connection.h"
#include "IOThreadM.h"


#define RECV_MSG_QUEUE_SIZE 255
#define SEND_MSG_QUEUE_SIZE 255

Connection::Connection(SOCKET _socket) :m_recvQueue(RECV_MSG_QUEUE_SIZE),m_sendQueue(SEND_MSG_QUEUE_SIZE)
{
	m_socket = _socket;
	m_sendMsg = nullptr;
	m_sendPos = 0;
	m_recvMsg = nullptr;
	m_recvPos = 0;
	m_sendAction = new SendAction(this);
	m_recvAction = new RecvAction(this);
	PostRecv();
}


Connection::~Connection()
{
	delete m_sendAction;
	delete m_recvAction;
}

void Connection::SendMsg(const Message& msg)
{
	Message* pMsg = new Message(msg);
	if (!m_sendQueue.push(pMsg))
	{
		printf("send queue full\n");
		return;
	}
	if (m_sendQueue.Size() == 1)
	{
		PostSend();
	}
}

void Connection::RecvMsg(Message* msg)
{
	if (!m_recvQueue.push(msg))
	{
		printf("m_recvQueue full\n");
	}
}

void Connection::ProcessMsg()
{
	Message* msg = m_recvQueue.pop();
	while (msg)
	{
		// do msg
		delete msg;
		msg = m_recvQueue.pop();
	}
}

void Connection::PostSend()
{
	if (m_sendMsg == nullptr)//从队列里取一个消息
	{
		m_sendMsg = m_sendQueue.pop();
		if (m_sendMsg == nullptr)
			return;
	}
	WSABUF wsabuf;
	wsabuf.buf = m_sendMsg->Data() + m_sendPos;
	wsabuf.len = m_sendMsg->Length() - m_sendPos;
	WSASend(m_socket, &wsabuf, 1, NULL, NULL, m_sendAction, NULL);
}

void Connection::PostRecv()
{
	if (m_recvMsg == nullptr)
	{
		m_recvMsg = new Message();
		m_recvPos = 0;
	}
	WSABUF	 wsabuf;	
	wsabuf.buf = m_recvMsg->Data()+m_recvPos;
	wsabuf.len = m_recvMsg->BodyLen()? m_recvMsg->Length() - m_recvPos:HEADER_LEN - m_recvPos;
	WSARecv(m_socket, &wsabuf, 1, nullptr,nullptr, m_recvAction, NULL);
}

void Connection::OnSend(int num)
{
	m_sendPos += num;
	if (m_sendPos >= m_sendMsg->Length())
	{
		m_sendMsg = nullptr;
		m_sendPos = 0;
	}
	PostSend();
}

void Connection::OnRecv(int num)
{
	m_recvPos += num;
	if (m_recvMsg->BodyLen() == 0 && m_recvPos >= HEADER_LEN)
	{
		m_recvMsg->DecodeHeader();
	}
	else if (m_recvMsg->BodyLen() && m_recvPos >= m_recvMsg->Length())
	{
		RecvMsg(m_recvMsg);
		m_recvMsg = nullptr;
		m_recvPos = 0;
	}
	PostRecv();
}

