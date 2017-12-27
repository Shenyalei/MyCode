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
	}
}
