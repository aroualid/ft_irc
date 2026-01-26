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
static ACommand* g_cmd;

static void setup(void)
{
	g_server = new Server(6667, "test123");
	g_alice = new ClientMock(3, "localhost", *g_server);
	g_bob = new ClientMock(4, "localhost", *g_server);
	g_cmd = CommandFactory::getInstance().createCommand(irc::TOPIC, *g_server);
}

static void teardown(void)
{
	delete g_cmd;
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

static Message topicMsg(const char* channel, const char* topic = NULL)
{
	Message msg;
	msg.m_command = "TOPIC";
	msg.m_params.push_back(channel);
	if (topic)
		msg.m_params.push_back(topic);
	return msg;
}

TestSuite(TopicCommand, .init = setup, .fini = teardown);

Test(TopicCommand, factory_creates_topic_command)
{
	cr_assert_not_null(g_cmd, "Factory failed to create TOPIC command");
	cr_assert_str_eq(g_cmd->getName(), "TOPIC");
}

Test(TopicCommand, query_no_topic)
{
	registerClient(g_alice, "alice");
	createChannelWithAlice();

	g_cmd->execute(g_alice, topicMsg("#test"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("331") != std::string::npos, "Expected RPL_NOTOPIC (331)");
	cr_assert(buffer.find("#test") != std::string::npos);
}

Test(TopicCommand, query_existing_topic)
{
	registerClient(g_alice, "alice");
	IChannel* channel = createChannelWithAlice();
	channel->setTopic("Welcome to #test!");

	g_cmd->execute(g_alice, topicMsg("#test"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("332") != std::string::npos, "Expected RPL_TOPIC (332)");
	cr_assert(buffer.find("Welcome to #test!") != std::string::npos);
}

Test(TopicCommand, set_topic_success)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");
	IChannel* channel = createChannelWithAlice();
	addMember(channel, g_bob);
	channel->setTopicRestricted(false);

	g_cmd->execute(g_alice, topicMsg("#test", "New topic here"));

	cr_assert_str_eq(channel->getTopic().c_str(), "New topic here");

	std::string aliceBuffer = g_alice->getBuffer().getWriteBuffer();
	std::string bobBuffer = g_bob->getBuffer().getWriteBuffer();
	cr_assert(aliceBuffer.find("TOPIC") != std::string::npos,
			  "Alice should receive TOPIC broadcast");
	cr_assert(bobBuffer.find("TOPIC") != std::string::npos, "Bob should receive TOPIC broadcast");
	cr_assert(aliceBuffer.find("New topic here") != std::string::npos);
}

Test(TopicCommand, set_empty_topic)
{
	registerClient(g_alice, "alice");
	IChannel* channel = createChannelWithAlice();
	channel->setTopicRestricted(false);
	channel->setTopic("Old topic");

	g_cmd->execute(g_alice, topicMsg("#test", ""));

	cr_assert(channel->getTopic().empty(), "Topic should be cleared");
}

Test(TopicCommand, err_needmoreparams)
{
	registerClient(g_alice, "alice");

	Message msg;
	msg.m_command = "TOPIC";

	g_cmd->execute(g_alice, msg);

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("461") != std::string::npos, "Expected ERR_NEEDMOREPARAMS (461)");
}

Test(TopicCommand, err_nosuchchannel)
{
	registerClient(g_alice, "alice");

	g_cmd->execute(g_alice, topicMsg("#nonexistent"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("403") != std::string::npos, "Expected ERR_NOSUCHCHANNEL (403)");
}

Test(TopicCommand, err_notonchannel)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	IChannel* channel = g_server->createChannel("#test", g_bob);
	g_bob->joinChannel(channel);

	g_cmd->execute(g_alice, topicMsg("#test"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("442") != std::string::npos, "Expected ERR_NOTONCHANNEL (442)");
}

Test(TopicCommand, err_chanoprivsneeded_topic_restricted)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	IChannel* channel = createChannelWithAlice();
	addMember(channel, g_bob);
	channel->setTopicRestricted(true);

	cr_assert_not(channel->isOperator(g_bob));

	g_cmd->execute(g_bob, topicMsg("#test", "Bob's topic"));

	std::string buffer = g_bob->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("482") != std::string::npos, "Expected ERR_CHANOPRIVSNEEDED (482)");
	cr_assert(channel->getTopic().empty(), "Topic should not be changed");
}

Test(TopicCommand, operator_can_set_topic_when_restricted)
{
	registerClient(g_alice, "alice");
	IChannel* channel = createChannelWithAlice();
	channel->setTopicRestricted(true);

	cr_assert(channel->isOperator(g_alice));

	g_cmd->execute(g_alice, topicMsg("#test", "Operator topic"));

	cr_assert_str_eq(channel->getTopic().c_str(), "Operator topic");
}

Test(TopicCommand, non_operator_can_set_topic_when_not_restricted)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	IChannel* channel = createChannelWithAlice();
	addMember(channel, g_bob);
	channel->setTopicRestricted(false);

	cr_assert_not(channel->isOperator(g_bob));

	g_cmd->execute(g_bob, topicMsg("#test", "Bob's topic"));

	cr_assert_str_eq(channel->getTopic().c_str(), "Bob's topic");
}

Test(TopicCommand, non_operator_can_query_topic_when_restricted)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	IChannel* channel = createChannelWithAlice();
	addMember(channel, g_bob);
	channel->setTopicRestricted(true);
	channel->setTopic("Channel topic");

	cr_assert_not(channel->isOperator(g_bob));

	g_cmd->execute(g_bob, topicMsg("#test"));

	std::string buffer = g_bob->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("332") != std::string::npos, "Expected RPL_TOPIC (332)");
	cr_assert(buffer.find("Channel topic") != std::string::npos);
}

Test(TopicCommand, topic_message_has_proper_prefix)
{
	registerClient(g_alice, "alice", "alice_user");
	g_alice->setRealname("Alice User");
	registerClient(g_bob, "bob");

	IChannel* channel = createChannelWithAlice();
	addMember(channel, g_bob);
	channel->setTopicRestricted(false);

	g_cmd->execute(g_alice, topicMsg("#test", "New topic"));

	std::string bobBuffer = g_bob->getBuffer().getWriteBuffer();
	cr_assert(bobBuffer.find(":alice!alice_user@localhost") != std::string::npos,
			  "TOPIC message missing proper prefix");
}

Test(TopicCommand, requires_registration)
{
	g_cmd->execute(g_alice, topicMsg("#test"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("451") != std::string::npos, "Expected ERR_NOTREGISTERED (451)");
}

Test(TopicCommand, topic_with_spaces)
{
	registerClient(g_alice, "alice");
	IChannel* channel = createChannelWithAlice();
	channel->setTopicRestricted(false);

	g_cmd->execute(g_alice, topicMsg("#test", "This is a topic with multiple words"));

	cr_assert_str_eq(channel->getTopic().c_str(), "This is a topic with multiple words");
}
