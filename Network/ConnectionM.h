#ifndef NETWORK_SESSIONM_
#define NETWORK_SESSIONM_

#include "Connection.h"
class ConnectionM : public Singleton<ConnectionM>
{
public:
	ConnectionM();
	~ConnectionM();
	bool AddConnection(SOCKET socket);
	bool RemoveConnection(SOCKET socket);
	Connection* GetConnection(SOCKET socket);
	Connection* GetConnection(const std::string& ip);
private:
	std::map<SOCKET, Connection*> m_data;
	std::unordered_map<std::string, SOCKET> m_ipToSocket;
};

#endif