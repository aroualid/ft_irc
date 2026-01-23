#include "commands/CommandFactory.hpp"
#include "commands/CommandType.hpp"
#include "commands/PongCommand.hpp"
#include "mocks/Client.hpp"
#include "mocks/Server.hpp"
#include <criterion/criterion.h>

TestSuite(PongCommand);

// Critical test: Custom error for missing origin
Test(PongCommand, error_no_origin)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost");
	client.setNickname("alice");

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::PONG, server);
	cr_assert_not_null(cmd, "Factory failed to create PONG command");

	Message msg;
	msg.m_command = "PONG";
	// No parameters

	cmd->execute(&client, msg);

	std::string buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("409") != std::string::npos,
			  "Should send ERR_NOORIGIN (409), not ERR_NEEDMOREPARAMS (461)");
	cr_assert(buffer.find("No origin specified") != std::string::npos,
			  "Should include error message");
	delete cmd;
}

// Valid PONG is silently accepted
Test(PongCommand, valid_pong_accepted)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost");
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::PONG, server);

	Message msg;
	msg.m_command = "PONG";
	msg.m_params.push_back("server.hostname");

	cmd->execute(&client, msg);

	// Should not crash, should not send any error
	std::string buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("409") == std::string::npos, "Should not send error for valid PONG");
	delete cmd;
}

// PONG works before authentication (important!)
Test(PongCommand, works_before_authentication)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost");
	// Not authenticated, no nickname

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::PONG, server);

	Message msg;
	msg.m_command = "PONG";
	msg.m_params.push_back("test");

	cmd->execute(&client, msg);

	// Should work without errors
	std::string buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("409") == std::string::npos, "Should accept PONG before authentication");
	delete cmd;
}