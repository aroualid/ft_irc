#pragma once

#include "Message.hpp"

class MessageParser
{
public:
	static Message parse(const std::string& raw);
	static std::string serialize(const Message& message);
	static bool isValid(const std::string& raw);
};
