#include <WinSock2.h>
#include <mswsock.h>
#include "Common.h"
#include "Connection.h"
#include "IOThreadM.h"
#include "ConnectionM.h"


#define RECV_MSG_QUEUE_SIZE 255
#define SEND_MSG_QUEUE_SIZE 1024

std::mutex Connection::m_recvQueueMutex;
RingBuffer<Connection::MsgEvent> Connection::m_recvQueue(SEND_MSG_QUEUE_SIZE);

Connection::Connection(SOCKET _socket) :m_sendQueue(SEND_MSG_QUEUE_SIZE)
{
	m_port = 0;
	m_socket = _socket;
	m_sendMsg = nullptr;
	m_sendPos = 0;
	m_recvMsg = new Message();
	m_recvPos = 0;
	m_Close = 0;
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
	if (!m_sendQueue.push(msg))
	{
		printf("send queue full\n");
		return;
	}
	if (m_sendQueue.Size() == 1)
	{
		m_sendPos = 0;
		m_sendMsg = &m_sendQueue.back();
		PostSend();
	}
}

void Connection::RecvMsg(Message* msg)
{
	std::lock_guard<std::mutex> guard(m_recvQueueMutex);
	MsgEvent event{this, msg};
	if (!m_recvQueue.push(event))
	{
		printf("m_recvQueue full\n");
	}
}

void Connection::ProcessMsg()
{
	std::lock_guard<std::mutex> guard(m_recvQueueMutex);
	while (!m_recvQueue.Empty())
	{
		MsgEvent event = m_recvQueue.back();
		m_recvQueue.pop();
		// do msg
		MSG_HANDLE handle = GetMsgHandle(event.msg->Opcode());
		if (handle)
		{
			handle(*event.conn, *event.msg);
			delete event.msg;
		}
	}
}

void Connection::PostSend()
{
	DWORD flags = 0;
	WSABUF wsabuf;
	wsabuf.buf = m_sendMsg->Data() + m_sendPos;
	wsabuf.len = m_sendMsg->Length() - m_sendPos;
	if (SOCKET_ERROR == WSASend(m_socket, &wsabuf, 1, NULL, flags, m_sendAction, NULL))
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			printf("Socket:%llu WSASend Error:%d\n", m_socket,WSAGetLastError());
			m_sendAction->OnFail();
		}
	}
}

void Connection::PostRecv()
{
	DWORD flags = 0;
	DWORD bytesRecvd = 0;
	WSABUF	 wsabuf;	
	wsabuf.buf = m_recvMsg->Data()+m_recvPos;
	wsabuf.len = m_recvPos >= HEADER_LEN? m_recvMsg->Length() - m_recvPos:HEADER_LEN - m_recvPos;
	if (SOCKET_ERROR == WSARecv(m_socket, &wsabuf, 1, &bytesRecvd, &flags, m_recvAction, nullptr))
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			printf("Socket:%llu WSARecv Error:%d\n", m_socket,WSAGetLastError());
			m_recvAction->OnFail();
		}
	}
}

void Connection::OnSend(int num)
{
	m_sendPos += num;
	printf("send %d bytes data\n", num);
	if (m_sendPos >= m_sendMsg->Length())
	{
		m_sendQueue.pop();
		m_sendMsg = nullptr;
		m_sendPos = 0;
		if (m_sendQueue.Empty())
			return;
		m_sendMsg = &m_sendQueue.back();
	}
	PostSend();
}

void Connection::OnRecv(int num)
{
	m_recvPos += num;
	printf("revc %d bytes data\n", num);
	if (m_recvPos >= HEADER_LEN && m_recvPos == m_recvMsg->Length())
	{
		RecvMsg(m_recvMsg);
		m_recvMsg = new Message();
		m_recvPos = 0;
	}
	PostRecv();
}

void Connection::Close()
{
	//发送事件和接受事件都关闭的情况下才能删除连接
	if (InterlockedIncrement(&m_Close) == 2)
	{
		closesocket(m_socket);
		ConnectionM::GetInstance().RemoveConnection(m_socket);
	}
}

