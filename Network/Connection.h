#ifndef NETWORK_CONNECTION_
#define NETWORK_CONNECTION_

#include "Message.h"

class Connection
{
public:
	Connection();
	~Connection();
	void SendMsg(const Message& msg);
	void RecvMsg(const Message& msg);
};

#endif // NETWORK_CONNECTION_