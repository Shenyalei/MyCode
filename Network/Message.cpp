#include "Common.h"
#include "Message.h"

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
