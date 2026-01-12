#include <criterion/criterion.h>
#include "mocks/Server.hpp"
#include "mocks/Client.hpp"
#include "mocks/Channel.hpp"
#include "commands/JoinCommand.hpp"
#include "protocol/Message.hpp"

Test(JoinCommand, single_channel_join)
{
	Server server(6667, "test123");
	Client client(3, "localhost");
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	JoinCommand cmd(server);
	Message msg;
	msg.m_command = "JOIN";
	msg.m_params.push_back("#test");

	cmd.execute(&client, msg);

	cr_assert(client.isInChannel("#test"));
	cr_assert_eq(server.getChannelCount(), 1);

	IChannel* channel = server.getChannel("#test");
	cr_assert_not_null(channel);
	cr_assert_eq(channel->getMemberCount(), 1);
	cr_assert(channel->hasMember(&client));
	cr_assert(channel->isOperator(&client));
}

Test(JoinCommand, multiple_channels_join)
{
	Server server(6667, "test123");
	Client client(3, "localhost");
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	JoinCommand cmd(server);
	Message msg;
	msg.m_command = "JOIN";
	msg.m_params.push_back("#test,#foo,#bar");

	cmd.execute(&client, msg);

	cr_assert(client.isInChannel("#test"));
	cr_assert(client.isInChannel("#foo"));
	cr_assert(client.isInChannel("#bar"));
	cr_assert_eq(server.getChannelCount(), 3);
}

Test(JoinCommand, channel_with_key)
{
	Server server(6667, "test123");
	Client client1(3, "localhost");
	client1.setPasswordProvided(true);
	client1.setNickname("alice");
	client1.setUsername("alice");

	Client client2(4, "localhost");
	client2.setPasswordProvided(true);
	client2.setNickname("bob");
	client2.setUsername("bob");

	JoinCommand cmd(server);
	Message msg1;
	msg1.m_command = "JOIN";
	msg1.m_params.push_back("#secret");
	cmd.execute(&client1, msg1);

	IChannel* channel = server.getChannel("#secret");
	cr_assert_not_null(channel);
	channel->setKey("password123");

	Message msg2;
	msg2.m_command = "JOIN";
	msg2.m_params.push_back("#secret");
	msg2.m_params.push_back("password123");

	cmd.execute(&client2, msg2);

	cr_assert(client2.isInChannel("#secret"));
	cr_assert_eq(channel->getMemberCount(), 2);
}

Test(JoinCommand, channel_with_wrong_key)
{
	Server server(6667, "test123");
	Client client1(3, "localhost");
	client1.setPasswordProvided(true);
	client1.setNickname("alice");
	client1.setUsername("alice");

	Client client2(4, "localhost");
	client2.setPasswordProvided(true);
	client2.setNickname("bob");
	client2.setUsername("bob");

	JoinCommand cmd(server);
	Message msg1;
	msg1.m_command = "JOIN";
	msg1.m_params.push_back("#secret");
	cmd.execute(&client1, msg1);

	IChannel* channel = server.getChannel("#secret");
	channel->setKey("password123");

	Message msg2;
	msg2.m_command = "JOIN";
	msg2.m_params.push_back("#secret");
	msg2.m_params.push_back("wrongpass");

	cmd.execute(&client2, msg2);

	cr_assert_not(client2.isInChannel("#secret"));
	cr_assert_eq(channel->getMemberCount(), 1);
}

Test(JoinCommand, multiple_channels_with_keys)
{
	Server server(6667, "test123");
	Client client(3, "localhost");
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	JoinCommand cmd(server);
	Message msg;
	msg.m_command = "JOIN";
	msg.m_params.push_back("#test,#foo");
	msg.m_params.push_back("key1,key2");

	cmd.execute(&client, msg);

	cr_assert(client.isInChannel("#test"));
	cr_assert(client.isInChannel("#foo"));
	cr_assert_eq(server.getChannelCount(), 2);
}

Test(JoinCommand, invalid_channel_name_no_prefix)
{
	Server server(6667, "test123");
	Client client(3, "localhost");
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	JoinCommand cmd(server);
	Message msg;
	msg.m_command = "JOIN";
	msg.m_params.push_back("invalid");

	cmd.execute(&client, msg);

	cr_assert_not(client.isInChannel("invalid"));
	cr_assert_eq(server.getChannelCount(), 0);
}

Test(JoinCommand, invalid_channel_name_with_space)
{
	Server server(6667, "test123");
	Client client(3, "localhost");
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	JoinCommand cmd(server);
	Message msg;
	msg.m_command = "JOIN";
	msg.m_params.push_back("#test channel");

	cmd.execute(&client, msg);

	cr_assert_not(client.isInChannel("#test channel"));
	cr_assert_eq(server.getChannelCount(), 0);
}

Test(JoinCommand, invalid_channel_name_too_long)
{
	Server server(6667, "test123");
	Client client(3, "localhost");
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	JoinCommand cmd(server);
	Message msg;
	msg.m_command = "JOIN";
	std::string longName = "#";
	for (int i = 0; i < 250; i++)
		longName += "a";
	msg.m_params.push_back(longName);

	cmd.execute(&client, msg);

	cr_assert_not(client.isInChannel(longName));
	cr_assert_eq(server.getChannelCount(), 0);
}

Test(JoinCommand, join_already_in_channel)
{
	Server server(6667, "test123");
	Client client(3, "localhost");
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	JoinCommand cmd(server);
	Message msg;
	msg.m_command = "JOIN";
	msg.m_params.push_back("#test");

	cmd.execute(&client, msg);
	cmd.execute(&client, msg);

	IChannel* channel = server.getChannel("#test");
	cr_assert_eq(channel->getMemberCount(), 1);
}

Test(JoinCommand, invite_only_channel_no_invite)
{
	Server server(6667, "test123");
	Client client1(3, "localhost");
	client1.setPasswordProvided(true);
	client1.setNickname("alice");
	client1.setUsername("alice");

	Client client2(4, "localhost");
	client2.setPasswordProvided(true);
	client2.setNickname("bob");
	client2.setUsername("bob");

	JoinCommand cmd(server);
	Message msg1;
	msg1.m_command = "JOIN";
	msg1.m_params.push_back("#private");
	cmd.execute(&client1, msg1);

	IChannel* channel = server.getChannel("#private");
	channel->setInviteOnly(true);

	Message msg2;
	msg2.m_command = "JOIN";
	msg2.m_params.push_back("#private");
	cmd.execute(&client2, msg2);

	cr_assert_not(client2.isInChannel("#private"));
	cr_assert_eq(channel->getMemberCount(), 1);
}

Test(JoinCommand, invite_only_channel_with_invite)
{
	Server server(6667, "test123");
	Client client1(3, "localhost");
	client1.setPasswordProvided(true);
	client1.setNickname("alice");
	client1.setUsername("alice");

	Client client2(4, "localhost");
	client2.setPasswordProvided(true);
	client2.setNickname("bob");
	client2.setUsername("bob");

	JoinCommand cmd(server);
	Message msg1;
	msg1.m_command = "JOIN";
	msg1.m_params.push_back("#private");
	cmd.execute(&client1, msg1);

	IChannel* channel = server.getChannel("#private");
	channel->setInviteOnly(true);
	channel->addInvite(&client2);

	Message msg2;
	msg2.m_command = "JOIN";
	msg2.m_params.push_back("#private");
	cmd.execute(&client2, msg2);

	cr_assert(client2.isInChannel("#private"));
	cr_assert_eq(channel->getMemberCount(), 2);
}

Test(JoinCommand, channel_full)
{
	Server server(6667, "test123");
	Client client1(3, "localhost");
	client1.setPasswordProvided(true);
	client1.setNickname("alice");
	client1.setUsername("alice");

	Client client2(4, "localhost");
	client2.setPasswordProvided(true);
	client2.setNickname("bob");
	client2.setUsername("bob");

	JoinCommand cmd(server);
	Message msg1;
	msg1.m_command = "JOIN";
	msg1.m_params.push_back("#limited");
	cmd.execute(&client1, msg1);

	IChannel* channel = server.getChannel("#limited");
	channel->setUserLimit(1);

	Message msg2;
	msg2.m_command = "JOIN";
	msg2.m_params.push_back("#limited");
	cmd.execute(&client2, msg2);

	cr_assert_not(client2.isInChannel("#limited"));
	cr_assert_eq(channel->getMemberCount(), 1);
}

Test(JoinCommand, first_member_becomes_operator)
{
	Server server(6667, "test123");
	Client client(3, "localhost");
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	JoinCommand cmd(server);
	Message msg;
	msg.m_command = "JOIN";
	msg.m_params.push_back("#new");

	cmd.execute(&client, msg);

	IChannel* channel = server.getChannel("#new");
	cr_assert(channel->isOperator(&client));
}

Test(JoinCommand, ampersand_channel_prefix)
{
	Server server(6667, "test123");
	Client client(3, "localhost");
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	JoinCommand cmd(server);
	Message msg;
	msg.m_command = "JOIN";
	msg.m_params.push_back("&local");

	cmd.execute(&client, msg);

	cr_assert(client.isInChannel("&local"));
	IChannel* channel = server.getChannel("&local");
	cr_assert_not_null(channel);
}

Test(JoinCommand, empty_channel_name)
{
	Server server(6667, "test123");
	Client client(3, "localhost");
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	JoinCommand cmd(server);
	Message msg;
	msg.m_command = "JOIN";
	msg.m_params.push_back("");

	cmd.execute(&client, msg);

	cr_assert_eq(server.getChannelCount(), 0);
}

Test(JoinCommand, mixed_valid_invalid_channels)
{
	Server server(6667, "test123");
	Client client(3, "localhost");
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	JoinCommand cmd(server);
	Message msg;
	msg.m_command = "JOIN";
	msg.m_params.push_back("#valid,invalid,#alsoValid");

	cmd.execute(&client, msg);

	cr_assert(client.isInChannel("#valid"));
	cr_assert_not(client.isInChannel("invalid"));
	cr_assert(client.isInChannel("#alsoValid"));
	cr_assert_eq(server.getChannelCount(), 2);
}
