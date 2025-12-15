#pragma once

#include <string>

class NumericReply
{
private:
	int m_code;
	std::string m_target;
	std::string m_message;

public:
	NumericReply(int code, const std::string& target, const std::string& message);

	std::string toString() const;

	static NumericReply welcome(const std::string& nick);
	static NumericReply needMoreParams(const std::string& nick, const std::string& command);
	static NumericReply nicknameAlreadyInUse(const std::string& nick, const std::string& bad_nick);
	static NumericReply chanOPrivsNeeded(const std::string& nick, const std::string& chan);
};
