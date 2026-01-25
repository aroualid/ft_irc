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
	g_server = new Server(6667, "secretpass");
	g_alice = new ClientMock(3, "localhost", *g_server);
	g_cmd = CommandFactory::getInstance().createCommand(irc::PASS, *g_server);
}

static void teardown(void)
{
	delete g_cmd;
	delete g_alice;
	delete g_server;
}

// Helper to build a PASS message
static Message passMsg(const char* password)
{
	Message msg;
	msg.m_command = "PASS";
	if (password)
		msg.m_params.push_back(password);
	return msg;
}

TestSuite(PassCommand, .init = setup, .fini = teardown);

// ============================================================================
// RFC 4.1.1: Basic functionality
// ============================================================================

Test(PassCommand, factory_creates_pass_command)
{
	cr_assert_not_null(g_cmd, "Factory failed to create PASS command. Is it registered?");
	cr_assert_str_eq(g_cmd->getName(), "PASS");
}

Test(PassCommand, correct_password_authenticates)
{
	g_cmd->execute(g_alice, passMsg("secretpass"));

	cr_assert(g_alice->isPasswordProvided(),
			  "Client should be authenticated after correct password");
}

Test(PassCommand, wrong_password_rejected)
{
	g_cmd->execute(g_alice, passMsg("wrongpass"));

	cr_assert_not(g_alice->isPasswordProvided(),
				  "Client should not be authenticated with wrong password");
}

// ============================================================================
// RFC 4.1.1: Multiple PASS commands - only last one counts
// ============================================================================

Test(PassCommand, multiple_pass_last_counts_correct)
{
	g_cmd->execute(g_alice, passMsg("wrong1"));
	g_cmd->execute(g_alice, passMsg("wrong2"));
	g_cmd->execute(g_alice, passMsg("secretpass"));

	cr_assert(g_alice->isPasswordProvided(), "Last correct password should authenticate");
}

Test(PassCommand, multiple_pass_last_counts_wrong)
{
	g_cmd->execute(g_alice, passMsg("secretpass"));
	g_cmd->execute(g_alice, passMsg("wrongpass"));

	// Only last password matters
	cr_assert_not(g_alice->isPasswordProvided(), "Last wrong password should not authenticate");
}

// ============================================================================
// RFC 4.1.1: ERR_ALREADYREGISTRED (462)
// ============================================================================

Test(PassCommand, error_already_registered)
{
	// Fully register the client
	g_alice->setPasswordProvided(true);
	g_alice->setNickname("alice");
	g_alice->setUsername("alice");

	g_cmd->execute(g_alice, passMsg("secretpass"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("462") != std::string::npos, "Expected ERR_ALREADYREGISTRED (462)");
}

// ============================================================================
// RFC 4.1.1: ERR_NEEDMOREPARAMS (461)
// ============================================================================

Test(PassCommand, error_no_parameters)
{
	Message msg;
	msg.m_command = "PASS";
	// No params

	g_cmd->execute(g_alice, msg);

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("461") != std::string::npos, "Expected ERR_NEEDMOREPARAMS (461)");
	cr_assert_not(g_alice->isPasswordProvided());
}

// ============================================================================
// RFC 4.1.1: PASS must precede NICK/USER
// ============================================================================

Test(PassCommand, works_before_nick_user)
{
	g_cmd->execute(g_alice, passMsg("secretpass"));

	cr_assert(g_alice->isPasswordProvided(), "PASS should work before NICK/USER");
	cr_assert_not(g_alice->isRegistered(), "Client should not be registered yet");
}

// ============================================================================
// Edge cases
// ============================================================================

Test(PassCommand, empty_password_param)
{
	g_cmd->execute(g_alice, passMsg(""));

	// Empty password should not match non-empty server password
	cr_assert_not(g_alice->isPasswordProvided());
}

Test(PassCommand, password_with_spaces)
{
	// Create server with password containing spaces
	delete g_cmd;
	delete g_alice;
	delete g_server;

	g_server = new Server(6667, "pass with spaces");
	g_alice = new ClientMock(3, "localhost", *g_server);
	g_cmd = CommandFactory::getInstance().createCommand(irc::PASS, *g_server);

	g_cmd->execute(g_alice, passMsg("pass with spaces"));

	cr_assert(g_alice->isPasswordProvided(), "Password with spaces should work");
}

Test(PassCommand, case_sensitive_password)
{
	g_cmd->execute(g_alice, passMsg("SECRETPASS")); // uppercase

	cr_assert_not(g_alice->isPasswordProvided(), "Password should be case-sensitive");
}

Test(PassCommand, special_chars_in_password)
{
	delete g_cmd;
	delete g_alice;
	delete g_server;

	g_server = new Server(6667, "p@ss!#$%^&*()");
	g_alice = new ClientMock(3, "localhost", *g_server);
	g_cmd = CommandFactory::getInstance().createCommand(irc::PASS, *g_server);

	g_cmd->execute(g_alice, passMsg("p@ss!#$%^&*()"));

	cr_assert(g_alice->isPasswordProvided(), "Password with special chars should work");
}

// ============================================================================
// Integration with registration flow
// ============================================================================

Test(PassCommand, pass_then_nick_then_user_completes_registration)
{
	g_cmd->execute(g_alice, passMsg("secretpass"));
	cr_assert(g_alice->isPasswordProvided());
	cr_assert_not(g_alice->isRegistered());

	// Complete registration
	g_alice->setNickname("alice");
	g_alice->setUsername("alice");

	cr_assert(g_alice->isRegistered());
}

Test(PassCommand, requires_registration_false)
{
	// PASS should work for unregistered clients
	cr_assert_not(g_cmd->requiresRegistration());
}
