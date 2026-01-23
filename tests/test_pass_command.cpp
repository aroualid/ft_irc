#include "commands/ACommand.hpp"
#include "commands/CommandFactory.hpp"
#include "commands/CommandType.hpp"
#include "mocks/Client.hpp"
#include "mocks/Server.hpp"
#include "protocol/Message.hpp"
#include <criterion/criterion.h>

TestSuite(PassCommand);

// RFC 4.1.1: PASS command sets connection password
Test(PassCommand, sets_password_correctly)
{
	Server server(6667, "secret123");
	ClientMock client(3, "localhost");

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::PASS, server);
	cr_assert_not_null(cmd, "Factory failed to create PASS command");

	Message msg;
	msg.m_command = "PASS";
	msg.m_params.push_back("secret123");

	cmd->execute(&client, msg);

	cr_assert(client.isPasswordProvided(), "Client should be authenticated after correct password");
	delete cmd;
}

Test(PassCommand, rejects_wrong_password)
{
	Server server(6667, "correctpass");
	ClientMock client(3, "localhost");

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::PASS, server);
	cr_assert_not_null(cmd, "Factory failed to create PASS command");

	Message msg;
	msg.m_command = "PASS";
	msg.m_params.push_back("wrongpass");

	cmd->execute(&client, msg);

	cr_assert_not(client.isPasswordProvided(),
				  "Client should not be authenticated with wrong password");
	delete cmd;
}

// RFC 4.1.1: Multiple PASS commands allowed, only last one counts
Test(PassCommand, multiple_pass_only_last_counts)
{
	Server server(6667, "finalpass");
	ClientMock client(3, "localhost");

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::PASS, server);
	cr_assert_not_null(cmd, "Factory failed to create PASS command");

	Message msg1;
	msg1.m_command = "PASS";
	msg1.m_params.push_back("wrongpass1");
	cmd->execute(&client, msg1);

	Message msg2;
	msg2.m_command = "PASS";
	msg2.m_params.push_back("wrongpass2");
	cmd->execute(&client, msg2);

	Message msg3;
	msg3.m_command = "PASS";
	msg3.m_params.push_back("finalpass");
	cmd->execute(&client, msg3);

	cr_assert(client.isPasswordProvided(),
			  "Client should be authenticated after final correct password");
	delete cmd;
}

// RFC 4.1.1: ERR_ALREADYREGISTRED if already registered
Test(PassCommand, error_if_already_registered)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost");
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");
	// Now client is registered

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::PASS, server);
	cr_assert_not_null(cmd, "Factory failed to create PASS command");

	Message msg;
	msg.m_command = "PASS";
	msg.m_params.push_back("testpass");

	cmd->execute(&client, msg);

	// Should send ERR_ALREADYREGISTRED (462)
	std::string buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("462") != std::string::npos,
			  "Should send ERR_ALREADYREGISTRED when already registered");
	delete cmd;
}

// RFC 4.1.1: ERR_NEEDMOREPARAMS if no password provided (handled by template method)
Test(PassCommand, error_if_no_parameters)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost");

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::PASS, server);
	cr_assert_not_null(cmd, "Factory failed to create PASS command");

	Message msg;
	msg.m_command = "PASS";
	// No parameters

	cmd->execute(&client, msg);

	// Should send ERR_NEEDMOREPARAMS (461) via template method
	std::string buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("461") != std::string::npos,
			  "Should send ERR_NEEDMOREPARAMS when no password provided");
	cr_assert_not(client.isPasswordProvided(), "Client should not be authenticated");
	delete cmd;
}

// RFC 4.1.1: PASS must be sent before NICK/USER
Test(PassCommand, must_be_sent_before_registration)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost");

	// This test verifies PASS works when sent first
	ACommand* passCmd = CommandFactory::getInstance()->createCommand(irc::PASS, server);
	cr_assert_not_null(passCmd, "Factory failed to create PASS command");

	Message passMsg;
	passMsg.m_command = "PASS";
	passMsg.m_params.push_back("testpass");
	passCmd->execute(&client, passMsg);

	cr_assert(client.isPasswordProvided(), "PASS should work before registration");
	cr_assert_not(client.isRegistered(), "Client should not yet be registered");
	delete passCmd;
}