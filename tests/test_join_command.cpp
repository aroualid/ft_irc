#include "commands/CommandFactory.hpp"
#include "commands/JoinCommand.hpp"
#include "mocks/Channel.hpp"
#include "mocks/Client.hpp"
#include "mocks/Server.hpp"
#include "protocol/Message.hpp"
#include <criterion/criterion.h>

Test(JoinCommand, requires_registration)
{
	Server server(6667, "test123");
	ClientMock client(3, "localhost", server);
	// ClientMock is NOT registered (no password, no username)
	client.setNickname("unregistered");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::JOIN, server);
	cr_assert_not_null(cmd, "Factory failed to create JOIN command. Is it registered?");
	Message msg;
	msg.m_command = "JOIN";
	msg.m_params.push_back("#test");

	cmd->execute(&client, msg);

	// Should receive ERR_NOTREGISTERED (451)
	const std::string& buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("451") != std::string::npos, "Expected ERR_NOTREGISTERED (451)");
	cr_assert(buffer.find("You have not registered") != std::string::npos);

	// ClientMock should NOT have joined the channel
	cr_assert_not(client.isInChannel("#test"));
	cr_assert_eq(server.getChannelCount(), 0);
	delete cmd;
}

Test(JoinCommand, requires_registration_no_password)
{
	Server server(6667, "test123");
	ClientMock client(3, "localhost", server);
	// Has nickname and username but NOT password
	client.setNickname("alice");
	client.setUsername("alice");
	// client.setPasswordProvided(true); // intentionally not set

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::JOIN, server);
	cr_assert_not_null(cmd, "Factory failed to create JOIN command. Is it registered?");
	Message msg;
	msg.m_command = "JOIN";
	msg.m_params.push_back("#test");

	cmd->execute(&client, msg);

	// Should receive ERR_NOTREGISTERED (451)
	const std::string& buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("451") != std::string::npos, "Expected ERR_NOTREGISTERED (451)");

	// ClientMock should NOT have joined the channel
	cr_assert_not(client.isInChannel("#test"));
	cr_assert_eq(server.getChannelCount(), 0);
	delete cmd;
}

Test(JoinCommand, single_channel_join)
{
	Server server(6667, "test123");
	ClientMock client(3, "localhost", server);
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::JOIN, server);
	cr_assert_not_null(cmd, "Factory failed to create JOIN command. Is it registered?");
	Message msg;
	msg.m_command = "JOIN";
	msg.m_params.push_back("#test");

	cmd->execute(&client, msg);

	cr_assert(client.isInChannel("#test"));
	cr_assert_eq(server.getChannelCount(), 1);

	IChannel* channel = server.getChannel("#test");
	cr_assert_not_null(channel);
	cr_assert_eq(channel->getMemberCount(), 1);
	cr_assert(channel->hasMember(&client));
	cr_assert(channel->isOperator(&client));
	delete cmd;
}

Test(JoinCommand, multiple_channels_join)
{
	Server server(6667, "test123");
	ClientMock client(3, "localhost", server);
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::JOIN, server);
	cr_assert_not_null(cmd, "Factory failed to create JOIN command. Is it registered?");
	Message msg;
	msg.m_command = "JOIN";
	msg.m_params.push_back("#test,#foo,#bar");

	cmd->execute(&client, msg);

	cr_assert(client.isInChannel("#test"));
	cr_assert(client.isInChannel("#foo"));
	cr_assert(client.isInChannel("#bar"));
	cr_assert_eq(server.getChannelCount(), 3);
	delete cmd;
}

Test(JoinCommand, channel_with_key)
{
	Server server(6667, "test123");
	ClientMock client1(3, "localhost", server);
	client1.setPasswordProvided(true);
	client1.setNickname("alice");
	client1.setUsername("alice");

	ClientMock client2(4, "localhost", server);
	client2.setPasswordProvided(true);
	client2.setNickname("bob");
	client2.setUsername("bob");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::JOIN, server);
	cr_assert_not_null(cmd, "Factory failed to create JOIN command. Is it registered?");
	Message msg1;
	msg1.m_command = "JOIN";
	msg1.m_params.push_back("#secret");
	cmd->execute(&client1, msg1);

	IChannel* channel = server.getChannel("#secret");
	cr_assert_not_null(channel);
	channel->setKey("password123");

	Message msg2;
	msg2.m_command = "JOIN";
	msg2.m_params.push_back("#secret");
	msg2.m_params.push_back("password123");

	cmd->execute(&client2, msg2);

	cr_assert(client2.isInChannel("#secret"));
	cr_assert_eq(channel->getMemberCount(), 2);
	delete cmd;
}

Test(JoinCommand, channel_with_wrong_key)
{
	Server server(6667, "test123");
	ClientMock client1(3, "localhost", server);
	client1.setPasswordProvided(true);
	client1.setNickname("alice");
	client1.setUsername("alice");

	ClientMock client2(4, "localhost", server);
	client2.setPasswordProvided(true);
	client2.setNickname("bob");
	client2.setUsername("bob");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::JOIN, server);
	cr_assert_not_null(cmd, "Factory failed to create JOIN command. Is it registered?");
	Message msg1;
	msg1.m_command = "JOIN";
	msg1.m_params.push_back("#secret");
	cmd->execute(&client1, msg1);

	IChannel* channel = server.getChannel("#secret");
	channel->setKey("password123");

	Message msg2;
	msg2.m_command = "JOIN";
	msg2.m_params.push_back("#secret");
	msg2.m_params.push_back("wrongpass");

	cmd->execute(&client2, msg2);

	cr_assert_not(client2.isInChannel("#secret"));
	cr_assert_eq(channel->getMemberCount(), 1);
	delete cmd;
}

Test(JoinCommand, multiple_channels_with_keys)
{
	Server server(6667, "test123");
	ClientMock client(3, "localhost", server);
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::JOIN, server);
	cr_assert_not_null(cmd, "Factory failed to create JOIN command. Is it registered?");
	Message msg;
	msg.m_command = "JOIN";
	msg.m_params.push_back("#test,#foo");
	msg.m_params.push_back("key1,key2");

	cmd->execute(&client, msg);

	cr_assert(client.isInChannel("#test"));
	cr_assert(client.isInChannel("#foo"));
	cr_assert_eq(server.getChannelCount(), 2);
	delete cmd;
}

Test(JoinCommand, invalid_channel_name_no_prefix)
{
	Server server(6667, "test123");
	ClientMock client(3, "localhost", server);
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::JOIN, server);
	cr_assert_not_null(cmd, "Factory failed to create JOIN command. Is it registered?");
	Message msg;
	msg.m_command = "JOIN";
	msg.m_params.push_back("invalid");

	cmd->execute(&client, msg);

	cr_assert_not(client.isInChannel("invalid"));
	cr_assert_eq(server.getChannelCount(), 0);
	delete cmd;
}

Test(JoinCommand, invalid_channel_name_with_space)
{
	Server server(6667, "test123");
	ClientMock client(3, "localhost", server);
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::JOIN, server);
	cr_assert_not_null(cmd, "Factory failed to create JOIN command. Is it registered?");
	Message msg;
	msg.m_command = "JOIN";
	msg.m_params.push_back("#test channel");

	cmd->execute(&client, msg);

	cr_assert_not(client.isInChannel("#test channel"));
	cr_assert_eq(server.getChannelCount(), 0);
	delete cmd;
}

Test(JoinCommand, invalid_channel_name_too_long)
{
	Server server(6667, "test123");
	ClientMock client(3, "localhost", server);
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::JOIN, server);
	cr_assert_not_null(cmd, "Factory failed to create JOIN command. Is it registered?");
	Message msg;
	msg.m_command = "JOIN";
	std::string longName = "#";
	for (int i = 0; i < 250; i++)
		longName += "a";
	msg.m_params.push_back(longName);

	cmd->execute(&client, msg);

	cr_assert_not(client.isInChannel(longName));
	cr_assert_eq(server.getChannelCount(), 0);
	delete cmd;
}

Test(JoinCommand, join_already_in_channel)
{
	Server server(6667, "test123");
	ClientMock client(3, "localhost", server);
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::JOIN, server);
	cr_assert_not_null(cmd, "Factory failed to create JOIN command. Is it registered?");
	Message msg;
	msg.m_command = "JOIN";
	msg.m_params.push_back("#test");

	cmd->execute(&client, msg);
	cmd->execute(&client, msg);

	IChannel* channel = server.getChannel("#test");
	cr_assert_eq(channel->getMemberCount(), 1);
	delete cmd;
}

Test(JoinCommand, invite_only_channel_no_invite)
{
	Server server(6667, "test123");
	ClientMock client1(3, "localhost", server);
	client1.setPasswordProvided(true);
	client1.setNickname("alice");
	client1.setUsername("alice");

	ClientMock client2(4, "localhost", server);
	client2.setPasswordProvided(true);
	client2.setNickname("bob");
	client2.setUsername("bob");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::JOIN, server);
	cr_assert_not_null(cmd, "Factory failed to create JOIN command. Is it registered?");
	Message msg1;
	msg1.m_command = "JOIN";
	msg1.m_params.push_back("#private");
	cmd->execute(&client1, msg1);

	IChannel* channel = server.getChannel("#private");
	channel->setInviteOnly(true);

	Message msg2;
	msg2.m_command = "JOIN";
	msg2.m_params.push_back("#private");
	cmd->execute(&client2, msg2);

	cr_assert_not(client2.isInChannel("#private"));
	cr_assert_eq(channel->getMemberCount(), 1);
	delete cmd;
}

Test(JoinCommand, invite_only_channel_with_invite)
{
	Server server(6667, "test123");
	ClientMock client1(3, "localhost", server);
	client1.setPasswordProvided(true);
	client1.setNickname("alice");
	client1.setUsername("alice");

	ClientMock client2(4, "localhost", server);
	client2.setPasswordProvided(true);
	client2.setNickname("bob");
	client2.setUsername("bob");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::JOIN, server);
	cr_assert_not_null(cmd, "Factory failed to create JOIN command. Is it registered?");
	Message msg1;
	msg1.m_command = "JOIN";
	msg1.m_params.push_back("#private");
	cmd->execute(&client1, msg1);

	IChannel* channel = server.getChannel("#private");
	channel->setInviteOnly(true);
	channel->addInvite(&client2);

	Message msg2;
	msg2.m_command = "JOIN";
	msg2.m_params.push_back("#private");
	cmd->execute(&client2, msg2);

	cr_assert(client2.isInChannel("#private"));
	cr_assert_eq(channel->getMemberCount(), 2);
	delete cmd;
}

Test(JoinCommand, channel_full)
{
	Server server(6667, "test123");
	ClientMock client1(3, "localhost", server);
	client1.setPasswordProvided(true);
	client1.setNickname("alice");
	client1.setUsername("alice");

	ClientMock client2(4, "localhost", server);
	client2.setPasswordProvided(true);
	client2.setNickname("bob");
	client2.setUsername("bob");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::JOIN, server);
	cr_assert_not_null(cmd, "Factory failed to create JOIN command. Is it registered?");
	Message msg1;
	msg1.m_command = "JOIN";
	msg1.m_params.push_back("#limited");
	cmd->execute(&client1, msg1);

	IChannel* channel = server.getChannel("#limited");
	channel->setUserLimit(1);

	Message msg2;
	msg2.m_command = "JOIN";
	msg2.m_params.push_back("#limited");
	cmd->execute(&client2, msg2);

	cr_assert_not(client2.isInChannel("#limited"));
	cr_assert_eq(channel->getMemberCount(), 1);
	delete cmd;
}

Test(JoinCommand, first_member_becomes_operator)
{
	Server server(6667, "test123");
	ClientMock client(3, "localhost", server);
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::JOIN, server);
	cr_assert_not_null(cmd, "Factory failed to create JOIN command. Is it registered?");
	Message msg;
	msg.m_command = "JOIN";
	msg.m_params.push_back("#new");

	cmd->execute(&client, msg);

	IChannel* channel = server.getChannel("#new");
	cr_assert(channel->isOperator(&client));
	delete cmd;
}

Test(JoinCommand, ampersand_channel_prefix)
{
	Server server(6667, "test123");
	ClientMock client(3, "localhost", server);
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::JOIN, server);
	cr_assert_not_null(cmd, "Factory failed to create JOIN command. Is it registered?");
	Message msg;
	msg.m_command = "JOIN";
	msg.m_params.push_back("&local");

	cmd->execute(&client, msg);

	cr_assert(client.isInChannel("&local"));
	IChannel* channel = server.getChannel("&local");
	cr_assert_not_null(channel);
	delete cmd;
}

Test(JoinCommand, empty_channel_name)
{
	Server server(6667, "test123");
	ClientMock client(3, "localhost", server);
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::JOIN, server);
	cr_assert_not_null(cmd, "Factory failed to create JOIN command. Is it registered?");
	Message msg;
	msg.m_command = "JOIN";
	msg.m_params.push_back("");

	cmd->execute(&client, msg);

	cr_assert_eq(server.getChannelCount(), 0);
	delete cmd;
}

Test(JoinCommand, mixed_valid_invalid_channels)
{
	Server server(6667, "test123");
	ClientMock client(3, "localhost", server);
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::JOIN, server);
	cr_assert_not_null(cmd, "Factory failed to create JOIN command. Is it registered?");
	Message msg;
	msg.m_command = "JOIN";
	msg.m_params.push_back("#valid,invalid,#alsoValid");

	cmd->execute(&client, msg);

	cr_assert(client.isInChannel("#valid"));
	cr_assert_not(client.isInChannel("invalid"));
	cr_assert(client.isInChannel("#alsoValid"));
	cr_assert_eq(server.getChannelCount(), 2);
	delete cmd;
}
