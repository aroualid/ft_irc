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
	g_cmd = CommandFactory::getInstance().createCommand(irc::INVITE, *g_server);
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

static Message inviteMsg(const char* target, const char* channel)
{
	Message msg;
	msg.m_command = "INVITE";
	msg.m_params.push_back(target);
	msg.m_params.push_back(channel);
	return msg;
}

TestSuite(InviteCommand, .init = setup, .fini = teardown);

Test(InviteCommand, factory_creates_invite_command)
{
	cr_assert_not_null(g_cmd, "Factory failed to create INVITE command");
	cr_assert_str_eq(g_cmd->getName(), "INVITE");
}

Test(InviteCommand, successful_invite)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	IChannel* channel = createChannelWithAlice();

	cr_assert_not(channel->hasMember(g_bob));
	cr_assert_not(channel->isInvited(g_bob));

	g_cmd->execute(g_alice, inviteMsg("bob", "#test"));

	cr_assert(channel->isInvited(g_bob), "Bob should be invited");

	std::string aliceBuffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(aliceBuffer.find("341") != std::string::npos,
			  "Alice should receive RPL_INVITING (341)");
	cr_assert(aliceBuffer.find("bob") != std::string::npos);
	cr_assert(aliceBuffer.find("#test") != std::string::npos);

	std::string bobBuffer = g_bob->getBuffer().getWriteBuffer();
	cr_assert(bobBuffer.find("INVITE") != std::string::npos, "Bob should receive INVITE message");
	cr_assert(bobBuffer.find("bob") != std::string::npos);
	cr_assert(bobBuffer.find("#test") != std::string::npos);
}

Test(InviteCommand, err_needmoreparams_missing_channel)
{
	registerClient(g_alice, "alice");

	Message msg;
	msg.m_command = "INVITE";
	msg.m_params.push_back("bob");

	g_cmd->execute(g_alice, msg);

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("461") != std::string::npos, "Expected ERR_NEEDMOREPARAMS (461)");
}

Test(InviteCommand, err_needmoreparams_no_params)
{
	registerClient(g_alice, "alice");

	Message msg;
	msg.m_command = "INVITE";

	g_cmd->execute(g_alice, msg);

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("461") != std::string::npos, "Expected ERR_NEEDMOREPARAMS (461)");
}

Test(InviteCommand, err_nosuchnick)
{
	registerClient(g_alice, "alice");
	createChannelWithAlice();

	g_cmd->execute(g_alice, inviteMsg("nonexistent", "#test"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("401") != std::string::npos, "Expected ERR_NOSUCHNICK (401)");
	cr_assert(buffer.find("nonexistent") != std::string::npos);
}

Test(InviteCommand, err_nosuchchannel)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	g_cmd->execute(g_alice, inviteMsg("bob", "#nonexistent"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("403") != std::string::npos, "Expected ERR_NOSUCHCHANNEL (403)");
	cr_assert(buffer.find("#nonexistent") != std::string::npos);
}

Test(InviteCommand, err_notonchannel)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	IChannel* channel = g_server->createChannel("#test", g_bob);
	g_bob->joinChannel(channel);

	g_cmd->execute(g_alice, inviteMsg("bob", "#test"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("442") != std::string::npos, "Expected ERR_NOTONCHANNEL (442)");
}

Test(InviteCommand, err_useronchannel)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	IChannel* channel = createChannelWithAlice();
	addMember(channel, g_bob);

	g_cmd->execute(g_alice, inviteMsg("bob", "#test"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("443") != std::string::npos, "Expected ERR_USERONCHANNEL (443)");
	cr_assert(buffer.find("bob") != std::string::npos);
	cr_assert(buffer.find("#test") != std::string::npos);
}

Test(InviteCommand, err_chanoprivsneeded_invite_only_channel)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");
	registerClient(g_charlie, "charlie");

	IChannel* channel = createChannelWithAlice();
	addMember(channel, g_bob);
	channel->setInviteOnly(true);

	cr_assert_not(channel->isOperator(g_bob));

	g_cmd->execute(g_bob, inviteMsg("charlie", "#test"));

	std::string buffer = g_bob->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("482") != std::string::npos, "Expected ERR_CHANOPRIVSNEEDED (482)");
}

Test(InviteCommand, operator_can_invite_to_invite_only_channel)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	IChannel* channel = createChannelWithAlice();
	channel->setInviteOnly(true);

	cr_assert(channel->isOperator(g_alice));

	g_cmd->execute(g_alice, inviteMsg("bob", "#test"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("341") != std::string::npos, "Alice should receive RPL_INVITING (341)");
	cr_assert(channel->isInvited(g_bob));
}

Test(InviteCommand, non_operator_can_invite_to_normal_channel)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");
	registerClient(g_charlie, "charlie");

	IChannel* channel = createChannelWithAlice();
	addMember(channel, g_bob);

	cr_assert_not(channel->isInviteOnly());
	cr_assert_not(channel->isOperator(g_bob));

	g_cmd->execute(g_bob, inviteMsg("charlie", "#test"));

	std::string buffer = g_bob->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("341") != std::string::npos, "Bob should receive RPL_INVITING (341)");
	cr_assert(channel->isInvited(g_charlie));
}

Test(InviteCommand, invite_message_has_proper_prefix)
{
	registerClient(g_alice, "alice", "alice_user");
	g_alice->setRealname("Alice User");
	registerClient(g_bob, "bob");

	createChannelWithAlice();

	g_cmd->execute(g_alice, inviteMsg("bob", "#test"));

	std::string bobBuffer = g_bob->getBuffer().getWriteBuffer();
	cr_assert(bobBuffer.find(":alice!alice_user@localhost") != std::string::npos,
			  "INVITE message missing proper prefix");
}

Test(InviteCommand, requires_registration)
{
	g_cmd->execute(g_alice, inviteMsg("bob", "#test"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("451") != std::string::npos, "Expected ERR_NOTREGISTERED (451)");
}
