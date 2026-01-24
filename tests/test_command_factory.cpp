#include "commands/ACommand.hpp"
#include "commands/CommandFactory.hpp"
#include "commands/CommandRegistration.hpp"
#include "commands/CommandType.hpp"
#include "mocks/Server.hpp"
#include <criterion/criterion.h>

class MockCommand : public ACommand
{
public:
	MockCommand(IServer& server) : ACommand(server)
	{
	}
	~MockCommand()
	{
	}
	void doExecute(IClient* client, const Message& message)
	{
		(void)client;
		(void)message;
	}
	const char* getName() const
	{
		return "MOCK";
	}
	static ACommand* create(IServer& server)
	{
		return new MockCommand(server);
	}
};

class MockCommand2 : public ACommand
{
public:
	MockCommand2(IServer& server) : ACommand(server)
	{
	}
	~MockCommand2()
	{
	}
	void doExecute(IClient* client, const Message& message)
	{
		(void)client;
		(void)message;
	}
	const char* getName() const
	{
		return "MOCK2";
	}
	static ACommand* create(IServer& server)
	{
		return new MockCommand2(server);
	}
};

REGISTER_COMMAND(MockCommand, irc::MOCK, "MOCK");
REGISTER_COMMAND(MockCommand2, irc::MOCK2, "MOCK2");

/*
class NotACommand {
public:
	NotACommand() {}
	// This class does not inherit from ACommand and lacks a 'create' method.
};
REGISTER_COMMAND(NotACommand, "FAIL");
*/

TestSuite(CommandFactory);

Test(CommandFactory, has_command)
{
	cr_log_info("Testing CommandFactory::hasCommand()...");
	CommandFactory& factory = CommandFactory::getInstance();

	cr_assert(factory.hasCommand(irc::MOCK),
			  "hasCommand() should return true for a registered command.");
	cr_assert(factory.hasCommand(irc::MOCK2),
			  "hasCommand() should return true for a second registered command.");
	cr_assert_not(factory.hasCommand(irc::CMD_UNKNOWN),
				  "hasCommand() should return false for an unregistered command.");
}

Test(CommandFactory, create_command)
{
	cr_log_info("Testing CommandFactory::createCommand()...");
	CommandFactory& factory = CommandFactory::getInstance();
	Server mockServer(6667, "password");

	ACommand* mock_cmd = factory.createCommand(irc::MOCK, mockServer);
	cr_assert_not_null(mock_cmd, "Factory failed to create 'MOCK' command. Is it registered?");
	std::string cmd1_name = mock_cmd->getName();
	cr_assert_str_eq(cmd1_name.c_str(), "MOCK", "The created command has the correct name.");
	delete mock_cmd;

	ACommand* mock_cmd2 = factory.createCommand(irc::MOCK2, mockServer);
	cr_assert_not_null(mock_cmd2, "createCommand() should return a valid object for 'MOCK2'.");
	std::string cmd2_name = mock_cmd2->getName();
	cr_assert_str_eq(cmd2_name.c_str(), "MOCK2", "The created command has the correct name.");
	delete mock_cmd2;

	ACommand* non_existent_cmd = factory.createCommand(irc::CMD_UNKNOWN, mockServer);
	cr_assert_null(non_existent_cmd,
				   "createCommand() should return NULL for an unregistered command.");
}
