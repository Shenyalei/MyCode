#ifndef NETWORK_MESSAGE__
#define NETWORK_MESSAGE__

#include <google/protobuf/message.h>

class Message {
public:

	static const int MAX_LENGTH = 65535;
	static const int HEADER_LEN = 4;
	static const int MAX_BODY_LEN = MAX_LENGTH - HEADER_LEN;

	Message(WORD opcode = 0, ::google::protobuf::Message* msg = nullptr);
	~Message() {};


	char* Body() { return m_data + HEADER_LEN; }

	char* Data() { return m_data; }
	void SetData(const char* data, WORD len);

	WORD Opcode() { return m_opcode; }
	void SetOpcode(WORD opcode) { m_opcode = opcode; }

	WORD BodyLen() { return m_bodyLen; }
	WORD Length() { return HEADER_LEN + m_bodyLen; }

	bool Serialize(::google::protobuf::Message& msg);
	bool Deserialize(::google::protobuf::Message& msg);

	void DecodeHeader();
	void EncodeHeader();

private:
	WORD m_opcode;
	WORD m_bodyLen;
	char m_data[MAX_LENGTH];
};

#endif 
