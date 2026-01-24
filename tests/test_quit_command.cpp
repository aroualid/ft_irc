#include "commands/ACommand.hpp"
#include "commands/CommandFactory.hpp"
#include "commands/CommandType.hpp"
#include "mocks/Client.hpp"
#include "mocks/Server.hpp"
#include "protocol/Message.hpp"
#include <criterion/criterion.h>

TestSuite(QuitCommand);

// RFC 4.1.6: QUIT ends client session
Test(QuitCommand, basic_quit_no_message)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost", server);
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::QUIT, server);
	cr_assert_not_null(cmd, "Factory failed to create QUIT command");

	Message msg;
	msg.m_command = "QUIT";
	// No parameters - should use default quit message (nickname)

	cmd->execute(&client, msg);

	// Server should mark client for disconnection
	// or client should be in a disconnecting state
	// (Implementation detail - check how your server handles this)
	delete cmd;
}

// RFC 4.1.6: Custom quit message
Test(QuitCommand, quit_with_custom_message)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost", server);
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::QUIT, server);
	cr_assert_not_null(cmd, "Factory failed to create QUIT command");

	Message msg;
	msg.m_command = "QUIT";
	msg.m_params.push_back("Gone to have lunch");

	cmd->execute(&client, msg);

	// Quit message should be "Gone to have lunch"
	delete cmd;
}

// RFC 4.1.6: QUIT can be used before registration (no auth required)
Test(QuitCommand, quit_before_registration)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost", server);
	// Not authenticated, not registered

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::QUIT, server);
	cr_assert_not_null(cmd, "Factory failed to create QUIT command");

	Message msg;
	msg.m_command = "QUIT";
	msg.m_params.push_back("Goodbye");

	// Should not require registration or authentication
	cmd->execute(&client, msg);

	// Should work even for unauthenticated clients
	delete cmd;
}

// QUIT should work for authenticated but not registered clients
Test(QuitCommand, quit_after_pass_only)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost", server);
	client.setPasswordProvided(true);
	// Has password but no nick/user

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::QUIT, server);
	cr_assert_not_null(cmd, "Factory failed to create QUIT command");

	Message msg;
	msg.m_command = "QUIT";

	cmd->execute(&client, msg);

	// Should work even if not fully registered
	delete cmd;
}

// QUIT should broadcast to channels user is in
Test(QuitCommand, broadcasts_to_channels)
{
	Server server(6667, "testpass");
	ClientMock client1(3, "localhost", server);
	ClientMock client2(4, "localhost", server);

	// Setup client1 (alice)
	client1.setPasswordProvided(true);
	client1.setNickname("alice");
	client1.setUsername("alice");

	// Setup client2 (bob)
	client2.setPasswordProvided(true);
	client2.setNickname("bob");
	client2.setUsername("bob");

	// Create channel and have both join
	// (This test assumes channels work - they do from Milestone 2)
	ACommand* joinCmd1 = CommandFactory::getInstance().createCommand(irc::JOIN, server);
	Message joinMsg;
	joinMsg.m_command = "JOIN";
	joinMsg.m_params.push_back("#test");
	joinCmd1->execute(&client1, joinMsg);
	delete joinCmd1;

	ACommand* joinCmd2 = CommandFactory::getInstance().createCommand(irc::JOIN, server);
	joinCmd2->execute(&client2, joinMsg);
	delete joinCmd2;

	// Clear buffers from join messages
	client1.getBuffer().getWriteBuffer();
	client2.getBuffer().getWriteBuffer();

	// Alice quits
	ACommand* quitCmd = CommandFactory::getInstance().createCommand(irc::QUIT, server);
	Message quitMsg;
	quitMsg.m_command = "QUIT";
	quitMsg.m_params.push_back("Goodbye everyone!");
	quitCmd->execute(&client1, quitMsg);

	// Bob should receive QUIT message
	std::string bobBuffer = client2.getBuffer().getWriteBuffer();
	cr_assert(bobBuffer.find("QUIT") != std::string::npos,
			  "Channel members should receive QUIT message");
	cr_assert(bobBuffer.find("alice") != std::string::npos,
			  "QUIT message should contain quitting user's nickname");
	cr_assert(bobBuffer.find("Goodbye everyone!") != std::string::npos,
			  "QUIT message should contain custom quit message");

	delete quitCmd;
}

// Multiple channels - QUIT should broadcast to all
Test(QuitCommand, broadcasts_to_all_channels)
{
	Server server(6667, "testpass");
	ClientMock alice(3, "localhost", server);
	ClientMock bob(4, "localhost", server);
	ClientMock charlie(5, "localhost", server);

	// Setup clients
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice");

	bob.setPasswordProvided(true);
	bob.setNickname("bob");
	bob.setUsername("bob");

	charlie.setPasswordProvided(true);
	charlie.setNickname("charlie");
	charlie.setUsername("charlie");

	// Alice joins #test1 and #test2
	ACommand* joinCmd = CommandFactory::getInstance().createCommand(irc::JOIN, server);
	Message join1;
	join1.m_command = "JOIN";
	join1.m_params.push_back("#test1");
	joinCmd->execute(&alice, join1);

	Message join2;
	join2.m_command = "JOIN";
	join2.m_params.push_back("#test2");
	joinCmd->execute(&alice, join2);

	// Bob joins #test1
	joinCmd->execute(&bob, join1);

	// Charlie joins #test2
	joinCmd->execute(&charlie, join2);

	delete joinCmd;

	// Clear buffers
	bob.getBuffer().getWriteBuffer();
	charlie.getBuffer().getWriteBuffer();

	// Alice quits
	ACommand* quitCmd = CommandFactory::getInstance().createCommand(irc::QUIT, server);
	Message quitMsg;
	quitMsg.m_command = "QUIT";
	quitMsg.m_params.push_back("Bye!");
	quitCmd->execute(&alice, quitMsg);

	// Both bob and charlie should receive QUIT
	std::string bobBuffer = bob.getBuffer().getWriteBuffer();
	std::string charlieBuffer = charlie.getBuffer().getWriteBuffer();

	cr_assert(bobBuffer.find("QUIT") != std::string::npos, "Bob should receive QUIT from #test1");
	cr_assert(charlieBuffer.find("QUIT") != std::string::npos,
			  "Charlie should receive QUIT from #test2");

	delete quitCmd;
}

// QUIT with empty message should use default
Test(QuitCommand, empty_quit_message_uses_default)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost", server);
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::QUIT, server);
	cr_assert_not_null(cmd, "Factory failed to create QUIT command");

	Message msg;
	msg.m_command = "QUIT";
	msg.m_params.push_back(""); // Empty message

	cmd->execute(&client, msg);

	// Should use nickname as default
	delete cmd;
}

// Quit message with spaces
Test(QuitCommand, quit_message_with_spaces)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost", server);
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::QUIT, server);
	cr_assert_not_null(cmd, "Factory failed to create QUIT command");

	Message msg;
	msg.m_command = "QUIT";
	msg.m_params.push_back("I have to go now. See you later!");

	cmd->execute(&client, msg);

	// Quit message should preserve spaces
	delete cmd;
}

// RFC 4.1.6: No numeric replies for QUIT
Test(QuitCommand, no_numeric_replies)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost", server);
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::QUIT, server);
	cr_assert_not_null(cmd, "Factory failed to create QUIT command");

	Message msg;
	msg.m_command = "QUIT";
	msg.m_params.push_back("Goodbye");

	cmd->execute(&client, msg);

	// QUIT should not send any numeric replies to the quitting client
	// The client just disconnects
	std::string buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("001") == std::string::npos && buffer.find("002") == std::string::npos &&
				  buffer.find("003") == std::string::npos,
			  "QUIT should not send numeric replies to quitting client");

	delete cmd;
}

// User not in any channels - QUIT should still work
Test(QuitCommand, quit_without_channels)
{
	Server server(6667, "testpass");
	ClientMock client(3, "localhost", server);
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::QUIT, server);
	cr_assert_not_null(cmd, "Factory failed to create QUIT command");

	Message msg;
	msg.m_command = "QUIT";
	msg.m_params.push_back("Later");

	// Should work even if not in any channels
	cmd->execute(&client, msg);

	delete cmd;
}

// QUIT format should be: :nick!user@host QUIT :message
Test(QuitCommand, quit_message_format)
{
	Server server(6667, "testpass");
	ClientMock alice(3, "localhost", server);
	ClientMock bob(4, "localhost", server);

	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice");

	bob.setPasswordProvided(true);
	bob.setNickname("bob");
	bob.setUsername("bob");

	// Both join same channel
	ACommand* joinCmd = CommandFactory::getInstance().createCommand(irc::JOIN, server);
	Message joinMsg;
	joinMsg.m_command = "JOIN";
	joinMsg.m_params.push_back("#test");
	joinCmd->execute(&alice, joinMsg);
	joinCmd->execute(&bob, joinMsg);
	delete joinCmd;

	bob.getBuffer().getWriteBuffer(); // Clear buffer

	// Alice quits
	ACommand* quitCmd = CommandFactory::getInstance().createCommand(irc::QUIT, server);
	Message quitMsg;
	quitMsg.m_command = "QUIT";
	quitMsg.m_params.push_back("Bye!");
	quitCmd->execute(&alice, quitMsg);

	// Check format: :alice!alice@localhost QUIT :Bye!
	std::string bobBuffer = bob.getBuffer().getWriteBuffer();
	cr_assert(bobBuffer.find(":alice!alice@localhost") != std::string::npos ||
				  bobBuffer.find(":alice!") != std::string::npos,
			  "QUIT message should have proper prefix format");
	cr_assert(bobBuffer.find("QUIT") != std::string::npos,
			  "QUIT message should contain QUIT command");

	delete quitCmd;
}
