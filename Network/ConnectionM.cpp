#include <Winsock2.h>
#include <Mstcpip.h>

#include "Common.h"
#include "ConnectionM.h"

#define KEEPALIVE_INTERVAL 1000
#define KEEPALIVE_TIME 18000

ConnectionM::ConnectionM()
{
}


ConnectionM::~ConnectionM()
{
}

bool ConnectionM::AddConnection(SOCKET socket)
{
	static std::mutex add_conn_mutex;
	std::lock_guard<std::mutex> guard(add_conn_mutex);

	if (m_data.find(socket) != m_data.end())
	{
		printf("duplicate socket %d\n",socket);
		return false;
	}
	Connection* pconn = new Connection(socket);
	 m_data[socket]= pconn;

	//get peer ip and port
	Connection& conn = *m_data[socket];
	sockaddr_in sa; 
	memset(&sa, 0, sizeof(sa));
	int len = sizeof(sa);
	if (SOCKET_ERROR == getpeername(socket, (sockaddr*)&sa, &len))
	{
		printf("getpeername Error:%d\n", WSAGetLastError());
	}
    conn.m_ip = inet_ntoa(sa.sin_addr);
	conn.m_port = ntohs(sa.sin_port);

   	m_ipToSocket[conn.m_ip] = socket;

	//keep alive
	tcp_keepalive tcpin;
	tcpin.onoff = 1;		
	tcpin.keepaliveinterval = KEEPALIVE_INTERVAL;
	tcpin.keepalivetime = KEEPALIVE_TIME;
	DWORD dwSize = 0;
	if (SOCKET_ERROR == WSAIoctl(socket, SIO_KEEPALIVE_VALS, &tcpin, sizeof(tcpin), NULL, 0, &dwSize, NULL, NULL))
	{
		printf("WSAIoclt KeepAlive Error:%d\n", WSAGetLastError());
	}

	printf("Add Connection success! Peer IP:%s,Port:%d ConnectionNum:%d\n", conn.m_ip.c_str(), conn.m_port,m_data.size());
	return true;
}

bool ConnectionM::RemoveConnection(SOCKET socket)
{
	static std::mutex remove_conn_mutex;
	std::lock_guard<std::mutex> guard(remove_conn_mutex);
	Connection* conn = GetConnection(socket);
	if (!conn)
		return false;
	m_ipToSocket.erase(conn->m_ip);
	m_data.erase(socket);
	printf("Socket:%llu IP:%s Port:%d Connection Closed ConnectionNum:%d\n", socket, conn->m_ip.c_str(), conn->m_port, m_data.size());
	delete conn;
	return true;
}

Connection* ConnectionM::GetConnection(SOCKET socket)
{
	auto it = m_data.find(socket);
	if (it == m_data.end())
		return nullptr;
	return it->second;
}

Connection* ConnectionM::GetConnection(const std::string& ip)
{
	auto it = m_ipToSocket.find(ip);
	if (it == m_ipToSocket.end())
		return nullptr;
	return GetConnection(it->second);
}
