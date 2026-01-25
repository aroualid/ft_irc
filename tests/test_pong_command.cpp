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
	g_cmd = CommandFactory::getInstance().createCommand(irc::PONG, *g_server);
}

static void teardown(void)
{
	delete g_cmd;
	delete g_alice;
	delete g_server;
}

// Helper to build a PONG message
static Message pongMsg(const char* origin = NULL, const char* destination = NULL)
{
	Message msg;
	msg.m_command = "PONG";
	if (origin)
		msg.m_params.push_back(origin);
	if (destination)
		msg.m_params.push_back(destination);
	return msg;
}

TestSuite(PongCommand, .init = setup, .fini = teardown);

// ============================================================================
// RFC 4.6.3: Basic functionality
// ============================================================================

Test(PongCommand, factory_creates_pong_command)
{
	cr_assert_not_null(g_cmd, "Factory failed to create PONG command. Is it registered?");
	cr_assert_str_eq(g_cmd->getName(), "PONG");
}

Test(PongCommand, valid_pong_accepted)
{
	g_alice->setPasswordProvided(true);
	g_alice->setNickname("alice");
	g_alice->setUsername("alice");

	g_cmd->execute(g_alice, pongMsg("server.hostname"));

	// Should not crash, should not send any error
	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("409") == std::string::npos, "Should not send error for valid PONG");
}

Test(PongCommand, pong_silently_accepted)
{
	g_alice->setPasswordProvided(true);
	g_alice->setNickname("alice");
	g_alice->setUsername("alice");

	g_cmd->execute(g_alice, pongMsg("origin"));

	// PONG should be silently accepted (no reply needed)
	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.empty() || buffer.find("PONG") == std::string::npos);
}

// ============================================================================
// RFC 4.6.3: ERR_NOORIGIN (409)
// ============================================================================

Test(PongCommand, error_no_origin)
{
	g_alice->setNickname("alice");

	Message msg;
	msg.m_command = "PONG";
	// No parameters

	g_cmd->execute(g_alice, msg);

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("409") != std::string::npos,
			  "Should send ERR_NOORIGIN (409), not ERR_NEEDMOREPARAMS (461)");
	cr_assert(buffer.find("No origin specified") != std::string::npos,
			  "Should include error message");
}

Test(PongCommand, empty_origin_accepted)
{
	// Empty string origin is technically valid (just an empty origin)
	// Different from missing origin entirely
	g_cmd->execute(g_alice, pongMsg(""));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	// Should not send error (empty origin is a valid parameter)
	cr_assert(buffer.find("409") == std::string::npos);
}

// ============================================================================
// RFC 4.6.3: Works before registration
// ============================================================================

Test(PongCommand, works_before_authentication)
{
	// Not authenticated, no nickname

	g_cmd->execute(g_alice, pongMsg("test"));

	// Should work without errors
	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("409") == std::string::npos, "Should accept PONG before authentication");
}

Test(PongCommand, works_before_password)
{
	// No password set

	g_cmd->execute(g_alice, pongMsg("origin"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("409") == std::string::npos);
}

Test(PongCommand, works_before_nick)
{
	g_alice->setPasswordProvided(true);
	// No nickname set

	g_cmd->execute(g_alice, pongMsg("origin"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("409") == std::string::npos);
}

// ============================================================================
// RFC 4.6.3: With destination (optional second parameter)
// ============================================================================

Test(PongCommand, with_destination)
{
	g_alice->setPasswordProvided(true);
	g_alice->setNickname("alice");
	g_alice->setUsername("alice");

	g_cmd->execute(g_alice, pongMsg("origin", "destination"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("409") == std::string::npos);
}

// ============================================================================
// Edge cases
// ============================================================================

Test(PongCommand, origin_with_special_chars)
{
	g_alice->setPasswordProvided(true);
	g_alice->setNickname("alice");
	g_alice->setUsername("alice");

	g_cmd->execute(g_alice, pongMsg("origin:1234"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("409") == std::string::npos);
}

Test(PongCommand, numeric_origin)
{
	g_alice->setPasswordProvided(true);
	g_alice->setNickname("alice");
	g_alice->setUsername("alice");

	g_cmd->execute(g_alice, pongMsg("1234567890"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("409") == std::string::npos);
}

Test(PongCommand, requires_registration_false)
{
	// PONG should work for unregistered clients
	cr_assert_not(g_cmd->requiresRegistration());
}
