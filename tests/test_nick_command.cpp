#include "commands/ACommand.hpp"
#include "commands/CommandFactory.hpp"
#include "commands/CommandType.hpp"
#include "mocks/Channel.hpp"
#include "mocks/Client.hpp"
#include "mocks/Server.hpp"
#include "protocol/Message.hpp"
#include <criterion/criterion.h>

// Test fixture - fresh instance created for each test
static Server* g_server;
static ClientMock* g_alice;
static ClientMock* g_bob;
static ACommand* g_cmd;

static void setup(void)
{
	g_server = new Server(6667, "testpass");
	g_alice = new ClientMock(3, "localhost", *g_server);
	g_bob = new ClientMock(4, "localhost", *g_server);
	g_cmd = CommandFactory::getInstance().createCommand(irc::NICK, *g_server);
}

static void teardown(void)
{
	delete g_cmd;
	delete g_bob;
	delete g_alice;
	delete g_server;
}

// Helper to build a NICK message
static Message nickMsg(const char* nickname)
{
	Message msg;
	msg.m_command = "NICK";
	if (nickname)
		msg.m_params.push_back(nickname);
	return msg;
}

// Helper to register a client
static void registerClient(ClientMock* client, const char* nick)
{
	client->setPasswordProvided(true);
	client->setNickname(nick);
	client->setUsername(nick);
	g_server->registerClient(nick, client);
}

TestSuite(NickCommand, .init = setup, .fini = teardown);

// ============================================================================
// RFC 4.1.2: Basic functionality
// ============================================================================

Test(NickCommand, factory_creates_nick_command)
{
	cr_assert_not_null(g_cmd, "Factory failed to create NICK command. Is it registered?");
	cr_assert_str_eq(g_cmd->getName(), "NICK");
}

Test(NickCommand, sets_nickname_correctly)
{
	g_alice->setPasswordProvided(true);

	g_cmd->execute(g_alice, nickMsg("alice"));

	cr_assert_str_eq(g_alice->getNickname().c_str(), "alice");
}

Test(NickCommand, nickname_change_allowed)
{
	registerClient(g_alice, "alice");

	g_cmd->execute(g_alice, nickMsg("bob"));

	cr_assert_str_eq(g_alice->getNickname().c_str(), "bob");
}

// ============================================================================
// RFC 4.1.2: Nickname format validation (RFC 1459 section 2.3.1)
// <nick> ::= <letter> { <letter> | <number> | <special> }
// <special> ::= '-' | '[' | ']' | '\' | '`' | '^' | '{' | '}'
// Max 9 characters
// ============================================================================

Test(NickCommand, accepts_valid_alphanumeric)
{
	g_alice->setPasswordProvided(true);

	g_cmd->execute(g_alice, nickMsg("Alice123"));

	cr_assert_str_eq(g_alice->getNickname().c_str(), "Alice123");
}

Test(NickCommand, accepts_max_nine_chars)
{
	g_alice->setPasswordProvided(true);

	g_cmd->execute(g_alice, nickMsg("abcdefghi")); // exactly 9

	cr_assert_str_eq(g_alice->getNickname().c_str(), "abcdefghi");
}

Test(NickCommand, accepts_special_char_bracket)
{
	g_alice->setPasswordProvided(true);

	g_cmd->execute(g_alice, nickMsg("alice[0]"));

	cr_assert_str_eq(g_alice->getNickname().c_str(), "alice[0]");
}

Test(NickCommand, accepts_special_char_curly)
{
	g_alice->setPasswordProvided(true);

	g_cmd->execute(g_alice, nickMsg("a{b}c"));

	cr_assert_str_eq(g_alice->getNickname().c_str(), "a{b}c");
}

Test(NickCommand, accepts_special_char_pipe)
{
	g_alice->setPasswordProvided(true);

	g_cmd->execute(g_alice, nickMsg("a|b"));

	cr_assert_str_eq(g_alice->getNickname().c_str(), "a|b");
}

Test(NickCommand, accepts_special_char_backslash)
{
	g_alice->setPasswordProvided(true);

	g_cmd->execute(g_alice, nickMsg("a\\b"));

	cr_assert_str_eq(g_alice->getNickname().c_str(), "a\\b");
}

Test(NickCommand, accepts_special_char_caret)
{
	g_alice->setPasswordProvided(true);

	g_cmd->execute(g_alice, nickMsg("a^b"));

	cr_assert_str_eq(g_alice->getNickname().c_str(), "a^b");
}

Test(NickCommand, accepts_special_char_backtick)
{
	g_alice->setPasswordProvided(true);

	g_cmd->execute(g_alice, nickMsg("a`b"));

	cr_assert_str_eq(g_alice->getNickname().c_str(), "a`b");
}

Test(NickCommand, accepts_special_char_dash)
{
	g_alice->setPasswordProvided(true);

	g_cmd->execute(g_alice, nickMsg("a-b"));

	cr_assert_str_eq(g_alice->getNickname().c_str(), "a-b");
}

// ============================================================================
// RFC 4.1.2: ERR_ERRONEUSNICKNAME (432)
// ============================================================================

Test(NickCommand, rejects_nickname_starting_with_number)
{
	g_alice->setPasswordProvided(true);

	g_cmd->execute(g_alice, nickMsg("123alice"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("432") != std::string::npos, "Expected ERR_ERRONEUSNICKNAME (432)");
	cr_assert(g_alice->getNickname().empty());
}

Test(NickCommand, rejects_nickname_too_long)
{
	g_alice->setPasswordProvided(true);

	g_cmd->execute(g_alice, nickMsg("abcdefghij")); // 10 chars, max is 9

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("432") != std::string::npos, "Expected ERR_ERRONEUSNICKNAME (432)");
}

Test(NickCommand, rejects_nickname_with_invalid_chars)
{
	g_alice->setPasswordProvided(true);

	g_cmd->execute(g_alice, nickMsg("alice@bob"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("432") != std::string::npos, "Expected ERR_ERRONEUSNICKNAME (432)");
}

Test(NickCommand, rejects_nickname_with_space)
{
	g_alice->setPasswordProvided(true);

	g_cmd->execute(g_alice, nickMsg("ali ce"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("432") != std::string::npos, "Expected ERR_ERRONEUSNICKNAME (432)");
}

Test(NickCommand, rejects_nickname_starting_with_dash)
{
	g_alice->setPasswordProvided(true);

	g_cmd->execute(g_alice, nickMsg("-alice"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("432") != std::string::npos, "Expected ERR_ERRONEUSNICKNAME (432)");
}

Test(NickCommand, rejects_empty_nickname_after_param)
{
	g_alice->setPasswordProvided(true);

	g_cmd->execute(g_alice, nickMsg(""));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("431") != std::string::npos || buffer.find("432") != std::string::npos,
			  "Expected ERR_NONICKNAMEGIVEN (431) or ERR_ERRONEUSNICKNAME (432)");
}

// ============================================================================
// RFC 4.1.2: ERR_NICKNAMEINUSE (433)
// ============================================================================

Test(NickCommand, rejects_nickname_in_use)
{
	registerClient(g_alice, "alice");
	g_bob->setPasswordProvided(true);

	g_cmd->execute(g_bob, nickMsg("alice"));

	std::string buffer = g_bob->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("433") != std::string::npos, "Expected ERR_NICKNAMEINUSE (433)");
	cr_assert(g_bob->getNickname().empty());
}

Test(NickCommand, rejects_nickname_in_use_case_insensitive)
{
	registerClient(g_alice, "Alice");
	g_bob->setPasswordProvided(true);

	g_cmd->execute(g_bob, nickMsg("ALICE"));

	std::string buffer = g_bob->getBuffer().getWriteBuffer();
	// IRC is case-insensitive for nicknames per RFC
	cr_assert(buffer.find("433") != std::string::npos, "Expected ERR_NICKNAMEINUSE (433)");
}

Test(NickCommand, allows_same_nick_from_same_client)
{
	registerClient(g_alice, "alice");

	g_cmd->execute(g_alice, nickMsg("alice"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("433") == std::string::npos, "Should not send ERR_NICKNAMEINUSE to self");
}

// ============================================================================
// RFC 4.1.2: ERR_NONICKNAMEGIVEN (431) / ERR_NEEDMOREPARAMS (461)
// ============================================================================

Test(NickCommand, error_no_parameters)
{
	g_alice->setPasswordProvided(true);

	Message msg;
	msg.m_command = "NICK";
	// No params

	g_cmd->execute(g_alice, msg);

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("461") != std::string::npos || buffer.find("431") != std::string::npos,
			  "Expected ERR_NEEDMOREPARAMS (461) or ERR_NONICKNAMEGIVEN (431)");
}

// ============================================================================
// Registration completion: RPL_WELCOME (001)
// ============================================================================

Test(NickCommand, sends_welcome_on_registration_complete)
{
	g_alice->setPasswordProvided(true);
	g_alice->setUsername("alice");
	// Only missing NICK

	g_cmd->execute(g_alice, nickMsg("alice"));

	cr_assert(g_alice->isRegistered());
	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("001") != std::string::npos, "Expected RPL_WELCOME (001)");
}

Test(NickCommand, no_welcome_without_password)
{
	// No password
	g_alice->setUsername("alice");

	g_cmd->execute(g_alice, nickMsg("alice"));

	cr_assert_not(g_alice->isRegistered());
}

Test(NickCommand, no_welcome_without_username)
{
	g_alice->setPasswordProvided(true);
	// No username

	g_cmd->execute(g_alice, nickMsg("alice"));

	cr_assert_not(g_alice->isRegistered());
}

// ============================================================================
// Nickname change notifications
// ============================================================================

Test(NickCommand, nickname_change_after_registration)
{
	registerClient(g_alice, "alice");

	g_cmd->execute(g_alice, nickMsg("alice2"));

	cr_assert_str_eq(g_alice->getNickname().c_str(), "alice2");
	cr_assert(g_alice->isRegistered());
}

// ============================================================================
// Edge cases
// ============================================================================

Test(NickCommand, single_char_nickname)
{
	g_alice->setPasswordProvided(true);

	g_cmd->execute(g_alice, nickMsg("A"));

	cr_assert_str_eq(g_alice->getNickname().c_str(), "A");
}

Test(NickCommand, all_special_chars)
{
	g_alice->setPasswordProvided(true);

	g_cmd->execute(g_alice, nickMsg("a[]\\`^{}"));

	cr_assert_str_eq(g_alice->getNickname().c_str(), "a[]\\`^{}");
}

Test(NickCommand, works_before_password)
{
	// Per RFC, NICK can be sent before registration is complete
	// but user won't be registered until PASS is provided

	g_cmd->execute(g_alice, nickMsg("alice"));

	cr_assert_str_eq(g_alice->getNickname().c_str(), "alice");
	cr_assert_not(g_alice->isRegistered());
}
