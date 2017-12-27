#ifndef NETWORK_SESSIONM_
#define NETWORK_SESSIONM_

#include "Connection.h"
class ConnectionM : public Singleton<ConnectionM>
{
public:
	ConnectionM();
	~ConnectionM();
	void AddConnection(SOCKET socket);
private:
	std::unordered_map<SOCKET, Connection> m_data;
};

#endif