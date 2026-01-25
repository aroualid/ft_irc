#include "commands/ACommand.hpp"
#include "commands/CommandFactory.hpp"
#include "commands/CommandType.hpp"
#include "core/IChannel.hpp"
#include "mocks/Channel.hpp"
#include "mocks/Client.hpp"
#include "mocks/Server.hpp"
#include "protocol/Message.hpp"
#include <criterion/criterion.h>

// Test fixture
static Server* g_server;
static ClientMock* g_alice;
static ClientMock* g_bob;
static ClientMock* g_charlie;
static ACommand* g_cmd;

static void setup(void)
{
	g_server = new Server(6667, "testpass");
	g_alice = new ClientMock(3, "localhost", *g_server);
	g_bob = new ClientMock(4, "localhost", *g_server);
	g_charlie = new ClientMock(5, "localhost", *g_server);
	g_cmd = CommandFactory::getInstance().createCommand(irc::JOIN, *g_server);
}

static void teardown(void)
{
	delete g_cmd;
	delete g_charlie;
	delete g_bob;
	delete g_alice;
	delete g_server;
}

// Helper to register a client
static void registerClient(ClientMock* client, const char* nick)
{
	client->setPasswordProvided(true);
	client->setNickname(nick);
	client->setUsername(nick);
}

// Helper to build a JOIN message
static Message joinMsg(const char* channels, const char* keys = NULL)
{
	Message msg;
	msg.m_command = "JOIN";
	if (channels)
		msg.m_params.push_back(channels);
	if (keys)
		msg.m_params.push_back(keys);
	return msg;
}

TestSuite(JoinCommand, .init = setup, .fini = teardown);

// ============================================================================
// RFC 4.2.1: Basic functionality
// ============================================================================

Test(JoinCommand, factory_creates_join_command)
{
	cr_assert_not_null(g_cmd, "Factory failed to create JOIN command. Is it registered?");
	cr_assert_str_eq(g_cmd->getName(), "JOIN");
}

Test(JoinCommand, single_channel_join)
{
	registerClient(g_alice, "alice");

	g_cmd->execute(g_alice, joinMsg("#test"));

	cr_assert(g_alice->isInChannel("#test"));
	cr_assert_eq(g_server->getChannelCount(), 1);

	IChannel* channel = g_server->getChannel("#test");
	cr_assert_not_null(channel);
	cr_assert_eq(channel->getMemberCount(), 1);
	cr_assert(channel->hasMember(g_alice));
}

Test(JoinCommand, first_member_becomes_operator)
{
	registerClient(g_alice, "alice");

	g_cmd->execute(g_alice, joinMsg("#new"));

	IChannel* channel = g_server->getChannel("#new");
	cr_assert(channel->isOperator(g_alice));
}

Test(JoinCommand, second_member_not_operator)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	g_cmd->execute(g_alice, joinMsg("#test"));
	g_cmd->execute(g_bob, joinMsg("#test"));

	IChannel* channel = g_server->getChannel("#test");
	cr_assert(channel->isOperator(g_alice));
	cr_assert_not(channel->isOperator(g_bob));
}

// ============================================================================
// RFC 4.2.1: Multiple channels
// ============================================================================

Test(JoinCommand, multiple_channels_comma_separated)
{
	registerClient(g_alice, "alice");

	g_cmd->execute(g_alice, joinMsg("#test,#foo,#bar"));

	cr_assert(g_alice->isInChannel("#test"));
	cr_assert(g_alice->isInChannel("#foo"));
	cr_assert(g_alice->isInChannel("#bar"));
	cr_assert_eq(g_server->getChannelCount(), 3);
}

Test(JoinCommand, multiple_channels_with_keys)
{
	registerClient(g_alice, "alice");

	g_cmd->execute(g_alice, joinMsg("#test,#foo", "key1,key2"));

	cr_assert(g_alice->isInChannel("#test"));
	cr_assert(g_alice->isInChannel("#foo"));
}

Test(JoinCommand, mixed_valid_invalid_channels)
{
	registerClient(g_alice, "alice");

	g_cmd->execute(g_alice, joinMsg("#valid,invalid,#alsoValid"));

	cr_assert(g_alice->isInChannel("#valid"));
	cr_assert_not(g_alice->isInChannel("invalid"));
	cr_assert(g_alice->isInChannel("#alsoValid"));
	cr_assert_eq(g_server->getChannelCount(), 2);
}

// ============================================================================
// RFC 4.2.1: Channel with key (+k mode)
// ============================================================================

Test(JoinCommand, channel_with_correct_key)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	g_cmd->execute(g_alice, joinMsg("#secret"));
	IChannel* channel = g_server->getChannel("#secret");
	channel->setKey("password123");

	g_cmd->execute(g_bob, joinMsg("#secret", "password123"));

	cr_assert(g_bob->isInChannel("#secret"));
	cr_assert_eq(channel->getMemberCount(), 2);
}

Test(JoinCommand, channel_with_wrong_key)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	g_cmd->execute(g_alice, joinMsg("#secret"));
	IChannel* channel = g_server->getChannel("#secret");
	channel->setKey("password123");

	g_cmd->execute(g_bob, joinMsg("#secret", "wrongpass"));

	cr_assert_not(g_bob->isInChannel("#secret"));
	std::string buffer = g_bob->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("475") != std::string::npos, "Expected ERR_BADCHANNELKEY (475)");
}

Test(JoinCommand, channel_with_no_key_when_required)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	g_cmd->execute(g_alice, joinMsg("#secret"));
	IChannel* channel = g_server->getChannel("#secret");
	channel->setKey("password123");

	g_cmd->execute(g_bob, joinMsg("#secret")); // No key provided

	cr_assert_not(g_bob->isInChannel("#secret"));
	std::string buffer = g_bob->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("475") != std::string::npos, "Expected ERR_BADCHANNELKEY (475)");
}

// ============================================================================
// RFC 4.2.1: Invite-only channel (+i mode)
// ============================================================================

Test(JoinCommand, invite_only_without_invite)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	g_cmd->execute(g_alice, joinMsg("#private"));
	IChannel* channel = g_server->getChannel("#private");
	channel->setInviteOnly(true);

	g_cmd->execute(g_bob, joinMsg("#private"));

	cr_assert_not(g_bob->isInChannel("#private"));
	std::string buffer = g_bob->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("473") != std::string::npos, "Expected ERR_INVITEONLYCHAN (473)");
}

Test(JoinCommand, invite_only_with_invite)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	g_cmd->execute(g_alice, joinMsg("#private"));
	IChannel* channel = g_server->getChannel("#private");
	channel->setInviteOnly(true);
	channel->addInvite(g_bob);

	g_cmd->execute(g_bob, joinMsg("#private"));

	cr_assert(g_bob->isInChannel("#private"));
	cr_assert_eq(channel->getMemberCount(), 2);
}

Test(JoinCommand, invite_cleared_after_join)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	g_cmd->execute(g_alice, joinMsg("#private"));
	IChannel* channel = g_server->getChannel("#private");
	channel->setInviteOnly(true);
	channel->addInvite(g_bob);

	g_cmd->execute(g_bob, joinMsg("#private"));

	// Invite should be cleared after successful join
	cr_assert_not(channel->isInvited(g_bob));
}

// ============================================================================
// RFC 4.2.1: User limit (+l mode)
// ============================================================================

Test(JoinCommand, channel_full)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	g_cmd->execute(g_alice, joinMsg("#limited"));
	IChannel* channel = g_server->getChannel("#limited");
	channel->setUserLimit(1);

	g_cmd->execute(g_bob, joinMsg("#limited"));

	cr_assert_not(g_bob->isInChannel("#limited"));
	std::string buffer = g_bob->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("471") != std::string::npos, "Expected ERR_CHANNELISFULL (471)");
}

Test(JoinCommand, channel_not_full)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	g_cmd->execute(g_alice, joinMsg("#limited"));
	IChannel* channel = g_server->getChannel("#limited");
	channel->setUserLimit(2);

	g_cmd->execute(g_bob, joinMsg("#limited"));

	cr_assert(g_bob->isInChannel("#limited"));
	cr_assert_eq(channel->getMemberCount(), 2);
}

// ============================================================================
// RFC 4.2.1: Channel name validation (RFC 1459 section 1.3)
// - Must start with # or &
// - Max 200 characters
// - No spaces, ^G, or commas
// ============================================================================

Test(JoinCommand, hash_channel_prefix)
{
	registerClient(g_alice, "alice");

	g_cmd->execute(g_alice, joinMsg("#test"));

	cr_assert(g_alice->isInChannel("#test"));
}

Test(JoinCommand, ampersand_channel_prefix)
{
	registerClient(g_alice, "alice");

	g_cmd->execute(g_alice, joinMsg("&local"));

	cr_assert(g_alice->isInChannel("&local"));
	IChannel* channel = g_server->getChannel("&local");
	cr_assert_not_null(channel);
}

Test(JoinCommand, invalid_channel_no_prefix)
{
	registerClient(g_alice, "alice");

	g_cmd->execute(g_alice, joinMsg("invalid"));

	cr_assert_not(g_alice->isInChannel("invalid"));
	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("403") != std::string::npos, "Expected ERR_NOSUCHCHANNEL (403)");
}

Test(JoinCommand, invalid_channel_with_space)
{
	registerClient(g_alice, "alice");

	g_cmd->execute(g_alice, joinMsg("#test channel"));

	cr_assert_not(g_alice->isInChannel("#test channel"));
}

Test(JoinCommand, invalid_channel_too_long)
{
	registerClient(g_alice, "alice");

	std::string longName = "#";
	for (int i = 0; i < 250; i++)
		longName += "a";

	Message msg = joinMsg(longName.c_str());
	g_cmd->execute(g_alice, msg);

	cr_assert_not(g_alice->isInChannel(longName));
}

Test(JoinCommand, empty_channel_name)
{
	registerClient(g_alice, "alice");

	g_cmd->execute(g_alice, joinMsg(""));

	cr_assert_eq(g_server->getChannelCount(), 0);
}

// ============================================================================
// RFC 4.2.1: ERR_NEEDMOREPARAMS (461)
// ============================================================================

Test(JoinCommand, error_no_parameters)
{
	registerClient(g_alice, "alice");

	Message msg;
	msg.m_command = "JOIN";
	// No params

	g_cmd->execute(g_alice, msg);

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("461") != std::string::npos, "Expected ERR_NEEDMOREPARAMS (461)");
}

// ============================================================================
// RFC 4.2.1: ERR_NOTREGISTERED (451)
// ============================================================================

Test(JoinCommand, requires_registration)
{
	g_alice->setNickname("unregistered");
	// No password, not registered

	g_cmd->execute(g_alice, joinMsg("#test"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("451") != std::string::npos, "Expected ERR_NOTREGISTERED (451)");
	cr_assert_not(g_alice->isInChannel("#test"));
}

Test(JoinCommand, requires_registration_no_password)
{
	g_alice->setNickname("alice");
	g_alice->setUsername("alice");
	// No password

	g_cmd->execute(g_alice, joinMsg("#test"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("451") != std::string::npos, "Expected ERR_NOTREGISTERED (451)");
}

// ============================================================================
// RFC 4.2.1: Already in channel
// ============================================================================

Test(JoinCommand, join_already_in_channel)
{
	registerClient(g_alice, "alice");

	g_cmd->execute(g_alice, joinMsg("#test"));
	g_cmd->execute(g_alice, joinMsg("#test"));

	IChannel* channel = g_server->getChannel("#test");
	cr_assert_eq(channel->getMemberCount(), 1);
}

// ============================================================================
// RFC 4.2.1: JOIN replies
// ============================================================================

Test(JoinCommand, receives_join_broadcast)
{
	registerClient(g_alice, "alice");

	g_cmd->execute(g_alice, joinMsg("#test"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("JOIN") != std::string::npos);
	cr_assert(buffer.find("#test") != std::string::npos);
}

Test(JoinCommand, receives_names_reply)
{
	registerClient(g_alice, "alice");

	g_cmd->execute(g_alice, joinMsg("#test"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("353") != std::string::npos, "Expected RPL_NAMREPLY (353)");
	cr_assert(buffer.find("366") != std::string::npos, "Expected RPL_ENDOFNAMES (366)");
}

Test(JoinCommand, receives_topic_reply_empty)
{
	registerClient(g_alice, "alice");

	g_cmd->execute(g_alice, joinMsg("#test"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("331") != std::string::npos, "Expected RPL_NOTOPIC (331)");
}

Test(JoinCommand, receives_topic_reply_with_topic)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	g_cmd->execute(g_alice, joinMsg("#test"));
	IChannel* channel = g_server->getChannel("#test");
	channel->setTopic("Welcome to #test!");

	g_cmd->execute(g_bob, joinMsg("#test"));

	std::string buffer = g_bob->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("332") != std::string::npos, "Expected RPL_TOPIC (332)");
	cr_assert(buffer.find("Welcome to #test!") != std::string::npos);
}

Test(JoinCommand, other_members_receive_join)
{
	registerClient(g_alice, "alice");
	registerClient(g_bob, "bob");

	g_cmd->execute(g_alice, joinMsg("#test"));
	g_alice->getBuffer().getWriteBuffer(); // Clear alice's buffer

	g_cmd->execute(g_bob, joinMsg("#test"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("JOIN") != std::string::npos);
	cr_assert(buffer.find("bob") != std::string::npos);
}

// ============================================================================
// Edge cases
// ============================================================================

Test(JoinCommand, channel_name_with_numbers)
{
	registerClient(g_alice, "alice");

	g_cmd->execute(g_alice, joinMsg("#channel123"));

	cr_assert(g_alice->isInChannel("#channel123"));
}

Test(JoinCommand, channel_name_with_dash)
{
	registerClient(g_alice, "alice");

	g_cmd->execute(g_alice, joinMsg("#my-channel"));

	cr_assert(g_alice->isInChannel("#my-channel"));
}

Test(JoinCommand, channel_name_with_underscore)
{
	registerClient(g_alice, "alice");

	g_cmd->execute(g_alice, joinMsg("#my_channel"));

	cr_assert(g_alice->isInChannel("#my_channel"));
}

Test(JoinCommand, more_channels_than_keys)
{
	registerClient(g_alice, "alice");

	// 3 channels, only 1 key
	g_cmd->execute(g_alice, joinMsg("#a,#b,#c", "key1"));

	cr_assert(g_alice->isInChannel("#a"));
	cr_assert(g_alice->isInChannel("#b"));
	cr_assert(g_alice->isInChannel("#c"));
}

Test(JoinCommand, join_message_has_prefix)
{
	registerClient(g_alice, "alice");
	g_alice->setUsername("alice_user");

	g_cmd->execute(g_alice, joinMsg("#test"));

	std::string buffer = g_alice->getBuffer().getWriteBuffer();
	cr_assert(buffer.find("alice!alice_user@localhost") != std::string::npos,
			  "JOIN message should have proper prefix");
}
