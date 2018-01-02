#include <winsock2.h>

#include "Common.h"
#include "IOThreadM.h"
#include "Connection.h"


#define LOCAL_IP "127.0.0.1"
#define LISTEN_PORT 10001
enum OPCODE
{
	OPCODE_PRINT,
};

void InitMsg()
{
	DefineMsgHandle(OPCODE_PRINT, [](Connection& conn, Message& msg) {
		printf("okkk\n");
		return true;
	});
}

int main(int argc, char** argv)
{
	InitMsg();
	IOThreadM::GetInstance().Start();
//	IOThreadM::GetInstance().Listen(LOCAL_IP, htons(LISTEN_PORT));
	//start client

	return 0;
}