#pragma once

namespace irc
{
enum CommandType
{
	// client registration
	PASS,
	NICK,
	USER,
	QUIT,

	// channel operations
	JOIN,
	PART,

	// messaging
	PRIVMSG,
	NOTICE,

	// test-only commands
	MOCK,
	MOCK2,

	// ping / pong
	PING,
	PONG,

	// operator stuff
	KICK,
	INVITE,
	TOPIC,
	MODE,

	// channel info
	WHO,
	NAMES,

	// unknown/invalid
	CMD_UNKNOWN
};
}
