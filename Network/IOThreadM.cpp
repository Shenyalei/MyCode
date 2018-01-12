#include <WinSock2.h>
#include <mswsock.h>
#include <WS2tcpip.h>
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

ConnectAction::ConnectAction(SOCKET socket)
{
	connSocket = socket;
}

ConnectAction::~ConnectAction()
{
	delete this;
}

bool ConnectAction::OnComplete(DWORD num)
{
	ConnectionM::GetInstance().AddConnection(connSocket);
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
	for (auto&& thread : m_workerThreads)
	{
		thread.join();
	}
}

bool IOThreadM::Start()
{
	WSADATA wsaData; WSAStartup(MAKEWORD(2, 2), &wsaData);

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
	inet_pton(AF_INET, ip.c_str(), (void*)&service.sin_addr);
	//service.sin_addr.s_addr = inet_addr(ip.c_str());
	service.sin_port = htons(port);
	bind(m_listenSocket, (sockaddr*)&service, sizeof(service));

	listen(m_listenSocket, BACKLOG_NUM);

	AcceptAction* action = new AcceptAction;

	PostAccept(action);
	return true;
}

bool IOThreadM::Connect(const std::string& ip, WORD port)
{
	SOCKET connSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// 以下的绑定很重要，也是容易漏掉的。（如果少了绑定，在 ConnextEx 时将得到错误代码：10022 — 提供了一个无效的参数
	sockaddr_in local_addr;
	ZeroMemory(&local_addr, sizeof(sockaddr_in));
	local_addr.sin_family = AF_INET;
	bind(connSocket, (sockaddr *)(&local_addr), sizeof(sockaddr_in));

	AddSocket(connSocket);

	LPFN_CONNECTEX lpfnConnectEx = NULL;
	DWORD dwBytes = 0;
	GUID GuidConnectEx = WSAID_CONNECTEX;

	WSAIoctl(connSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,&GuidConnectEx, sizeof(GuidConnectEx),&lpfnConnectEx, sizeof(lpfnConnectEx), &dwBytes, 0, 0);

	sockaddr_in addrPeer;
	ZeroMemory(&addrPeer, sizeof(sockaddr_in));
	addrPeer.sin_family = AF_INET;
	addrPeer.sin_addr.s_addr = inet_addr(ip.c_str());
	addrPeer.sin_port = htons(port);

	int nLen = sizeof(addrPeer);
	PVOID lpSendBuffer = NULL;
	DWORD dwSendDataLength = 0;
	DWORD dwBytesSent = 0;

	ConnectAction* action = new ConnectAction(connSocket);
	BOOL bResult = lpfnConnectEx(connSocket,
		(sockaddr *)&addrPeer,  // [in] 对方地址
		nLen,               // [in] 对方地址长度
		lpSendBuffer,       // [in] 连接后要发送的内容，这里不用
		dwSendDataLength,   // [in] 发送内容的字节数 ，这里不用
		&dwBytesSent,       // [out] 发送了多少个字节，这里不用
		action); // [in] 这东西复杂，下一篇有详解

	if (!bResult)      // 返回值处理
	{
/*
		if (WSAGetLastError() != ERROR_IO_PENDING)   // 调用失败
		{
			TRACE(TEXT("ConnextEx error: %d/n"), WSAGetLastError());
			return FALSE;
		}
		else;// 操作未决（正在进行中 … ）
		{
			TRACE0("WSAGetLastError() == ERROR_IO_PENDING/n");// 操作正在进行中
		}*/
	}

	return TRUE;
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

