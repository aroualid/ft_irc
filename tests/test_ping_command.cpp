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
	g_cmd = CommandFactory::getInstance().createCommand(irc::PING, *g_server);
}

static void teardown(void)
{
	delete g_cmd;
	delete g_alice;
	delete g_server;
}

// Helper to build a PING message
static Message pingMsg(const char* origin = NULL, const char* destination = NULL)
{
	Message msg;
	msg.m_command = "PING";
	if (origin)
		msg.m_params.push_back(origin);
	if (destination)
		msg.m_params.push_back(destination);
	return msg;
}

TestSuite(PingCommand, .init = setup, .fini = teardown);

// ============================================================================
// RFC 4.6.2: Basic functionality
// ============================================================================

Test(PingCommand, factory_creates_ping_command)
{
	cr_assert_not_null(g_cmd, "Factory failed to create PING command. Is it registered?");
	cr_assert_str_eq(g_cmd->getName(), "PING");
}

Test(PingCommand, basic_ping_receives_pong)
{
	g_cmd->execute(g_alice, pingMsg("client.hostname"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("PONG") != std::string::npos, "Should respond with PONG");
	cr_assert(buffer.find("client.hostname") != std::string::npos, "PONG should echo the origin");
}

Test(PingCommand, pong_echoes_origin)
{
	g_cmd->execute(g_alice, pingMsg("test-origin"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("test-origin") != std::string::npos);
}

// ============================================================================
// RFC 4.6.2: ERR_NOORIGIN (409)
// ============================================================================

Test(PingCommand, error_no_origin)
{
	g_alice->setNickname("alice");

	Message msg;
	msg.m_command = "PING";
	// No parameters

	g_cmd->execute(g_alice, msg);

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("409") != std::string::npos, "Should send ERR_NOORIGIN (409)");
}

Test(PingCommand, empty_origin_accepted)
{
	// Empty string origin is technically valid (just an empty origin)
	// Different from missing origin entirely
	g_cmd->execute(g_alice, pingMsg(""));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	// Should still respond with PONG (empty origin is a valid parameter)
	cr_assert(buffer.find("PONG") != std::string::npos);
}

// ============================================================================
// RFC 4.6.2: Works before registration
// ============================================================================

Test(PingCommand, works_before_registration)
{
	// Not authenticated, not registered

	g_cmd->execute(g_alice, pingMsg("test"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("PONG") != std::string::npos, "PING should work before authentication");
}

Test(PingCommand, works_before_password)
{
	// No password set

	g_cmd->execute(g_alice, pingMsg("origin"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("PONG") != std::string::npos);
}

Test(PingCommand, works_before_nick)
{
	g_alice->setPasswordProvided(true);
	// No nickname set

	g_cmd->execute(g_alice, pingMsg("origin"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("PONG") != std::string::npos);
}

// ============================================================================
// RFC 4.6.2: Works after registration
// ============================================================================

Test(PingCommand, works_after_registration)
{
	g_alice->setPasswordProvided(true);
	g_alice->setNickname("alice");
	g_alice->setUsername("alice");

	g_cmd->execute(g_alice, pingMsg("origin"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("PONG") != std::string::npos);
}

// ============================================================================
// RFC 4.6.2: With destination (optional second parameter)
// ============================================================================

Test(PingCommand, with_destination)
{
	g_cmd->execute(g_alice, pingMsg("origin", "destination"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("PONG") != std::string::npos);
}

// ============================================================================
// Edge cases
// ============================================================================

Test(PingCommand, origin_with_spaces)
{
	g_cmd->execute(g_alice, pingMsg("origin with spaces"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("PONG") != std::string::npos);
	cr_assert(buffer.find("origin with spaces") != std::string::npos);
}

Test(PingCommand, origin_with_special_chars)
{
	g_cmd->execute(g_alice, pingMsg("origin:1234"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("PONG") != std::string::npos);
	cr_assert(buffer.find("origin:1234") != std::string::npos);
}

Test(PingCommand, numeric_origin)
{
	g_cmd->execute(g_alice, pingMsg("1234567890"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("PONG") != std::string::npos);
	cr_assert(buffer.find("1234567890") != std::string::npos);
}

Test(PingCommand, requires_registration_false)
{
	// PING should work for unregistered clients
	cr_assert_not(g_cmd->requiresRegistration());
}
