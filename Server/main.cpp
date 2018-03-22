#include <winsock2.h>

#include "Common.h"
#include "IOThreadM.h"
#include "Connection.h"
#include "ConnectionM.h"
#include "message.pb.h"
#include "cpp_redis/cpp_redis"

#define LOCAL_IP "0.0.0.0"
#define LISTEN_PORT 10001

enum OpCode
{
	OP_NONE = 0,
	OP_CLIENT_SERVER,
	OP_SERVER_CLIENT,
};

void InitMsg()
{
	DefineMsgHandle(OP_CLIENT_SERVER, [](Connection& conn, Message& msg) {
		Msg::Test proto;
		msg.Deserialize(&proto);
		printf("client(%s:%d) get msg OP_CLIENT_SERVER : %s\n",conn.m_ip.c_str(),conn.m_port, proto.msg().c_str());
		proto.set_msg("server response");
		Message send(OP_SERVER_CLIENT, &proto);
		conn.SendMsg(send);
		return true;
	});
}

int main(int argc, char** argv)
{
	printf("Start Server:\n");

	//network io
	InitMsg();
	IOThreadM::GetInstance().Start();
	IOThreadM::GetInstance().Listen(LOCAL_IP, LISTEN_PORT);

	//redis client
	cpp_redis::client client;
	client.connect();
	client.set("hello", "42");
	client.get("hello", [](cpp_redis::reply& reply) {
		std::cout << "redis reply:" << reply << std::endl;
	});
	client.sync_commit();

	//logic
	printf("Server ProcessMsg\n");
	while (1)
	{
		Connection::ProcessMsg();
		Sleep(5*1000);
	}
	return 0;
}