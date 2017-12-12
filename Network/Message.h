#ifndef NETWORK_MESSAGE__
#define NETWORK_MESSAGE__

#include "Common.h"
#include <google/protobuf/message.h>


class Message {
public:

	static const int MAX_LENGTH = 1024 * 2;
	static const int HEADER_LEN = 4;
	static const int MAX_BODY_LEN = MAX_LENGTH - HEADER_LEN;

	Message(int opcode = 0, ::google::protobuf::Message* msg = nullptr) {};

	~Message();


	char* Body() { return data_ + HEADER_LEN; }
	char* Data() { return data_; }

	uint16_t opcode() { return opcode_; }
	uint16_t body_length() { return body_length_; }
	uint16_t length() { return header_length + body_length_; }

	bool parse(::google::protobuf::Message* msg);
	void serialize(::google::protobuf::Message* msg);

	bool decode_header();

	void encode_header();

	void set_opcode(uint16_t opcode) { opcode_ = opcode; }
	void set_data(const char* data, uint16_t body_length);

private:
	WORD opcode_;
	uint16_t body_length_;
	char m_data[MAX_LENGTH];
};

#endif 
