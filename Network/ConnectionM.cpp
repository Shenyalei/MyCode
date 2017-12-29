#include "Common.h"
#include "ConnectionM.h"



ConnectionM::ConnectionM()
{
}


ConnectionM::~ConnectionM()
{
}

void ConnectionM::AddConnection(SOCKET socket)
{
	if (!m_data.emplace(socket, socket).second)
	{
		printf("add connection duplicate\n");
		return;
	}
/*
	sockaddr_in sa; 
	memset(&sa, 0, sizeof(sa));
	int nSockAddrLen = sizeof(sa);
	getpeername(socket, (sockaddr*)&sa, &nSockAddrLen);
	inet_ntoa(sa.sin_addr), 64);*/
}
