#include "commands/ACommand.hpp"
#include "commands/CommandFactory.hpp"
#include "commands/CommandType.hpp"
#include "mocks/Channel.hpp"
#include "mocks/Client.hpp"
#include "mocks/Server.hpp"
#include "protocol/Message.hpp"
#include <criterion/criterion.h>

static Server* g_server;
static ClientMock* g_alice;
static ClientMock* g_bob;
static ClientMock* g_charlie;
static ACommand* g_cmd;

static void setup(void)
{
	g_server = new Server(6667, "test123");
	g_alice = new ClientMock(3, "localhost", *g_server);
	g_bob = new ClientMock(4, "localhost", *g_server);
	g_charlie = new ClientMock(5, "localhost", *g_server);
	g_cmd = CommandFactory::getInstance().createCommand(irc::MODE, *g_server);
}

static void teardown(void)
{
	delete g_cmd;
	delete g_charlie;
	delete g_bob;
	delete g_alice;
	delete g_server;
}

static void registerClient(ClientMock* client, const char* nick, const char* user = NULL)
{
	client->setPasswordProvided(true);
	client->setNickname(nick);
	client->setUsername(user ? user : nick);
	g_server->registerClient(nick, client);
}

static IChannel* createChannelWithAlice(const char* name = "#test")
{
	IChannel* channel = g_server->createChannel(name, g_alice);
	g_alice->joinChannel(channel);
	return channel;
}

static void addMember(IChannel* channel, ClientMock* client)
{
	channel->addMember(client);
	client->joinChannel(channel);
}

static Message modeMsg(const char* channel, const char* modes = NULL, const char* arg1 = NULL,
					   const char* arg2 = NULL)
{
	Message msg;
	msg.m_command = "MODE";
	msg.m_params.push_back(channel);
	if (modes)
		msg.m_params.push_back(modes);
	if (arg1)
		msg.m_params.push_back(arg1);
	if (arg2)
		msg.m_params.push_back(arg2);
	return msg;
}

TestSuite(ModeCommand, .init = setup, .fini = teardown);

Test(ModeCommand, factory_creates_mode_command)
{
	cr_assert_not_null(g_cmd, "Factory failed to create MODE command");
	cr_assert_str_eq(g_cmd->getName(), "MODE");
}

Test(ModeCommand, query_mode_empty_channel)
{
	registerClient(g_alice, "alice");
	IChannel* channel = createChannelWithAlice();
	channel->setInviteOnly(false);
	channel->setTopicRestricted(false);

	g_cmd->execute(g_alice, modeMsg("#test"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("324") != std::string::npos, "Expected RPL_CHANNELMODEIS (324)");
}

Test(ModeCommand, query_mode_with_modes_set)
{
	registerClient(g_alice, "alice");
	IChannel* channel = createChannelWithAlice();
	channel->setInviteOnly(true);
	channel->setTopicRestricted(true);

	g_cmd->execute(g_alice, modeMsg("#test"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("324") != std::string::npos, "Expected RPL_CHANNELMODEIS (324)");
	cr_assert(buffer.find("i") != std::string::npos || buffer.find("t") != std::string::npos);
}

Test(ModeCommand, set_invite_only)
{
	registerClient(g_alice, "alice");
	IChannel* channel = createChannelWithAlice();
	channel->setInviteOnly(false);

	g_cmd->execute(g_alice, modeMsg("#test", "+i"));

	cr_assert(channel->isInviteOnly(), "Channel should be invite-only");
}

Test(ModeCommand, unset_invite_only)
{
	registerClient(g_alice, "alice");
	IChannel* channel = createChannelWithAlice();
	channel->setInviteOnly(true);

	g_cmd->execute(g_alice, modeMsg("#test", "-i"));

	cr_assert_not(channel->isInviteOnly(), "Channel should not be invite-only");
}

Test(ModeCommand, set_topic_restricted)
{
	registerClient(g_alice, "alice");
	IChannel* channel = createChannelWithAlice();
	channel->setTopicRestricted(false);

	g_cmd->execute(g_alice, modeMsg("#test", "+t"));

	cr_assert(channel->isTopicRestricted(), "Channel should have topic restricted");
}

Test(ModeCommand, unset_topic_restricted)
{
	registerClient(g_alice, "alice");
	IChannel* channel = createChannelWithAlice();
	channel->setTopicRestricted(true);

	g_cmd->execute(g_alice, modeMsg("#test", "-t"));

	cr_assert_not(channel->isTopicRestricted(), "Channel should not have topic restricted");
}

Test(ModeCommand, set_channel_key)
{
	registerClient(g_alice, "alice");
	IChannel* channel = createChannelWithAlice();

	g_cmd->execute(g_alice, modeMsg("#test", "+k", "secret123"));

	cr_assert_str_eq(channel->getKey().c_str(), "secret123");
}

Test(ModeCommand, unset_channel_key)
{
	registerClient(g_alice, "alice");
	IChannel* channel = createChannelWithAlice();
	channel->setKey("oldkey");

	g_cmd->execute(g_alice, modeMsg("#test", "-k"));

	cr_assert(channel->getKey().empty(), "Key should be cleared");
}

Test(ModeCommand, set_user_limit)
{
	registerClient(g_alice, "alice");
	IChannel* channel = createChannelWithAlice();

	g_cmd->execute(g_alice, modeMsg("#test", "+l", "50"));

	cr_assert_eq(channel->getUserLimit(), 50);
}

Test(ModeCommand, unset_user_limit)
{
	registerClient(g_alice, "alice");
	IChannel* channel = createChannelWithAlice();
	channel->setUserLimit(25);

	g_cmd->execute(g_alice, modeMsg("#test", "-l"));

	cr_assert_eq(channel->getUserLimit(), -1);
}

Test(ModeCommand, give_operator)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");
	IChannel* channel = createChannelWithAlice();
	addMember(channel, g_bob);

	cr_assert_not(channel->isOperator(g_bob));

	g_cmd->execute(g_alice, modeMsg("#test", "+o", "bob"));

	cr_assert(channel->isOperator(g_bob), "Bob should be operator");
}

Test(ModeCommand, take_operator)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");
	IChannel* channel = createChannelWithAlice();
	addMember(channel, g_bob);
	channel->addOperator(g_bob);

	cr_assert(channel->isOperator(g_bob));

	g_cmd->execute(g_alice, modeMsg("#test", "-o", "bob"));

	cr_assert_not(channel->isOperator(g_bob), "Bob should not be operator");
}

Test(ModeCommand, combined_modes)
{
	registerClient(g_alice, "alice");
	IChannel* channel = createChannelWithAlice();
	channel->setInviteOnly(false);
	channel->setTopicRestricted(false);

	g_cmd->execute(g_alice, modeMsg("#test", "+it"));

	cr_assert(channel->isInviteOnly(), "Channel should be invite-only");
	cr_assert(channel->isTopicRestricted(), "Channel should have topic restricted");
}

Test(ModeCommand, combined_modes_with_params)
{
	registerClient(g_alice, "alice");
	IChannel* channel = createChannelWithAlice();

	g_cmd->execute(g_alice, modeMsg("#test", "+kl", "secret", "10"));

	cr_assert_str_eq(channel->getKey().c_str(), "secret");
	cr_assert_eq(channel->getUserLimit(), 10);
}

Test(ModeCommand, mixed_plus_minus_modes)
{
	registerClient(g_alice, "alice");
	IChannel* channel = createChannelWithAlice();
	channel->setInviteOnly(true);
	channel->setTopicRestricted(false);

	g_cmd->execute(g_alice, modeMsg("#test", "-i+t"));

	cr_assert_not(channel->isInviteOnly(), "Invite-only should be off");
	cr_assert(channel->isTopicRestricted(), "Topic restricted should be on");
}

Test(ModeCommand, err_nosuchchannel)
{
	registerClient(g_alice, "alice");

	g_cmd->execute(g_alice, modeMsg("#nonexistent", "+i"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("403") != std::string::npos, "Expected ERR_NOSUCHCHANNEL (403)");
}

Test(ModeCommand, err_notonchannel)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	IChannel* channel = g_server->createChannel("#test", g_bob);
	g_bob->joinChannel(channel);

	g_cmd->execute(g_alice, modeMsg("#test", "+i"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("442") != std::string::npos, "Expected ERR_NOTONCHANNEL (442)");
}

Test(ModeCommand, err_chanoprivsneeded)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	IChannel* channel = createChannelWithAlice();
	addMember(channel, g_bob);

	cr_assert_not(channel->isOperator(g_bob));

	g_cmd->execute(g_bob, modeMsg("#test", "+i"));

	std::string buffer = g_bob->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("482") != std::string::npos, "Expected ERR_CHANOPRIVSNEEDED (482)");
}

Test(ModeCommand, err_unknownmode)
{
	registerClient(g_alice, "alice");
	createChannelWithAlice();

	g_cmd->execute(g_alice, modeMsg("#test", "+x"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("472") != std::string::npos, "Expected ERR_UNKNOWNMODE (472)");
}

Test(ModeCommand, mode_broadcast_to_channel)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");
	IChannel* channel = createChannelWithAlice();
	addMember(channel, g_bob);

	g_cmd->execute(g_alice, modeMsg("#test", "+i"));

	std::string aliceBuffer = g_alice->getBuffer().getWriteBuffer();
	std::string bobBuffer = g_bob->getBuffer().getWriteBuffer();
	cr_assert(aliceBuffer.find("MODE") != std::string::npos, "Alice should receive MODE broadcast");
	cr_assert(bobBuffer.find("MODE") != std::string::npos, "Bob should receive MODE broadcast");
}

Test(ModeCommand, mode_message_has_proper_prefix)
{
	registerClient(g_alice, "alice", "alice_user");
	g_alice->setRealname("Alice User");
	registerClient(g_bob, "bob");
	IChannel* channel = createChannelWithAlice();
	addMember(channel, g_bob);

	g_cmd->execute(g_alice, modeMsg("#test", "+i"));

	std::string bobBuffer = g_bob->getBuffer().getWriteBuffer();
	cr_assert(bobBuffer.find(":alice!alice_user@localhost") != std::string::npos,
			  "MODE message missing proper prefix");
}

Test(ModeCommand, requires_registration)
{
	g_cmd->execute(g_alice, modeMsg("#test"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("451") != std::string::npos, "Expected ERR_NOTREGISTERED (451)");
}

Test(ModeCommand, mode_key_requires_param_on_set)
{
	registerClient(g_alice, "alice");
	IChannel* channel = createChannelWithAlice();

	g_cmd->execute(g_alice, modeMsg("#test", "+k"));

	cr_assert(channel->getKey().empty(), "Key should not be set without param");
}

Test(ModeCommand, mode_limit_requires_param_on_set)
{
	registerClient(g_alice, "alice");
	IChannel* channel = createChannelWithAlice();

	g_cmd->execute(g_alice, modeMsg("#test", "+l"));

	cr_assert_eq(channel->getUserLimit(), -1, "Limit should not be set without param");
}

Test(ModeCommand, mode_operator_requires_param)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");
	IChannel* channel = createChannelWithAlice();
	addMember(channel, g_bob);

	g_cmd->execute(g_alice, modeMsg("#test", "+o"));

	cr_assert_not(channel->isOperator(g_bob), "Bob should not become operator without param");
}

Test(ModeCommand, anyone_can_query_mode)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");
	IChannel* channel = createChannelWithAlice();
	channel->setInviteOnly(true);

	g_cmd->execute(g_bob, modeMsg("#test"));

	std::string buffer = g_bob->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("324") != std::string::npos,
			  "Non-member should be able to query channel modes");
}
