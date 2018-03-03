#ifndef NETWORK_MESSAGE__
#define NETWORK_MESSAGE__

#include <google/protobuf/message.h>

#define MAX_LENGTH 65535
#define HEADER_LEN 4
#define MAX_BODY_LEN  (MAX_LENGTH - HEADER_LEN)

/*
	消息格式
	包长 + 消息号 + 数据
	2B	+	2B	+ MAX_BODY_LEN
*/
class Connection;
class Message 
{
public:

	Message(WORD opcode = 0, ::google::protobuf::Message* msg = nullptr);
	Message(const Message& other);
	~Message() {};

	Message& operator=(const Message& other);
	char* Body() { return m_data + HEADER_LEN; }

	char* Data() { return m_data; }
	const char* Data() const { return m_data; }
	void SetData(const char* data, WORD len);

	WORD Opcode() const { return *(WORD*)(m_data+2); }
	void SetOpcode(WORD opcode) { *(WORD*)(m_data + 2) = opcode; }

	WORD BodyLen() const { return *(WORD*)m_data; }
	void SetBodyLen(WORD len) { *(WORD*)m_data = len; }

	WORD Length() const { return HEADER_LEN + BodyLen(); }

	bool Serialize(::google::protobuf::Message& msg);
	bool Deserialize(::google::protobuf::Message& msg);

private:
	char m_data[MAX_LENGTH];
};
typedef std::function<bool(Connection&, Message&)> MSG_HANDLE;
extern void DefineMsgHandle(WORD opcode,const MSG_HANDLE& handle);
extern MSG_HANDLE& GetMsgHandle(WORD opcode);

#endif 
