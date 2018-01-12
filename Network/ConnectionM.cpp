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
	Connection& conn = m_data[socket];
	sockaddr_in sa; 
	memset(&sa, 0, sizeof(sa));
	int nSockAddrLen = sizeof(sa);
	getpeername(socket, (sockaddr*)&sa, &nSockAddrLen);
	conn.m_ip = inet_ntoa(sa.sin_addr);
	conn.m_port = sa.sin_port;

	m_ipToSocket[conn.m_ip] = socket;
}

Connection* ConnectionM::GetConnection(SOCKET socket)
{
	auto it = m_data.find(socket);
	if (it == m_data.end())
		return nullptr;
	return &it->second;
}

Connection* ConnectionM::GetConnection(const std::string& ip)
{
	auto it = m_ipToSocket.find(ip);
	if (it == m_ipToSocket.end())
		return nullptr;
	return GetConnection(it->second);
}
