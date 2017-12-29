#ifndef NETWORK_MESSAGE__
#define NETWORK_MESSAGE__

#include <google/protobuf/message.h>

#define MAX_LENGTH 65535
#define HEADER_LEN 4
#define MAX_BODY_LEN  (MAX_LENGTH - HEADER_LEN)

class Connection;
class Message {
public:

	Message(WORD opcode = 0, ::google::protobuf::Message* msg = nullptr);
	Message(const Message& other);
	~Message() {};

	Message& operator=(const Message& other);
	char* Body() { return m_data + HEADER_LEN; }

	char* Data() { return m_data; }
	const char* Data() const { return m_data; }
	void SetData(const char* data, WORD len);

	WORD Opcode() const { return m_opcode; }
	void SetOpcode(WORD opcode) { m_opcode = opcode; }

	WORD BodyLen() const { return m_bodyLen; }
	WORD Length() const { return HEADER_LEN + m_bodyLen; }

	bool Serialize(::google::protobuf::Message& msg);
	bool Deserialize(::google::protobuf::Message& msg);

	void DecodeHeader();
	void EncodeHeader();

private:
	WORD m_opcode;
	WORD m_bodyLen;
	char m_data[MAX_LENGTH];
};
typedef std::function<bool(Connection&, Message&)> MSG_HANDLE;
extern void DefineMsgHandle(WORD opcode,const MSG_HANDLE& handle);
extern MSG_HANDLE& GetMsgHandle(WORD opcode);

#endif 
