#include "commands/ACommand.hpp"
#include "commands/CommandFactory.hpp"
#include "commands/CommandType.hpp"
#include "mocks/Client.hpp"
#include "mocks/Server.hpp"
#include "protocol/Message.hpp"
#include <criterion/criterion.h>

// Test fixture
static Server* g_server;
static ClientMock* g_alice;
static ACommand* g_cmd;

static void setup(void)
{
	g_server = new Server(6667, "testpass");
	g_alice = new ClientMock(3, "localhost", *g_server);
	g_cmd = CommandFactory::getInstance().createCommand(irc::USER, *g_server);
}

static void teardown(void)
{
	delete g_cmd;
	delete g_alice;
	delete g_server;
}

// Helper to build a USER message
// USER <username> <hostname> <servername> <realname>
static Message userMsg(const char* username, const char* hostname = "host",
					   const char* servername = "server", const char* realname = NULL)
{
	Message msg;
	msg.m_command = "USER";
	if (username)
		msg.m_params.push_back(username);
	if (hostname)
		msg.m_params.push_back(hostname);
	if (servername)
		msg.m_params.push_back(servername);
	if (realname)
		msg.m_params.push_back(realname);
	return msg;
}

TestSuite(UserCommand, .init = setup, .fini = teardown);

// ============================================================================
// RFC 4.1.3: Basic functionality
// ============================================================================

Test(UserCommand, factory_creates_user_command)
{
	cr_assert_not_null(g_cmd, "Factory failed to create USER command. Is it registered?");
	cr_assert_str_eq(g_cmd->getName(), "USER");
}

Test(UserCommand, sets_username_and_realname)
{
	g_alice->setPasswordProvided(true);

	g_cmd->execute(g_alice, userMsg("alice", "hostname", "servername", "Alice Smith"));

	cr_assert_str_eq(g_alice->getUsername().c_str(), "alice");
	cr_assert_str_eq(g_alice->getRealname().c_str(), "Alice Smith");
}

Test(UserCommand, hostname_and_servername_ignored)
{
	g_alice->setPasswordProvided(true);

	// Per RFC, hostname and servername are ignored for direct client connections
	g_cmd->execute(g_alice, userMsg("alice", "ignored_host", "ignored_server", "Alice"));

	cr_assert_str_eq(g_alice->getUsername().c_str(), "alice");
}

// ============================================================================
// RFC 4.1.3: ERR_NEEDMOREPARAMS (461)
// ============================================================================

Test(UserCommand, error_missing_all_params)
{
	g_alice->setPasswordProvided(true);

	Message msg;
	msg.m_command = "USER";
	// No params

	g_cmd->execute(g_alice, msg);

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("461") != std::string::npos, "Expected ERR_NEEDMOREPARAMS (461)");
}

Test(UserCommand, error_missing_realname)
{
	g_alice->setPasswordProvided(true);

	Message msg;
	msg.m_command = "USER";
	msg.m_params.push_back("alice");
	msg.m_params.push_back("hostname");
	msg.m_params.push_back("servername");
	// Missing realname

	g_cmd->execute(g_alice, msg);

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("461") != std::string::npos, "Expected ERR_NEEDMOREPARAMS (461)");
}

Test(UserCommand, error_empty_username)
{
	g_alice->setPasswordProvided(true);

	g_cmd->execute(g_alice, userMsg("", "hostname", "servername", "Alice Smith"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("461") != std::string::npos, "Expected ERR_NEEDMOREPARAMS (461)");
}

// ============================================================================
// RFC 4.1.3: ERR_ALREADYREGISTRED (462)
// ============================================================================

Test(UserCommand, error_already_registered)
{
	g_alice->setPasswordProvided(true);
	g_alice->setNickname("alice");
	g_alice->setUsername("alice");
	// Now registered

	g_cmd->execute(g_alice, userMsg("bob", "hostname", "servername", "Bob Smith"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("462") != std::string::npos, "Expected ERR_ALREADYREGISTRED (462)");
	// Username should not change
	cr_assert_str_eq(g_alice->getUsername().c_str(), "alice");
}

// ============================================================================
// Registration completion: RPL_WELCOME (001)
// ============================================================================

Test(UserCommand, sends_welcome_on_registration_complete)
{
	g_alice->setPasswordProvided(true);
	g_alice->setNickname("alice");
	// Only missing USER

	g_cmd->execute(g_alice, userMsg("alice", "host", "server", "Alice Smith"));

	cr_assert(g_alice->isRegistered());
	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("001") != std::string::npos, "Expected RPL_WELCOME (001)");
}

Test(UserCommand, no_welcome_without_password)
{
	// No password
	g_alice->setNickname("alice");

	g_cmd->execute(g_alice, userMsg("alice", "host", "server", "Alice"));

	cr_assert_not(g_alice->isRegistered());
}

Test(UserCommand, no_welcome_without_nickname)
{
	g_alice->setPasswordProvided(true);
	// No nickname

	g_cmd->execute(g_alice, userMsg("alice", "host", "server", "Alice"));

	cr_assert_not(g_alice->isRegistered());
}

// ============================================================================
// RFC 4.1.3: Realname with spaces
// ============================================================================

Test(UserCommand, realname_with_spaces)
{
	g_alice->setPasswordProvided(true);

	g_cmd->execute(g_alice, userMsg("alice", "host", "server", "Alice Marie Smith"));

	cr_assert_str_eq(g_alice->getRealname().c_str(), "Alice Marie Smith");
}

Test(UserCommand, realname_empty)
{
	g_alice->setPasswordProvided(true);

	g_cmd->execute(g_alice, userMsg("alice", "host", "server", ""));

	// Empty realname should be allowed
	cr_assert_str_eq(g_alice->getUsername().c_str(), "alice");
}

// ============================================================================
// Registration flow tests
// ============================================================================

Test(UserCommand, complete_registration_pass_nick_user)
{
	// Step 1: PASS
	ACommand* passCmd = CommandFactory::getInstance().createCommand(irc::PASS, *g_server);
	Message passMsg;
	passMsg.m_command = "PASS";
	passMsg.m_params.push_back("testpass");
	passCmd->execute(g_alice, passMsg);
	cr_assert(g_alice->isPasswordProvided());
	cr_assert_not(g_alice->isRegistered());
	delete passCmd;

	// Step 2: NICK
	ACommand* nickCmd = CommandFactory::getInstance().createCommand(irc::NICK, *g_server);
	Message nickMsg;
	nickMsg.m_command = "NICK";
	nickMsg.m_params.push_back("alice");
	nickCmd->execute(g_alice, nickMsg);
	cr_assert_str_eq(g_alice->getNickname().c_str(), "alice");
	cr_assert_not(g_alice->isRegistered());
	delete nickCmd;

	// Step 3: USER
	g_cmd->execute(g_alice, userMsg("alice", "host", "server", "Alice Smith"));

	cr_assert(g_alice->isRegistered());
	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("001") != std::string::npos, "Expected RPL_WELCOME (001)");
}

Test(UserCommand, complete_registration_pass_user_nick)
{
	// Alternative order: PASS -> USER -> NICK

	// Step 1: PASS
	ACommand* passCmd = CommandFactory::getInstance().createCommand(irc::PASS, *g_server);
	Message passMsg;
	passMsg.m_command = "PASS";
	passMsg.m_params.push_back("testpass");
	passCmd->execute(g_alice, passMsg);
	delete passCmd;

	// Step 2: USER (before NICK)
	g_cmd->execute(g_alice, userMsg("alice", "host", "server", "Alice Smith"));
	cr_assert_not(g_alice->isRegistered());

	// Step 3: NICK (completes registration)
	ACommand* nickCmd = CommandFactory::getInstance().createCommand(irc::NICK, *g_server);
	Message nickMsg;
	nickMsg.m_command = "NICK";
	nickMsg.m_params.push_back("alice");
	nickCmd->execute(g_alice, nickMsg);
	delete nickCmd;

	cr_assert(g_alice->isRegistered());
	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("001") != std::string::npos, "Expected RPL_WELCOME (001)");
}

// ============================================================================
// Edge cases
// ============================================================================

Test(UserCommand, username_with_special_chars)
{
	g_alice->setPasswordProvided(true);

	g_cmd->execute(g_alice, userMsg("alice_user", "host", "server", "Alice"));

	cr_assert_str_eq(g_alice->getUsername().c_str(), "alice_user");
}

Test(UserCommand, requires_registration_false)
{
	// USER should work for unregistered clients (that's how they register!)
	cr_assert_not(g_cmd->requiresRegistration());
}

Test(UserCommand, realname_with_colons)
{
	g_alice->setPasswordProvided(true);

	g_cmd->execute(g_alice, userMsg("alice", "host", "server", "Alice: The Great"));

	cr_assert_str_eq(g_alice->getRealname().c_str(), "Alice: The Great");
}

Test(UserCommand, long_realname)
{
	g_alice->setPasswordProvided(true);

	std::string longRealname = "This is a very long realname that contains many words and spaces";
	g_cmd->execute(g_alice, userMsg("alice", "host", "server", longRealname.c_str()));

	cr_assert_str_eq(g_alice->getRealname().c_str(), longRealname.c_str());
}
