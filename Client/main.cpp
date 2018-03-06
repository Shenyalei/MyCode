#include <winsock2.h>

#include "Common.h"
#include "IOThreadM.h"
#include "Connection.h"
#include "ConnectionM.h"
#include "message.pb.h"

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
		Msg::Test proto;
		msg.Deserialize(&proto);
		printf("client get msg OP_PRINT : %s\n",proto.msg().c_str());
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

	Msg::Test proto;
	proto.set_msg("Hello World!");
	Message send(OP_PRINT, &proto);
	conn->SendMsg(send);

	printf("Client ProcessMsg\n");
	while (1)
	{
		Connection::ProcessMsg();
		Sleep(5*1000);
	}
	return 0;
}