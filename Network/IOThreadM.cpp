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
	memset(buf, 0, sizeof(buf));
}

bool AcceptAction::OnComplete(DWORD num)
{
	setsockopt(acceptSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,(char*)&IOThreadM::GetInstance().m_listenSocket,sizeof(SOCKET));
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
	setsockopt(connSocket, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
	ConnectionM::GetInstance().AddConnection(connSocket);
	return true;
}

bool SendAction::OnComplete(DWORD num)
{
	conn->OnSend(num);
	return true;
}

void SendAction::OnFail()
{
	InterlockedIncrement(&conn->m_Close);
	conn->Close();
}

bool RecvAction::OnComplete(DWORD num)
{
	conn->OnRecv(num);
	return true;
}

void RecvAction::OnFail()
{
	InterlockedIncrement(&conn->m_Close);
	conn->Close();
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
			QWORD key = 0;
			while (1)
			{
				bool ret = GetQueuedCompletionStatus(m_IOCP, &numOfBytes, &key, &lpOverlapped, INFINITE);
				if (lpOverlapped)
				{
					if (ret)
						((ActionBase*)lpOverlapped)->OnComplete(numOfBytes);
					else
					{
						printf("GetQueuedCompletionStatus Fail Error:%d\n", WSAGetLastError());
						((ActionBase*)lpOverlapped)->OnFail();
					}
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

	bool reuse = true;
	setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));

	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr(ip.c_str());
	service.sin_port = htons(port);
	if (SOCKET_ERROR == bind(m_listenSocket, (sockaddr*)&service, sizeof(service)))
	{
		printf("bind error:%d\n", WSAGetLastError());
		return false;
	}

	if (SOCKET_ERROR == listen(m_listenSocket, BACKLOG_NUM))
	{
		printf("listen error:%d\n", WSAGetLastError());
		return false;
	}

	AcceptAction* action = new AcceptAction;
	PostAccept(action);
	printf("Listen IP:%s Port:%d\n", ip.c_str(), port);
	return true;
}

bool IOThreadM::Connect(const std::string& ip, WORD port)
{
	SOCKET connSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// 以下的绑定很重要，也是容易漏掉的。（如果少了绑定，在 ConnextEx 时将得到错误代码：10022 — 提供了一个无效的参数
	sockaddr_in local_addr;
	ZeroMemory(&local_addr, sizeof(sockaddr_in));
	local_addr.sin_family = AF_INET;
	if (SOCKET_ERROR == bind(connSocket, (sockaddr *)(&local_addr), sizeof(sockaddr_in)))
	{
		printf("Bind Connect Socket Error:%d\n", WSAGetLastError());
		return false;
	}

	AddSocket(connSocket);

	LPFN_CONNECTEX lpfnConnectEx = NULL;
	DWORD dwBytes = 0;
	GUID GuidConnectEx = WSAID_CONNECTEX;

	if (SOCKET_ERROR == WSAIoctl(connSocket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidConnectEx,
		sizeof(GuidConnectEx),
		&lpfnConnectEx,
		sizeof(lpfnConnectEx),
		&dwBytes, 0, 0))
	{
		printf("Get ConnectEx function pointer error:%d\n", WSAGetLastError());
		return false;
	}

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
	BOOL ret = lpfnConnectEx(connSocket,
		(sockaddr *)&addrPeer,  // [in] 对方地址
		nLen,               // [in] 对方地址长度
		lpSendBuffer,       // [in] 连接后要发送的内容，这里不用
		dwSendDataLength,   // [in] 发送内容的字节数 ，这里不用
		&dwBytesSent,       // [out] 发送了多少个字节，这里不用
		action);

	if (!ret)
	{
		if (WSAGetLastError() != ERROR_IO_PENDING) 
		{
			printf("ConnectEx error: %d/n", WSAGetLastError());
			return false;
		}
	}
	printf("Connect IP:%s Port:%d\n", ip.c_str(), port);
	return true;
}

bool IOThreadM::ConnectSync(const std::string& ip, WORD port)
{
	SOCKET connSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in addrPeer;
	ZeroMemory(&addrPeer, sizeof(sockaddr_in));
	addrPeer.sin_family = AF_INET;
	addrPeer.sin_addr.s_addr = inet_addr(ip.c_str());
	addrPeer.sin_port = htons(port);
	if (connect(connSocket, (sockaddr*)&addrPeer, sizeof(addrPeer)) == SOCKET_ERROR)
	{
		printf("ConnectSync Fail Error:%d\n", WSAGetLastError());
		closesocket(connSocket);
		return false;
	}
	IOThreadM::GetInstance().AddSocket(connSocket);
	ConnectionM::GetInstance().AddConnection(connSocket);
	printf("ConnectSync IP:%s Port:%d\n", ip.c_str(), port);
	return true;
}

void IOThreadM::AddSocket(SOCKET socket)
{
	if (!CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket), m_IOCP, 0, 0))
	{
		printf("CreateIoCompletionPort error:%d\n", WSAGetLastError());
	}
}

bool IOThreadM::PostAccept(AcceptAction* action)
{
	LPFN_ACCEPTEX lpfnAcceptEx = NULL;
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	DWORD bytes = 0;
	if (SOCKET_ERROR == WSAIoctl(m_listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx, sizeof(GuidAcceptEx),
		&lpfnAcceptEx, sizeof(lpfnAcceptEx),
		&bytes, NULL, NULL))
	{
		printf("get acceptEx function pointer error:%d\n", WSAGetLastError());
		return false;
	}

	action->acceptSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	bool ret = lpfnAcceptEx(m_listenSocket, action->acceptSocket, action->buf,0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,&bytes, action);
	if (!ret)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			printf("PostAccept fail,error:%d\n", WSAGetLastError());
			return false;
		}
	}
	return true;
}

