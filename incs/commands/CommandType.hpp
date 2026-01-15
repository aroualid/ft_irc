#pragma once

namespace irc
{
enum CommandType
{
	// client registration
	PASS,
	NICK,
	USER,

	// channel operations
	JOIN,
	PART,

	// messaging
	PRIVMSG,
	NOTICE,

	// test-only commands
	MOCK,
	MOCK2,

	// unknown/invalid
	CMD_UNKNOWN
};
}
