#include <winsock2.h>

#include "Common.h"
#include "IOThreadM.h"
#include "Connection.h"
#include "ConnectionM.h"

#define REMOTE_PORT 10001
#define REMOTE_IP "192.168.99.32"

enum OpCode
{
	OP_NONE = 0,
	OP_PRINT = 1,
};

void InitMsg()
{
	DefineMsgHandle(OP_PRINT, [](Connection& conn, Message& msg) {
		printf("client get msg OP_PRINT\n");
		return true;
	});
}

int main(int argc, char** argv)
{
	printf("Start Client:\n");
	IOThreadM::GetInstance().Start();
	InitMsg();

	IOThreadM::GetInstance().Connect(REMOTE_IP, REMOTE_PORT);
	Sleep(5 * 1000);

	Connection* conn = ConnectionM::GetInstance().GetConnection(REMOTE_IP);
	conn->SendMsg(Message(OP_PRINT));
	printf("Client ProcessMsg\n");
	while (1)
	{
		Connection::ProcessMsg();
		Sleep(5*1000);
	}
	return 0;
}