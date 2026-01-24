#include "commands/ACommand.hpp"
#include "commands/CommandFactory.hpp"
#include "commands/CommandType.hpp"
#include "mocks/Client.hpp"
#include "mocks/Server.hpp"
#include "protocol/Message.hpp"
#include <criterion/criterion.h>

TestSuite(UserCommand);

// RFC 4.1.3: Sets username and realname correctly
Test(UserCommand, sets_username_and_realname)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost");
	client.setPasswordProvided(true);

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::USER, server);
	cr_assert_not_null(cmd, "Factory failed to create USER command");

	Message msg;
	msg.m_command = "USER";
	msg.m_params.push_back("alice");	   // username
	msg.m_params.push_back("hostname");	   // hostname (ignored)
	msg.m_params.push_back("servername");  // servername (ignored)
	msg.m_params.push_back("Alice Smith"); // realname

	cmd->execute(&client, msg);

	cr_assert_str_eq(client.getUsername().c_str(), "alice", "Username should be set");
	cr_assert_str_eq(client.getRealname().c_str(), "Alice Smith", "Realname should be set");
	delete cmd;
}

// RFC 4.1.3: Requires all 4 parameters
Test(UserCommand, requires_four_parameters)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost");
	client.setPasswordProvided(true);

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::USER, server);
	cr_assert_not_null(cmd, "Factory failed to create USER command");

	Message msg;
	msg.m_command = "USER";
	msg.m_params.push_back("alice");
	msg.m_params.push_back("hostname");
	msg.m_params.push_back("servername");
	// Missing realname parameter

	cmd->execute(&client, msg);

	// Should send ERR_NEEDMOREPARAMS (461)
	std::string buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("461") != std::string::npos,
			  "Should send ERR_NEEDMOREPARAMS when missing parameters");
	delete cmd;
}

// RFC 4.1.3: ERR_ALREADYREGISTRED if already registered
Test(UserCommand, error_if_already_registered)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost");
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");
	// Client is now registered

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::USER, server);
	cr_assert_not_null(cmd, "Factory failed to create USER command");

	Message msg;
	msg.m_command = "USER";
	msg.m_params.push_back("bob");
	msg.m_params.push_back("hostname");
	msg.m_params.push_back("servername");
	msg.m_params.push_back("Bob Smith");

	cmd->execute(&client, msg);

	// Should send ERR_ALREADYREGISTRED (462)
	std::string buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("462") != std::string::npos,
			  "Should send ERR_ALREADYREGISTRED when already registered");
	// Username should not change
	cr_assert_str_eq(client.getUsername().c_str(), "alice",
					 "Username should not change when already registered");
	delete cmd;
}

// Completes registration when PASS+NICK+USER are all provided
Test(UserCommand, sends_welcome_on_registration)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost");
	client.setPasswordProvided(true);
	client.setNickname("alice");
	// Now just need USER to complete registration

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::USER, server);
	cr_assert_not_null(cmd, "Factory failed to create USER command");

	Message msg;
	msg.m_command = "USER";
	msg.m_params.push_back("alice");
	msg.m_params.push_back("hostname");
	msg.m_params.push_back("servername");
	msg.m_params.push_back("Alice Smith");

	cmd->execute(&client, msg);

	cr_assert(client.isRegistered(), "Client should be registered after PASS+NICK+USER");
	// Should send RPL_WELCOME (001)
	std::string buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("001") != std::string::npos,
			  "Should send RPL_WELCOME when registration completes");
	delete cmd;
}

// Realname can contain spaces
Test(UserCommand, realname_with_spaces)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost");
	client.setPasswordProvided(true);

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::USER, server);
	cr_assert_not_null(cmd, "Factory failed to create USER command");

	Message msg;
	msg.m_command = "USER";
	msg.m_params.push_back("alice");
	msg.m_params.push_back("hostname");
	msg.m_params.push_back("servername");
	msg.m_params.push_back("Alice Marie Smith");

	cmd->execute(&client, msg);

	cr_assert_str_eq(client.getRealname().c_str(), "Alice Marie Smith",
					 "Realname with spaces should be preserved");
	delete cmd;
}

// Empty username triggers error
Test(UserCommand, rejects_empty_username)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost");
	client.setPasswordProvided(true);

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::USER, server);
	cr_assert_not_null(cmd, "Factory failed to create USER command");

	Message msg;
	msg.m_command = "USER";
	msg.m_params.push_back(""); // Empty username
	msg.m_params.push_back("hostname");
	msg.m_params.push_back("servername");
	msg.m_params.push_back("Alice Smith");

	cmd->execute(&client, msg);

	// Should send ERR_NEEDMOREPARAMS (461)
	std::string buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("461") != std::string::npos, "Should send error when username is empty");
	delete cmd;
}

// Requires authentication (password)
Test(UserCommand, requires_authentication)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost");
	// No password provided

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::USER, server);
	cr_assert_not_null(cmd, "Factory failed to create USER command");

	Message msg;
	msg.m_command = "USER";
	msg.m_params.push_back("alice");
	msg.m_params.push_back("hostname");
	msg.m_params.push_back("servername");
	msg.m_params.push_back("Alice Smith");

	// Command should still execute but client won't be able to register
	cmd->execute(&client, msg);

	cr_assert_str_eq(client.getUsername().c_str(), "alice", "Username should be set");
	cr_assert_not(client.isRegistered(), "Client should not be registered without password");
	delete cmd;
}

// Full registration flow: PASS -> NICK -> USER
Test(UserCommand, complete_registration_flow)
{
	Server server(6667, "secret");
	ClientMock client(3, "localhost");

	// Step 1: PASS
	ACommand* passCmd = CommandFactory::getInstance()->createCommand(irc::PASS, server);
	Message passMsg;
	passMsg.m_command = "PASS";
	passMsg.m_params.push_back("secret");
	passCmd->execute(&client, passMsg);
	cr_assert(client.isPasswordProvided(), "Should be authenticated after PASS");
	cr_assert_not(client.isRegistered(), "Should not be registered yet");
	delete passCmd;

	// Step 2: NICK
	ACommand* nickCmd = CommandFactory::getInstance()->createCommand(irc::NICK, server);
	Message nickMsg;
	nickMsg.m_command = "NICK";
	nickMsg.m_params.push_back("alice");
	nickCmd->execute(&client, nickMsg);
	cr_assert_str_eq(client.getNickname().c_str(), "alice", "Nickname should be set");
	cr_assert_not(client.isRegistered(), "Should not be registered yet");
	delete nickCmd;

	// Step 3: USER (completes registration)
	ACommand* userCmd = CommandFactory::getInstance()->createCommand(irc::USER, server);
	Message userMsg;
	userMsg.m_command = "USER";
	userMsg.m_params.push_back("alice");
	userMsg.m_params.push_back("hostname");
	userMsg.m_params.push_back("servername");
	userMsg.m_params.push_back("Alice Smith");
	userCmd->execute(&client, userMsg);

	cr_assert(client.isRegistered(), "Should be registered after PASS+NICK+USER");
	std::string buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("001") != std::string::npos,
			  "Should send RPL_WELCOME on successful registration");
	delete userCmd;
}

// Alternative order: PASS -> USER -> NICK also works
Test(UserCommand, registration_alternative_order)
{
	Server server(6667, "secret");
	ClientMock client(3, "localhost");

	// Step 1: PASS
	ACommand* passCmd = CommandFactory::getInstance()->createCommand(irc::PASS, server);
	Message passMsg;
	passMsg.m_command = "PASS";
	passMsg.m_params.push_back("secret");
	passCmd->execute(&client, passMsg);
	delete passCmd;

	// Step 2: USER (before NICK)
	ACommand* userCmd = CommandFactory::getInstance()->createCommand(irc::USER, server);
	Message userMsg;
	userMsg.m_command = "USER";
	userMsg.m_params.push_back("alice");
	userMsg.m_params.push_back("hostname");
	userMsg.m_params.push_back("servername");
	userMsg.m_params.push_back("Alice Smith");
	userCmd->execute(&client, userMsg);
	cr_assert_not(client.isRegistered(), "Should not be registered without NICK");
	delete userCmd;

	// Step 3: NICK (completes registration)
	ACommand* nickCmd = CommandFactory::getInstance()->createCommand(irc::NICK, server);
	Message nickMsg;
	nickMsg.m_command = "NICK";
	nickMsg.m_params.push_back("alice");
	nickCmd->execute(&client, nickMsg);

	cr_assert(client.isRegistered(), "Should be registered after PASS+USER+NICK");
	std::string buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("001") != std::string::npos,
			  "Should send RPL_WELCOME on successful registration");
	delete nickCmd;
}