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
	static NumericReply noNicknameGiven(const std::string& nick);
	static NumericReply erroneusNickname(const std::string& nick, const std::string& bad_nick);
	static NumericReply nicknameInUse(const std::string& nick, const std::string& bad_nick);
	static NumericReply needMoreParams(const std::string& nick, const std::string& command);
	static NumericReply alreadyRegistered(const std::string& nick);
	static NumericReply passwordMismatch(const std::string& nick);
	static NumericReply noTopic(const std::string& nick, const std::string& channel);
	static NumericReply topic(const std::string& nick, const std::string& channel,
							  const std::string& topic);
	static NumericReply namReply(const std::string& nick, const std::string& channel,
								 const std::string& names);
	static NumericReply endOfNames(const std::string& nick, const std::string& channel);
	static NumericReply noSuchChannel(const std::string& nick, const std::string& channel);
	static NumericReply channelIsFull(const std::string& nick, const std::string& channel);
	static NumericReply inviteOnlyChan(const std::string& nick, const std::string& channel);
	static NumericReply badChannelKey(const std::string& nick, const std::string& channel);
	static NumericReply noRecipient(const std::string& nick, const std::string& command);
	static NumericReply noTextToSend(const std::string& nick);
	static NumericReply noSuchNick(const std::string& nick, const std::string& target);
	static NumericReply cannotSendToChan(const std::string& nick, const std::string& channel);
	static NumericReply notOnChannel(const std::string& nick, const std::string& channel);
};
