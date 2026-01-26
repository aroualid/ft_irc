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
	g_cmd = CommandFactory::getInstance().createCommand(irc::WHO, *g_server);
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

static Message whoMsg(const char* target)
{
	Message msg;
	msg.m_command = "WHO";
	msg.m_params.push_back(target);
	return msg;
}

TestSuite(WhoCommand, .init = setup, .fini = teardown);

Test(WhoCommand, factory_creates_who_command)
{
	cr_assert_not_null(g_cmd, "Factory failed to create WHO command");
	cr_assert_str_eq(g_cmd->getName(), "WHO");
}

Test(WhoCommand, who_channel_with_members)
{
	registerClient(g_alice, "alice", "alice_user");
	g_alice->setRealname("Alice User");
	registerClient(g_bob, "bob", "bob_user");
	g_bob->setRealname("Bob User");

	IChannel* channel = createChannelWithAlice();
	addMember(channel, g_bob);

	g_cmd->execute(g_alice, whoMsg("#test"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("352") != std::string::npos, "Expected RPL_WHOREPLY (352)");
	cr_assert(buffer.find("315") != std::string::npos, "Expected RPL_ENDOFWHO (315)");
	cr_assert(buffer.find("alice") != std::string::npos);
	cr_assert(buffer.find("bob") != std::string::npos);
	cr_assert(buffer.find("#test") != std::string::npos);
}

Test(WhoCommand, who_channel_shows_operator_flag)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	IChannel* channel = createChannelWithAlice();
	addMember(channel, g_bob);

	cr_assert(channel->isOperator(g_alice));
	cr_assert_not(channel->isOperator(g_bob));

	g_cmd->execute(g_alice, whoMsg("#test"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("H@") != std::string::npos,
			  "Alice should have H@ flag (here + operator)");
}

Test(WhoCommand, who_nonexistent_channel)
{
	registerClient(g_alice, "alice");

	g_cmd->execute(g_alice, whoMsg("#nonexistent"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("315") != std::string::npos, "Expected RPL_ENDOFWHO (315)");
	cr_assert(buffer.find("352") == std::string::npos,
			  "Should not have WHO reply for nonexistent channel");
}

Test(WhoCommand, who_user_by_nickname)
{
	registerClient(g_alice, "alice", "alice_user");
	g_alice->setRealname("Alice User");
	registerClient(g_bob, "bob");

	g_cmd->execute(g_bob, whoMsg("alice"));

	std::string buffer = g_bob->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("352") != std::string::npos, "Expected RPL_WHOREPLY (352)");
	cr_assert(buffer.find("315") != std::string::npos, "Expected RPL_ENDOFWHO (315)");
	cr_assert(buffer.find("alice") != std::string::npos);
	cr_assert(buffer.find("alice_user") != std::string::npos);
}

Test(WhoCommand, who_nonexistent_user)
{
	registerClient(g_alice, "alice");

	g_cmd->execute(g_alice, whoMsg("nonexistent"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("315") != std::string::npos, "Expected RPL_ENDOFWHO (315)");
	cr_assert(buffer.find("352") == std::string::npos,
			  "Should not have WHO reply for nonexistent user");
}

Test(WhoCommand, who_includes_realname)
{
	registerClient(g_alice, "alice", "alice_user");
	g_alice->setRealname("Alice Real Name");

	g_cmd->execute(g_alice, whoMsg("alice"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("Alice Real Name") != std::string::npos,
			  "WHO reply should include realname");
}

Test(WhoCommand, who_includes_hostname)
{
	registerClient(g_alice, "alice");

	g_cmd->execute(g_alice, whoMsg("alice"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("localhost") != std::string::npos, "WHO reply should include hostname");
}

Test(WhoCommand, err_needmoreparams)
{
	registerClient(g_alice, "alice");

	Message msg;
	msg.m_command = "WHO";

	g_cmd->execute(g_alice, msg);

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("461") != std::string::npos, "Expected ERR_NEEDMOREPARAMS (461)");
}

Test(WhoCommand, requires_registration)
{
	g_cmd->execute(g_alice, whoMsg("#test"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("451") != std::string::npos, "Expected ERR_NOTREGISTERED (451)");
}
