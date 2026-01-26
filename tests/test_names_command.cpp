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
	g_cmd = CommandFactory::getInstance().createCommand(irc::NAMES, *g_server);
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

static Message namesMsg(const char* channel)
{
	Message msg;
	msg.m_command = "NAMES";
	msg.m_params.push_back(channel);
	return msg;
}

TestSuite(NamesCommand, .init = setup, .fini = teardown);

Test(NamesCommand, factory_creates_names_command)
{
	cr_assert_not_null(g_cmd, "Factory failed to create NAMES command");
	cr_assert_str_eq(g_cmd->getName(), "NAMES");
}

Test(NamesCommand, names_channel_with_one_member)
{
	registerClient(g_alice, "alice");
	createChannelWithAlice();

	g_cmd->execute(g_alice, namesMsg("#test"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("353") != std::string::npos, "Expected RPL_NAMREPLY (353)");
	cr_assert(buffer.find("366") != std::string::npos, "Expected RPL_ENDOFNAMES (366)");
	cr_assert(buffer.find("@alice") != std::string::npos, "Alice should have @ prefix (operator)");
}

Test(NamesCommand, names_channel_with_multiple_members)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");
	registerClient(g_charlie, "charlie");

	IChannel* channel = createChannelWithAlice();
	addMember(channel, g_bob);
	addMember(channel, g_charlie);

	g_cmd->execute(g_alice, namesMsg("#test"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("353") != std::string::npos, "Expected RPL_NAMREPLY (353)");
	cr_assert(buffer.find("alice") != std::string::npos);
	cr_assert(buffer.find("bob") != std::string::npos);
	cr_assert(buffer.find("charlie") != std::string::npos);
}

Test(NamesCommand, names_shows_operator_prefix)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	IChannel* channel = createChannelWithAlice();
	addMember(channel, g_bob);
	channel->addOperator(g_bob);

	g_cmd->execute(g_alice, namesMsg("#test"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("@alice") != std::string::npos, "Alice should have @ prefix");
	cr_assert(buffer.find("@bob") != std::string::npos, "Bob should have @ prefix");
}

Test(NamesCommand, names_nonexistent_channel)
{
	registerClient(g_alice, "alice");

	g_cmd->execute(g_alice, namesMsg("#nonexistent"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("366") != std::string::npos, "Expected RPL_ENDOFNAMES (366)");
	cr_assert(buffer.find("353") == std::string::npos,
			  "Should not have NAMREPLY for nonexistent channel");
}

Test(NamesCommand, names_channel_symbol_in_reply)
{
	registerClient(g_alice, "alice");
	createChannelWithAlice();

	g_cmd->execute(g_alice, namesMsg("#test"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("= #test") != std::string::npos, "NAMREPLY should contain '= #test'");
}

Test(NamesCommand, names_non_member_can_request)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	createChannelWithAlice();

	g_cmd->execute(g_bob, namesMsg("#test"));

	std::string buffer = g_bob->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("353") != std::string::npos, "Expected RPL_NAMREPLY (353)");
	cr_assert(buffer.find("alice") != std::string::npos);
}

Test(NamesCommand, err_needmoreparams)
{
	registerClient(g_alice, "alice");

	Message msg;
	msg.m_command = "NAMES";

	g_cmd->execute(g_alice, msg);

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("461") != std::string::npos, "Expected ERR_NEEDMOREPARAMS (461)");
}

Test(NamesCommand, requires_registration)
{
	g_cmd->execute(g_alice, namesMsg("#test"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("451") != std::string::npos, "Expected ERR_NOTREGISTERED (451)");
}
