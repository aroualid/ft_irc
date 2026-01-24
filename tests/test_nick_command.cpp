#include "commands/ACommand.hpp"
#include "commands/CommandFactory.hpp"
#include "commands/CommandType.hpp"
#include "mocks/Client.hpp"
#include "mocks/Server.hpp"
#include "protocol/Message.hpp"
#include <criterion/criterion.h>

TestSuite(NickCommand);

// RFC 4.1.2: Sets nickname correctly
Test(NickCommand, sets_nickname_correctly)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost");
	client.setPasswordProvided(true);

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::NICK, server);
	cr_assert_not_null(cmd, "Factory failed to create NICK command");

	Message msg;
	msg.m_command = "NICK";
	msg.m_params.push_back("alice");

	cmd->execute(&client, msg);

	cr_assert_str_eq(client.getNickname().c_str(), "alice", "Nickname should be set to alice");
	delete cmd;
}

// RFC 4.1.2: Valid nickname format (starts with letter, max 9 chars)
Test(NickCommand, accepts_valid_nickname_format)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost");
	client.setPasswordProvided(true);

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::NICK, server);
	cr_assert_not_null(cmd, "Factory failed to create NICK command");

	Message msg;
	msg.m_command = "NICK";
	msg.m_params.push_back("Alice123");

	cmd->execute(&client, msg);

	cr_assert_str_eq(client.getNickname().c_str(), "Alice123", "Valid nickname should be accepted");
	delete cmd;
}

// RFC 4.1.2: ERR_ERRONEUSNICKNAME for invalid format
Test(NickCommand, rejects_nickname_starting_with_number)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost");
	client.setPasswordProvided(true);

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::NICK, server);
	cr_assert_not_null(cmd, "Factory failed to create NICK command");

	Message msg;
	msg.m_command = "NICK";
	msg.m_params.push_back("123alice");

	cmd->execute(&client, msg);

	// Should send ERR_ERRONEUSNICKNAME (432)
	std::string buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("432") != std::string::npos,
			  "Should send ERR_ERRONEUSNICKNAME for nickname starting with number");
	cr_assert(client.getNickname().empty(), "Nickname should not be set");
	delete cmd;
}

Test(NickCommand, rejects_nickname_too_long)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost");
	client.setPasswordProvided(true);

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::NICK, server);
	cr_assert_not_null(cmd, "Factory failed to create NICK command");

	Message msg;
	msg.m_command = "NICK";
	msg.m_params.push_back("ThisIsTooLong"); // More than 9 characters

	cmd->execute(&client, msg);

	// Should send ERR_ERRONEUSNICKNAME (432)
	std::string buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("432") != std::string::npos,
			  "Should send ERR_ERRONEUSNICKNAME for nickname over 9 characters");
	delete cmd;
}

// RFC 4.1.2: ERR_NICKNAMEINUSE if nickname already taken
Test(NickCommand, rejects_nickname_in_use)
{
	Server server(6667, "testpass");
	ClientMock client1(3, "localhost");
	ClientMock client2(4, "localhost");
	client1.setPasswordProvided(true);
	client2.setPasswordProvided(true);

	// Client1 takes "alice"
	client1.setNickname("alice");
	server.registerClient("alice", &client1);

	// Client2 tries to take "alice"
	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::NICK, server);
	cr_assert_not_null(cmd, "Factory failed to create NICK command");

	Message msg;
	msg.m_command = "NICK";
	msg.m_params.push_back("alice");

	cmd->execute(&client2, msg);

	// Should send ERR_NICKNAMEINUSE (433)
	std::string buffer = client2.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("433") != std::string::npos,
			  "Should send ERR_NICKNAMEINUSE when nickname already taken");
	cr_assert(client2.getNickname().empty(), "Nickname should not be set for client2");
	delete cmd;
}

// RFC 4.1.2: ERR_NONICKNAMEGIVEN if no nickname provided (handled by template method)
Test(NickCommand, error_if_no_parameters)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost");
	client.setPasswordProvided(true);

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::NICK, server);
	cr_assert_not_null(cmd, "Factory failed to create NICK command");

	Message msg;
	msg.m_command = "NICK";
	// No parameters

	cmd->execute(&client, msg);

	// Should send ERR_NEEDMOREPARAMS (461) via template method
	std::string buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("461") != std::string::npos,
			  "Should send ERR_NEEDMOREPARAMS when no nickname provided");
	delete cmd;
}

// Nickname changes are allowed
Test(NickCommand, allows_nickname_change)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost");
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");
	// Client is now registered

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::NICK, server);
	cr_assert_not_null(cmd, "Factory failed to create NICK command");

	Message msg;
	msg.m_command = "NICK";
	msg.m_params.push_back("bob");

	cmd->execute(&client, msg);

	cr_assert_str_eq(client.getNickname().c_str(), "bob",
					 "Registered client should be able to change nickname");
	delete cmd;
}

// Completes registration when PASS+NICK+USER are all provided
Test(NickCommand, sends_welcome_on_registration)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost");
	client.setPasswordProvided(true);
	client.setUsername("alice");
	// Now just need NICK to complete registration

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::NICK, server);
	cr_assert_not_null(cmd, "Factory failed to create NICK command");

	Message msg;
	msg.m_command = "NICK";
	msg.m_params.push_back("alice");

	cmd->execute(&client, msg);

	cr_assert(client.isRegistered(), "Client should be registered after PASS+NICK+USER");
	// Should send RPL_WELCOME (001)
	std::string buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("001") != std::string::npos,
			  "Should send RPL_WELCOME when registration completes");
	delete cmd;
}

// Special characters allowed in nickname
Test(NickCommand, accepts_special_chars_in_nickname)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost");
	client.setPasswordProvided(true);

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::NICK, server);
	cr_assert_not_null(cmd, "Factory failed to create NICK command");

	Message msg;
	msg.m_command = "NICK";
	msg.m_params.push_back("alice[0]"); // Special chars allowed: - [ ] \ ` ^ { } |

	cmd->execute(&client, msg);

	cr_assert_str_eq(client.getNickname().c_str(), "alice[0]",
					 "Nickname with special chars should be accepted");
	delete cmd;
}

// Requires authentication (password)
Test(NickCommand, requires_password_first)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost");
	// No password provided

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::NICK, server);
	cr_assert_not_null(cmd, "Factory failed to create NICK command");

	Message msg;
	msg.m_command = "NICK";
	msg.m_params.push_back("alice");

	// Command should still execute but client won't be able to register
	cmd->execute(&client, msg);

	// Nickname is set but client can't register without password
	cr_assert_str_eq(client.getNickname().c_str(), "alice", "Nickname should be set");
	cr_assert_not(client.isRegistered(), "Client should not be registered without password");
	delete cmd;
}
