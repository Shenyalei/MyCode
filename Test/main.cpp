#include <winsock2.h>

#include "Common.h"
#include "IOThreadM.h"
#include "Connection.h"
#include "ConnectionM.h"

#define LOCAL_IP "127.0.0.1"
#define LISTEN_PORT 10001
enum OpCode
{
	OP_PRINT,
};

void InitMsg()
{
	DefineMsgHandle(OP_PRINT, [](Connection& conn, Message& msg) {
		printf("okkk\n");
		return true; 
	});
}

int main(int argc, char** argv)
{
	InitMsg();
	IOThreadM::GetInstance().Start();
	IOThreadM::GetInstance().Listen(LOCAL_IP, LISTEN_PORT);
	//start client
	IOThreadM::GetInstance().Connect(LOCAL_IP,LISTEN_PORT);

	Sleep(1000);
	Connection* conn = ConnectionM::GetInstance().GetConnection(LOCAL_IP);
	if (!conn)
		return 0;
	//conn->SendMsg(Message(OP_PRINT));
	while (1)
	{
		Sleep(1000);
	}
	return 0;
}