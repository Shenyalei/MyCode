#include <WinSock2.h>
#include <mswsock.h>
#include "Common.h"
#include "IOThreadM.h"
#include "Connection.h"
#include "ConnectionM.h"

#define IO_THREAD_NUM 4 //io线程数量
#define BACKLOG_NUM 100 // 服务器没有accept的连接的队列长度
#define SEND_BUFFER_SIZE 1024//发送缓冲区长度
#define RECV_BUBFER_SIZE 1024//接受缓冲区长度

ActionBase::ActionBase() : OVERLAPPED()
{
	Init();
}

void ActionBase::Init()
{
	Internal = 0;
	InternalHigh = 0;
	Offset = 0;
	OffsetHigh = 0;
	hEvent = 0;
}

AcceptAction::AcceptAction()
{
}

void AcceptAction::Init()
{
	ActionBase::Init();
	acceptSocket = 0;
}

bool AcceptAction::OnComplete(DWORD num)
{
	IOThreadM::GetInstance().AddSocket(acceptSocket);
	ConnectionM::GetInstance().AddConnection(acceptSocket);
	Init();
	IOThreadM::GetInstance().PostAccept(this);
	return true;
}

bool SendAction::OnComplete(DWORD num)
{
	conn->OnSend(num);
	return true;
}

bool RecvAction::OnComplete(DWORD num)
{
	conn->OnRecv(num);
	return true;
}

IOThreadM::IOThreadM()
{
	m_IOCP = nullptr;
	m_listenSocket = 0;
}

IOThreadM::~IOThreadM()
{
}

bool IOThreadM::Start()
{
	m_IOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	for (int i = 0 ; i < IO_THREAD_NUM;++i)
	{
		m_workerThreads.push_back(std::thread([this]() {
			DWORD numOfBytes = 0;
			LPOVERLAPPED lpOverlapped;
			while (1)
			{
				bool ret = GetQueuedCompletionStatus(m_IOCP, &numOfBytes, nullptr, &lpOverlapped, INFINITE);
				if (lpOverlapped && ret)
				{
					((ActionBase*)lpOverlapped)->OnComplete(numOfBytes);
				}
			}
		}));
	}
	return true;
}

bool IOThreadM::Listen(const std::string& ip,WORD port)
{
	m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	AddSocket(m_listenSocket);

	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr(ip.c_str());
	service.sin_port = port;
	bind(m_listenSocket, (sockaddr*)&service, sizeof(service));

	listen(m_listenSocket, BACKLOG_NUM);

	AcceptAction* action = new AcceptAction;

	PostAccept(action);
	return true;
}

void IOThreadM::AddSocket(SOCKET socket)
{
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket), m_IOCP,0, 0);
}

void IOThreadM::PostAccept(AcceptAction* action)
{
	LPFN_ACCEPTEX lpfnAcceptEx = NULL;
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	DWORD bytes = 0;
	WSAIoctl(m_listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx, sizeof(GuidAcceptEx),
		&lpfnAcceptEx, sizeof(lpfnAcceptEx),
		&bytes, NULL, NULL);

	action->acceptSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	lpfnAcceptEx(m_listenSocket, action->acceptSocket, nullptr,0,0,0,&bytes, action);
}



