#include "IChannel.hpp"
#include "commands/ACommand.hpp"
#include "commands/CommandFactory.hpp"
#include "commands/PartCommand.hpp"
#include "mocks/Channel.hpp"
#include "mocks/Client.hpp"
#include "mocks/Server.hpp"
#include "protocol/Message.hpp"
#include <criterion/criterion.h>

Test(PartCommand, requires_registration)
{
	Server server(6667, "test123");
	ClientMock client(3, "localhost", server);
	// Client is NOT registered (no password)
	client.setNickname("unregistered");
	client.setUsername("unregistered");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::PART, server);
	cr_assert_not_null(cmd, "Factory failed to create PART command. Is it registered?");
	Message msg;
	msg.m_command = "PART";
	msg.m_params.push_back("#test");

	cmd->execute(&client, msg);

	// Should receive ERR_NOTREGISTERED (451)
	const std::string& buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("451") != std::string::npos, "Expected ERR_NOTREGISTERED (451)");
	cr_assert(buffer.find("You have not registered") != std::string::npos);
	delete cmd;
}

Test(PartCommand, successful_part_single_channel)
{
	Server server(6667, "test123");
	ClientMock client(3, "localhost", server);
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	// Join channel first
	IChannel* channel = server.createChannel("#test", &client);
	client.joinChannel(channel);

	cr_assert(client.isInChannel("#test"));
	cr_assert_eq(channel->getMemberCount(), 1);

	// Now PART
	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::PART, server);
	cr_assert_not_null(cmd, "Factory failed to create PART command. Is it registered?");
	Message msg;
	msg.m_command = "PART";
	msg.m_params.push_back("#test");

	cmd->execute(&client, msg);

	// Verify client left and channel was deleted (empty)
	cr_assert_not(client.isInChannel("#test"));
	cr_assert_null(server.getChannel("#test"));
	cr_assert_eq(server.getChannelCount(), 0);
	delete cmd;
}

Test(PartCommand, part_with_message)
{
	Server server(6667, "test123");
	ClientMock client(3, "localhost", server);
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	IChannel* channel = server.createChannel("#test", &client);
	client.joinChannel(channel);

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::PART, server);
	cr_assert_not_null(cmd, "Factory failed to create PART command. Is it registered?");
	Message msg;
	msg.m_command = "PART";
	msg.m_params.push_back("#test");
	msg.m_params.push_back("Going to lunch, BRB!");

	cmd->execute(&client, msg);

	// Check that PART message was broadcast
	const std::string& buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("PART") != std::string::npos);
	cr_assert(buffer.find("#test") != std::string::npos);
	cr_assert(buffer.find("Going to lunch, BRB!") != std::string::npos);

	cr_assert_not(client.isInChannel("#test"));
	delete cmd;
}

Test(PartCommand, part_nonexistent_channel)
{
	Server server(6667, "test123");
	ClientMock client(3, "localhost", server);
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::PART, server);
	cr_assert_not_null(cmd, "Factory failed to create PART command. Is it registered?");
	Message msg;
	msg.m_command = "PART";
	msg.m_params.push_back("#nonexistent");

	cmd->execute(&client, msg);

	// Check ERR_NOSUCHCHANNEL (403) was sent
	const std::string& buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("403") != std::string::npos);
	cr_assert(buffer.find("#nonexistent") != std::string::npos);
	cr_assert(buffer.find("No such channel") != std::string::npos);
	delete cmd;
}

Test(PartCommand, part_channel_not_on)
{
	Server server(6667, "test123");
	ClientMock alice(3, "localhost", server);
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice");

	ClientMock bob(4, "localhost", server);
	bob.setPasswordProvided(true);
	bob.setNickname("bob");
	bob.setUsername("bob");

	// Bob creates and joins #test
	IChannel* channel = server.createChannel("#test", &bob);
	bob.joinChannel(channel);

	// Alice tries to leave it (but she's not in it)
	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::PART, server);
	cr_assert_not_null(cmd, "Factory failed to create PART command. Is it registered?");
	Message msg;
	msg.m_command = "PART";
	msg.m_params.push_back("#test");

	cmd->execute(&alice, msg);

	// Check ERR_NOTONCHANNEL (442) was sent
	const std::string& buffer = alice.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("442") != std::string::npos);
	cr_assert(buffer.find("#test") != std::string::npos);
	cr_assert(buffer.find("not on that channel") != std::string::npos);
	delete cmd;
}

Test(PartCommand, part_no_parameters)
{
	Server server(6667, "test123");
	ClientMock client(3, "localhost", server);
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::PART, server);
	cr_assert_not_null(cmd, "Factory failed to create PART command. Is it registered?");
	Message msg;
	msg.m_command = "PART";
	// No parameters!

	cmd->execute(&client, msg);

	// Check ERR_NEEDMOREPARAMS (461) was sent
	const std::string& buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("461") != std::string::npos);
	cr_assert(buffer.find("PART") != std::string::npos);
	cr_assert(buffer.find("Not enough parameters") != std::string::npos);
	delete cmd;
}

Test(PartCommand, part_broadcasts_to_all_members)
{
	Server server(6667, "test123");
	ClientMock alice(3, "localhost", server);
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice");

	ClientMock bob(4, "localhost", server);
	bob.setPasswordProvided(true);
	bob.setNickname("bob");
	bob.setUsername("bob");

	ClientMock charlie(5, "localhost", server);
	charlie.setPasswordProvided(true);
	charlie.setNickname("charlie");
	charlie.setUsername("charlie");

	// All join #test
	IChannel* channel = server.createChannel("#test", &alice);
	alice.joinChannel(channel);
	channel->addMember(&bob);
	bob.joinChannel(channel);
	channel->addMember(&charlie);
	charlie.joinChannel(channel);

	cr_assert_eq(channel->getMemberCount(), 3);

	// Alice leaves
	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::PART, server);
	cr_assert_not_null(cmd, "Factory failed to create PART command. Is it registered?");
	Message msg;
	msg.m_command = "PART";
	msg.m_params.push_back("#test");

	cmd->execute(&alice, msg);

	// Alice should have received PART message
	const std::string& alice_buffer = alice.getBuffer().getWriteBuffer();
	cr_assert(alice_buffer.find("PART") != std::string::npos);
	cr_assert(alice_buffer.find("#test") != std::string::npos);

	// Bob should have received PART message
	const std::string& bob_buffer = bob.getBuffer().getWriteBuffer();
	cr_assert(bob_buffer.find("PART") != std::string::npos);

	// Charlie should have received PART message
	const std::string& charlie_buffer = charlie.getBuffer().getWriteBuffer();
	cr_assert(charlie_buffer.find("PART") != std::string::npos);

	// Verify alice left but channel still exists
	cr_assert_not(alice.isInChannel("#test"));
	cr_assert_eq(channel->getMemberCount(), 2);
	cr_assert_not_null(server.getChannel("#test"));
	delete cmd;
}

Test(PartCommand, last_member_parts_channel_deleted)
{
	Server server(6667, "test123");
	ClientMock alice(3, "localhost", server);
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice");

	ClientMock bob(4, "localhost", server);
	bob.setPasswordProvided(true);
	bob.setNickname("bob");
	bob.setUsername("bob");

	// Both join
	IChannel* channel = server.createChannel("#test", &alice);
	alice.joinChannel(channel);
	channel->addMember(&bob);
	bob.joinChannel(channel);

	cr_assert_eq(channel->getMemberCount(), 2);
	cr_assert_not_null(server.getChannel("#test"));

	// Alice leaves
	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::PART, server);
	cr_assert_not_null(cmd, "Factory failed to create PART command. Is it registered?");
	Message msg1;
	msg1.m_command = "PART";
	msg1.m_params.push_back("#test");
	cmd->execute(&alice, msg1);

	// Channel still exists (bob is still there)
	cr_assert_not_null(server.getChannel("#test"));
	cr_assert_eq(server.getChannelCount(), 1);

	// Bob leaves (last member)
	Message msg2;
	msg2.m_command = "PART";
	msg2.m_params.push_back("#test");
	cmd->execute(&bob, msg2);

	// Channel should be deleted
	cr_assert_null(server.getChannel("#test"));
	cr_assert_eq(server.getChannelCount(), 0);
	delete cmd;
}

Test(PartCommand, part_removes_operator_status)
{
	Server server(6667, "test123");
	ClientMock alice(3, "localhost", server);
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice");

	ClientMock bob(4, "localhost", server);
	bob.setPasswordProvided(true);
	bob.setNickname("bob");
	bob.setUsername("bob");

	// Alice creates channel (becomes operator)
	IChannel* channel = server.createChannel("#test", &alice);
	alice.joinChannel(channel);
	channel->addMember(&bob);
	bob.joinChannel(channel);

	cr_assert(channel->isOperator(&alice));
	cr_assert_not(channel->isOperator(&bob));

	// Alice leaves
	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::PART, server);
	cr_assert_not_null(cmd, "Factory failed to create PART command. Is it registered?");
	Message msg;
	msg.m_command = "PART";
	msg.m_params.push_back("#test");

	cmd->execute(&alice, msg);

	// Alice should no longer be operator (she's not even a member)
	cr_assert_not(channel->hasMember(&alice));
	cr_assert_not(channel->isOperator(&alice));
	delete cmd;
}

Test(PartCommand, part_with_prefix_in_broadcast)
{
	Server server(6667, "test123");
	ClientMock alice(3, "localhost", server);
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice_user");

	IChannel* channel = server.createChannel("#test", &alice);
	alice.joinChannel(channel);

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::PART, server);
	cr_assert_not_null(cmd, "Factory failed to create PART command. Is it registered?");
	Message msg;
	msg.m_command = "PART";
	msg.m_params.push_back("#test");

	cmd->execute(&alice, msg);

	// Check broadcast includes proper prefix (nick!user@host)
	const std::string& buffer = alice.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("alice!alice_user@localhost") != std::string::npos);
	cr_assert(buffer.find("PART") != std::string::npos);
	cr_assert(buffer.find("#test") != std::string::npos);
	delete cmd;
}

Test(PartCommand, part_ampersand_channel)
{
	Server server(6667, "test123");
	ClientMock client(3, "localhost", server);
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	// Join local channel with & prefix
	IChannel* channel = server.createChannel("&local", &client);
	client.joinChannel(channel);

	cr_assert(client.isInChannel("&local"));

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::PART, server);
	cr_assert_not_null(cmd, "Factory failed to create PART command. Is it registered?");
	Message msg;
	msg.m_command = "PART";
	msg.m_params.push_back("&local");

	cmd->execute(&client, msg);

	cr_assert_not(client.isInChannel("&local"));
	cr_assert_null(server.getChannel("&local"));
	delete cmd;
}

Test(PartCommand, part_empty_message_parameter)
{
	Server server(6667, "test123");
	ClientMock client(3, "localhost", server);
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	IChannel* channel = server.createChannel("#test", &client);
	client.joinChannel(channel);

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::PART, server);
	cr_assert_not_null(cmd, "Factory failed to create PART command. Is it registered?");
	Message msg;
	msg.m_command = "PART";
	msg.m_params.push_back("#test");
	msg.m_params.push_back(""); // Empty part message

	cmd->execute(&client, msg);

	// Should work fine, just no message
	cr_assert_not(client.isInChannel("#test"));
	delete cmd;
}

Test(PartCommand, part_channel_name_case_sensitive)
{
	Server server(6667, "test123");
	ClientMock client(3, "localhost", server);
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	// Create #Test (capital T)
	IChannel* channel = server.createChannel("#Test", &client);
	client.joinChannel(channel);

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::PART, server);
	cr_assert_not_null(cmd, "Factory failed to create PART command. Is it registered?");
	Message msg;
	msg.m_command = "PART";
	msg.m_params.push_back("#Test"); // Must match case

	cmd->execute(&client, msg);

	cr_assert_not(client.isInChannel("#Test"));
	delete cmd;
}

Test(PartCommand, operator_parts_doesnt_transfer_ops)
{
	Server server(6667, "test123");
	ClientMock alice(3, "localhost", server);
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice");

	ClientMock bob(4, "localhost", server);
	bob.setPasswordProvided(true);
	bob.setNickname("bob");
	bob.setUsername("bob");

	// Alice creates channel (becomes operator), bob joins
	IChannel* channel = server.createChannel("#test", &alice);
	alice.joinChannel(channel);
	channel->addMember(&bob);
	bob.joinChannel(channel);

	cr_assert(channel->isOperator(&alice));
	cr_assert_not(channel->isOperator(&bob));

	// Alice parts
	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::PART, server);
	cr_assert_not_null(cmd, "Factory failed to create PART command. Is it registered?");
	Message msg;
	msg.m_command = "PART";
	msg.m_params.push_back("#test");

	cmd->execute(&alice, msg);

	// Bob should NOT automatically become operator
	// (This is RFC 1459 behavior - ops don't transfer on PART)
	cr_assert_not(channel->isOperator(&bob));
	cr_assert_eq(channel->getMemberCount(), 1);
	delete cmd;
}

Test(PartCommand, multiple_channels_in_client_list)
{
	Server server(6667, "test123");
	ClientMock client(3, "localhost", server);
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	// Join multiple channels
	IChannel* channel1 = server.createChannel("#test1", &client);
	client.joinChannel(channel1);
	IChannel* channel2 = server.createChannel("#test2", &client);
	client.joinChannel(channel2);
	IChannel* channel3 = server.createChannel("#test3", &client);
	client.joinChannel(channel3);

	cr_assert(client.isInChannel("#test1"));
	cr_assert(client.isInChannel("#test2"));
	cr_assert(client.isInChannel("#test3"));

	// Part one channel
	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::PART, server);
	cr_assert_not_null(cmd, "Factory failed to create PART command. Is it registered?");
	Message msg;
	msg.m_command = "PART";
	msg.m_params.push_back("#test2");

	cmd->execute(&client, msg);

	// Should only leave #test2
	cr_assert(client.isInChannel("#test1"));
	cr_assert_not(client.isInChannel("#test2"));
	cr_assert(client.isInChannel("#test3"));
	delete cmd;
}
