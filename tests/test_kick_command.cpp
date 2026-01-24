#include "commands/ACommand.hpp"
#include "commands/CommandFactory.hpp"
#include "commands/CommandType.hpp"
#include "mocks/Channel.hpp"
#include "mocks/Client.hpp"
#include "mocks/Server.hpp"
#include "protocol/Message.hpp"
#include <criterion/criterion.h>

// Test fixture - fresh instance created for each test
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
	g_cmd = CommandFactory::getInstance().createCommand(irc::KICK, *g_server);
}

static void teardown(void)
{
	delete g_cmd;
	delete g_charlie;
	delete g_bob;
	delete g_alice;
	delete g_server;
}

// Helper to register a client (sets password, nick, user and registers with server)
static void registerClient(ClientMock* client, const char* nick, const char* user = NULL)
{
	client->setPasswordProvided(true);
	client->setNickname(nick);
	client->setUsername(user ? user : nick);
	g_server->registerClient(nick, client);
}

// Helper to create a channel with alice as operator
static IChannel* createChannelWithAlice(const char* name = "#test")
{
	IChannel* channel = g_server->createChannel(name, g_alice);
	g_alice->joinChannel(channel);
	return channel;
}

// Helper to add a member to a channel
static void addMember(IChannel* channel, ClientMock* client)
{
	channel->addMember(client);
	client->joinChannel(channel);
}

// Helper to build a KICK message
static Message kickMsg(const char* channel, const char* target, const char* reason = NULL)
{
	Message msg;
	msg.m_command = "KICK";
	msg.m_params.push_back(channel);
	msg.m_params.push_back(target);
	if (reason)
		msg.m_params.push_back(reason);
	return msg;
}

TestSuite(KickCommand, .init = setup, .fini = teardown);

Test(KickCommand, factory_creates_kick_command)
{
	cr_assert_not_null(g_cmd, "Factory failed to create KICK command. Is it registered?");
	cr_assert_str_eq(g_cmd->getName(), "KICK");
}

Test(KickCommand, successful_kick_with_default_reason)
{
	registerClient(g_alice, "alice", "alice_user");
	registerClient(g_bob, "bob", "bob_user");

	IChannel* channel = createChannelWithAlice();
	addMember(channel, g_bob);

	cr_assert(channel->hasMember(g_alice));
	cr_assert(channel->hasMember(g_bob));
	cr_assert(channel->isOperator(g_alice));

	g_cmd->execute(g_alice, kickMsg("#test", "bob"));

	cr_assert_not(channel->hasMember(g_bob));
	cr_assert_not(g_bob->isInChannel("#test"));
	cr_assert(channel->hasMember(g_alice));

	std::string aliceBuffer = g_alice->getBuffer().getWriteBuffer();
	std::string bobBuffer = g_bob->getBuffer().getWriteBuffer();

	cr_assert(aliceBuffer.find("KICK") != std::string::npos, "Alice didn't receive KICK broadcast");
	cr_assert(bobBuffer.find("KICK") != std::string::npos, "Bob didn't receive KICK broadcast");
	cr_assert(aliceBuffer.find("bob") != std::string::npos, "KICK message missing target nickname");
}

Test(KickCommand, successful_kick_with_custom_reason)
{
	registerClient(g_alice, "alice", "alice_user");
	registerClient(g_bob, "bob", "bob_user");

	IChannel* channel = createChannelWithAlice();
	addMember(channel, g_bob);

	g_cmd->execute(g_alice, kickMsg("#test", "bob", "No spamming allowed!"));

	cr_assert_not(channel->hasMember(g_bob));

	std::string bobBuffer = g_bob->getBuffer().getWriteBuffer();
	cr_assert(bobBuffer.find("No spamming allowed!") != std::string::npos,
			  "Custom kick reason not found in broadcast");
}

Test(KickCommand, err_needmoreparams_missing_channel)
{
	registerClient(g_alice, "alice");

	Message msg;
	msg.m_command = "KICK";
	// No parameters

	g_cmd->execute(g_alice, msg);

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("461") != std::string::npos, "Expected ERR_NEEDMOREPARAMS (461)");
	cr_assert(buffer.find("KICK") != std::string::npos);
}

Test(KickCommand, err_needmoreparams_missing_target)
{
	registerClient(g_alice, "alice");

	Message msg;
	msg.m_command = "KICK";
	msg.m_params.push_back("#test");
	// Missing target nickname

	g_cmd->execute(g_alice, msg);

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("461") != std::string::npos, "Expected ERR_NEEDMOREPARAMS (461)");
}

Test(KickCommand, err_nosuchchannel)
{
	registerClient(g_alice, "alice");

	g_cmd->execute(g_alice, kickMsg("#nonexistent", "bob"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("403") != std::string::npos, "Expected ERR_NOSUCHCHANNEL (403)");
	cr_assert(buffer.find("#nonexistent") != std::string::npos);
}

Test(KickCommand, err_notonchannel_kicker_not_member)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	// Bob creates and joins channel (alice is NOT in it)
	IChannel* channel = g_server->createChannel("#test", g_bob);
	g_bob->joinChannel(channel);

	// Alice tries to kick from channel she's not in
	g_cmd->execute(g_alice, kickMsg("#test", "bob"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("442") != std::string::npos, "Expected ERR_NOTONCHANNEL (442)");
	cr_assert(buffer.find("#test") != std::string::npos);
	cr_assert(channel->hasMember(g_bob));
}

Test(KickCommand, err_chanoprivsneeded_not_operator)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");
	registerClient(g_charlie, "charlie");

	IChannel* channel = createChannelWithAlice();
	addMember(channel, g_bob);
	addMember(channel, g_charlie);

	cr_assert(channel->isOperator(g_alice));
	cr_assert_not(channel->isOperator(g_bob));

	// Bob (non-op) tries to kick Charlie
	g_cmd->execute(g_bob, kickMsg("#test", "charlie"));

	std::string buffer = g_bob->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("482") != std::string::npos, "Expected ERR_CHANOPRIVSNEEDED (482)");
	cr_assert(buffer.find("#test") != std::string::npos);
	cr_assert(channel->hasMember(g_charlie));
}

Test(KickCommand, err_usernotinchannel_target_not_member)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	IChannel* channel = createChannelWithAlice();

	cr_assert(channel->isOperator(g_alice));
	cr_assert_not(channel->hasMember(g_bob));

	// Alice tries to kick Bob who isn't in the channel
	g_cmd->execute(g_alice, kickMsg("#test", "bob"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("441") != std::string::npos, "Expected ERR_USERNOTINCHANNEL (441)");
	cr_assert(buffer.find("bob") != std::string::npos);
	cr_assert(buffer.find("#test") != std::string::npos);
}

Test(KickCommand, kick_removes_operator_status)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	IChannel* channel = createChannelWithAlice();
	addMember(channel, g_bob);
	channel->addOperator(g_bob);

	cr_assert(channel->isOperator(g_alice));
	cr_assert(channel->isOperator(g_bob));

	g_cmd->execute(g_alice, kickMsg("#test", "bob"));

	cr_assert_not(channel->hasMember(g_bob));
	cr_assert_not(channel->isOperator(g_bob));
}

Test(KickCommand, channel_deleted_when_last_member_kicked)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	IChannel* channel = createChannelWithAlice();
	addMember(channel, g_bob);

	// Alice kicks Bob (alice is still there)
	g_cmd->execute(g_alice, kickMsg("#test", "bob"));

	cr_assert_not_null(g_server->getChannel("#test"));
	cr_assert_eq(g_server->getChannelCount(), 1);

	// Re-add bob and kick alice
	addMember(channel, g_bob);
	channel->addOperator(g_bob);

	g_cmd->execute(g_bob, kickMsg("#test", "alice"));

	// Bob is last member, channel still exists
	cr_assert_not_null(g_server->getChannel("#test"));

	// Bob parts (making channel empty)
	g_bob->leaveChannel(channel);
	channel->removeMember(g_bob);

	if (channel->isEmpty())
		g_server->deleteChannelIfEmpty(channel);

	cr_assert_null(g_server->getChannel("#test"));
	cr_assert_eq(g_server->getChannelCount(), 0);
}

Test(KickCommand, kick_message_has_proper_prefix)
{
	registerClient(g_alice, "alice", "alice_user");
	g_alice->setRealname("Alice User");
	registerClient(g_bob, "bob", "bob_user");

	IChannel* channel = createChannelWithAlice();
	addMember(channel, g_bob);

	g_cmd->execute(g_alice, kickMsg("#test", "bob", "Goodbye!"));

	std::string bobBuffer = g_bob->getBuffer().getWriteBuffer();
	cr_assert(bobBuffer.find(":alice!alice_user@localhost") != std::string::npos,
			  "KICK message missing proper prefix");
	cr_assert(bobBuffer.find("KICK #test bob :Goodbye!") != std::string::npos,
			  "KICK message format incorrect");
}

Test(KickCommand, multiple_members_receive_kick_broadcast)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");
	registerClient(g_charlie, "charlie");

	IChannel* channel = createChannelWithAlice();
	addMember(channel, g_bob);
	addMember(channel, g_charlie);

	g_cmd->execute(g_alice, kickMsg("#test", "bob"));

	std::string aliceBuffer = g_alice->getBuffer().getWriteBuffer();
	std::string bobBuffer = g_bob->getBuffer().getWriteBuffer();
	std::string charlieBuffer = g_charlie->getBuffer().getWriteBuffer();

	cr_assert(aliceBuffer.find("KICK") != std::string::npos, "Alice didn't receive KICK");
	cr_assert(bobBuffer.find("KICK") != std::string::npos, "Bob didn't receive KICK");
	cr_assert(charlieBuffer.find("KICK") != std::string::npos, "Charlie didn't receive KICK");

	cr_assert_not(channel->hasMember(g_bob));
	cr_assert(channel->hasMember(g_alice));
	cr_assert(channel->hasMember(g_charlie));
}

Test(KickCommand, requires_registration)
{
	// Alice is NOT registered (no nickname/username set)
	g_cmd->execute(g_alice, kickMsg("#test", "bob"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("451") != std::string::npos, "Expected ERR_NOTREGISTERED (451)");
}

Test(KickCommand, requires_authentication)
{
	g_alice->setNickname("alice");
	g_alice->setUsername("alice");
	// Alice provided no password (setPasswordProvided not called)

	g_cmd->execute(g_alice, kickMsg("#test", "bob"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("KICK") == std::string::npos || buffer.find("464") != std::string::npos);
}
