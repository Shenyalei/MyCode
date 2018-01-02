#include "Common.h"
#include "Message.h"

#define MAX_MSG_HANDLE 65535

static MSG_HANDLE g_msgHandleTable[MAX_MSG_HANDLE] = { nullptr };
void DefineMsgHandle(WORD opcode,const MSG_HANDLE& handle)
{
	if (opcode >= MAX_MSG_HANDLE)
	{
		printf("opcode beyond max\n");
		return;
	}
	g_msgHandleTable[opcode] = handle;
}

MSG_HANDLE& GetMsgHandle(WORD opcode)
{
	return g_msgHandleTable[opcode];
}

Message::Message(WORD opcode /*= 0*/, ::google::protobuf::Message* msg /*= nullptr*/)
{
	m_bodyLen = 0;
	m_opcode = opcode;
	if (msg)
		Serialize(*msg);
}

Message::Message(const Message & other)
{
	if (this == &other)
		return;
	m_opcode = other.Opcode();
	m_bodyLen = other.BodyLen();
	memcpy(m_data,other.Data(),other.Length());
}

Message& Message::operator=(const Message& other)
{
	if (this != &other)
	{
		m_opcode = other.Opcode();
		m_bodyLen = other.BodyLen();
		memcpy(m_data, other.Data(), other.Length());
	}
	return *this;
}

void Message::SetData(const char * data, WORD len)
{
	m_bodyLen = len;
	memcpy(m_data, data, len);
}

bool Message::Serialize(::google::protobuf::Message& msg)
{
	if (msg.ByteSize() > MAX_BODY_LEN)
		return false;
	m_bodyLen = msg.ByteSize();
	EncodeHeader();
	msg.SerializeToArray(Body(), BodyLen());
	return true;
}

bool Message::Deserialize(::google::protobuf::Message& msg)
{
	return msg.ParseFromArray(Body(), BodyLen());
}

void Message::DecodeHeader()
{
	UINT header = *(UINT*)m_data;
	m_bodyLen = header >> 16;
	m_opcode = (WORD)header;
}

void Message::EncodeHeader()
{
	UINT header = m_bodyLen << 16 | m_opcode;
	memcpy(m_data, &header, HEADER_LEN);
}
