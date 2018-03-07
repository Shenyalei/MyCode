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
	OP_CLIENT_SERVER = 1,
	OP_SERVER_CLIENT = 2,
};

void InitMsg()
{
	DefineMsgHandle(OP_SERVER_CLIENT, [](Connection& conn, Message& msg) {
		Msg::Test proto;
		msg.Deserialize(&proto);
		printf("client get msg OP_SERVER_CLIENT : %s\n",proto.msg().c_str());
		proto.set_msg("Hello World!");
		Message send(OP_CLIENT_SERVER, &proto);
		conn.SendMsg(send);
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
	Message send(OP_CLIENT_SERVER, &proto);
	conn->SendMsg(send);

	printf("Client ProcessMsg\n");
	while (1)
	{
		Connection::ProcessMsg();
		Sleep(5*1000);
	}
	return 0;
}