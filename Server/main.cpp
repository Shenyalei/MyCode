#include <winsock2.h>

#include "Common.h"
#include "IOThreadM.h"
#include "Connection.h"
#include "ConnectionM.h"

#define LOCAL_IP "0.0.0.0"
#define LISTEN_PORT 10001

enum OpCode
{
	OP_NONE = 0,
	OP_PRINT = 1,
};

void InitMsg()
{
	DefineMsgHandle(OP_PRINT, [](Connection& conn, Message& msg) {
		printf("server get msg OP_PRINT\n");
		return true; 
	});
}

int main(int argc, char** argv)
{
	printf("Start Server:\n");
	InitMsg();
	IOThreadM::GetInstance().Start();
	IOThreadM::GetInstance().Listen(LOCAL_IP, LISTEN_PORT);

	printf("Server ProcessMsg\n");
	while (1)
	{
		Connection::ProcessMsg();
		Sleep(5*1000);
	}
	return 0;
}