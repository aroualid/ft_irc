#include "commands/CommandFactory.hpp"
#include "commands/CommandType.hpp"
#include "commands/PingCommand.hpp"
#include "mocks/Client.hpp"
#include "mocks/Server.hpp"
#include <criterion/criterion.h>

TestSuite(PingCommand);

// PING with valid origin responds with PONG
Test(PingCommand, basic_ping_receives_pong)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost", server);

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::PING, server);
	cr_assert_not_null(cmd, "Factory failed to create PING command");

	Message msg;
	msg.m_command = "PING";
	msg.m_params.push_back("client.hostname");

	cmd->execute(&client, msg);

	std::string buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("PONG") != std::string::npos, "Should respond with PONG");
	cr_assert(buffer.find("client.hostname") != std::string::npos, "PONG should echo the origin");
	delete cmd;
}

// PING without parameters sends ERR_NOORIGIN
Test(PingCommand, error_no_origin)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost", server);
	client.setNickname("alice");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::PING, server);

	Message msg;
	msg.m_command = "PING";
	// No parameters

	cmd->execute(&client, msg);

	std::string buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("409") != std::string::npos, "Should send ERR_NOORIGIN (409)");
	delete cmd;
}

// PING works before authentication
Test(PingCommand, works_before_authentication)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost", server);
	// Not authenticated

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::PING, server);

	Message msg;
	msg.m_command = "PING";
	msg.m_params.push_back("test");

	cmd->execute(&client, msg);

	std::string buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("PONG") != std::string::npos, "PING should work before authentication");
	delete cmd;
}
