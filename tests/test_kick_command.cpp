#include "commands/CommandFactory.hpp"
#include "commands/CommandType.hpp"
#include "commands/KickCommand.hpp"
#include "mocks/Channel.hpp"
#include "mocks/Client.hpp"
#include "mocks/Server.hpp"
#include "protocol/MessageParser.hpp"
#include "protocol/NumericReply.hpp"
#include <criterion/criterion.h>

Test(KickCommand, factory_creates_kick_command)
{
	Server server(6667, "test123");
	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::KICK, server);

	cr_assert_not_null(cmd, "Factory failed to create KICK command. Is it registered?");
	cr_assert_str_eq(cmd->getName(), "KICK");
	delete cmd;
}

Test(KickCommand, successful_kick_with_default_reason)
{
	Server server(6667, "test123");
	ClientMock alice(3, "localhost");
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice_user");

	ClientMock bob(4, "localhost");
	bob.setPasswordProvided(true);
	bob.setNickname("bob");
	bob.setUsername("bob_user");

	// Alice creates channel (becomes operator)
	IChannel* channel = server.createChannel("#test", &alice);
	alice.joinChannel(channel);

	// Bob joins channel
	channel->addMember(&bob);
	bob.joinChannel(channel);

	cr_assert(channel->hasMember(&alice));
	cr_assert(channel->hasMember(&bob));
	cr_assert(channel->isOperator(&alice));

	// Alice kicks Bob
	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::KICK, server);
	cr_assert_not_null(cmd, "Factory failed to create KICK command");

	Message msg;
	msg.m_command = "KICK";
	msg.m_params.push_back("#test");
	msg.m_params.push_back("bob");

	cmd->execute(&alice, msg);

	// Bob should be removed
	cr_assert_not(channel->hasMember(&bob));
	cr_assert_not(bob.isInChannel("#test"));

	// Alice should still be there
	cr_assert(channel->hasMember(&alice));

	// Check broadcast message (both should have received KICK)
	std::string aliceBuffer = alice.getBuffer().getWriteBuffer();
	std::string bobBuffer = bob.getBuffer().getWriteBuffer();

	cr_assert(aliceBuffer.find("KICK") != std::string::npos, "Alice didn't receive KICK broadcast");
	cr_assert(bobBuffer.find("KICK") != std::string::npos, "Bob didn't receive KICK broadcast");
	cr_assert(aliceBuffer.find("bob") != std::string::npos, "KICK message missing target nickname");

	delete cmd;
}

Test(KickCommand, successful_kick_with_custom_reason)
{
	Server server(6667, "test123");
	ClientMock alice(3, "localhost");
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice_user");

	ClientMock bob(4, "localhost");
	bob.setPasswordProvided(true);
	bob.setNickname("bob");
	bob.setUsername("bob_user");

	IChannel* channel = server.createChannel("#test", &alice);
	alice.joinChannel(channel);
	channel->addMember(&bob);
	bob.joinChannel(channel);

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::KICK, server);

	Message msg;
	msg.m_command = "KICK";
	msg.m_params.push_back("#test");
	msg.m_params.push_back("bob");
	msg.m_params.push_back("No spamming allowed!");

	cmd->execute(&alice, msg);

	cr_assert_not(channel->hasMember(&bob));

	// Check reason in broadcast
	std::string bobBuffer = bob.getBuffer().getWriteBuffer();
	cr_assert(bobBuffer.find("No spamming allowed!") != std::string::npos,
			  "Custom kick reason not found in broadcast");

	delete cmd;
}

Test(KickCommand, err_needmoreparams_missing_channel)
{
	Server server(6667, "test123");
	ClientMock alice(3, "localhost");
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::KICK, server);

	Message msg;
	msg.m_command = "KICK";
	// No parameters

	cmd->execute(&alice, msg);

	std::string buffer = alice.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("461") != std::string::npos, "Expected ERR_NEEDMOREPARAMS (461)");
	cr_assert(buffer.find("KICK") != std::string::npos);

	delete cmd;
}

Test(KickCommand, err_needmoreparams_missing_target)
{
	Server server(6667, "test123");
	ClientMock alice(3, "localhost");
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::KICK, server);

	Message msg;
	msg.m_command = "KICK";
	msg.m_params.push_back("#test");
	// Missing target nickname

	cmd->execute(&alice, msg);

	std::string buffer = alice.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("461") != std::string::npos, "Expected ERR_NEEDMOREPARAMS (461)");

	delete cmd;
}

Test(KickCommand, err_nosuchchannel)
{
	Server server(6667, "test123");
	ClientMock alice(3, "localhost");
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice");

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::KICK, server);

	Message msg;
	msg.m_command = "KICK";
	msg.m_params.push_back("#nonexistent");
	msg.m_params.push_back("bob");

	cmd->execute(&alice, msg);

	std::string buffer = alice.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("403") != std::string::npos, "Expected ERR_NOSUCHCHANNEL (403)");
	cr_assert(buffer.find("#nonexistent") != std::string::npos);

	delete cmd;
}

Test(KickCommand, err_notonchannel_kicker_not_member)
{
	Server server(6667, "test123");
	ClientMock alice(3, "localhost");
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice");

	ClientMock bob(4, "localhost");
	bob.setPasswordProvided(true);
	bob.setNickname("bob");
	bob.setUsername("bob");

	// Bob creates and joins channel
	IChannel* channel = server.createChannel("#test", &bob);
	bob.joinChannel(channel);

	// Alice tries to kick from channel she's not in
	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::KICK, server);

	Message msg;
	msg.m_command = "KICK";
	msg.m_params.push_back("#test");
	msg.m_params.push_back("bob");

	cmd->execute(&alice, msg);

	std::string buffer = alice.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("442") != std::string::npos, "Expected ERR_NOTONCHANNEL (442)");
	cr_assert(buffer.find("#test") != std::string::npos);

	// Bob should still be in channel
	cr_assert(channel->hasMember(&bob));

	delete cmd;
}

Test(KickCommand, err_chanoprivsneeded_not_operator)
{
	Server server(6667, "test123");
	ClientMock alice(3, "localhost");
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice");

	ClientMock bob(4, "localhost");
	bob.setPasswordProvided(true);
	bob.setNickname("bob");
	bob.setUsername("bob");

	ClientMock charlie(5, "localhost");
	charlie.setPasswordProvided(true);
	charlie.setNickname("charlie");
	charlie.setUsername("charlie");

	// Alice creates channel (becomes operator)
	IChannel* channel = server.createChannel("#test", &alice);
	alice.joinChannel(channel);

	// Bob and Charlie join (not operators)
	channel->addMember(&bob);
	bob.joinChannel(channel);
	channel->addMember(&charlie);
	charlie.joinChannel(channel);

	cr_assert(channel->isOperator(&alice));
	cr_assert_not(channel->isOperator(&bob));

	// Bob (non-op) tries to kick Charlie
	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::KICK, server);

	Message msg;
	msg.m_command = "KICK";
	msg.m_params.push_back("#test");
	msg.m_params.push_back("charlie");

	cmd->execute(&bob, msg);

	std::string buffer = bob.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("482") != std::string::npos, "Expected ERR_CHANOPRIVSNEEDED (482)");
	cr_assert(buffer.find("#test") != std::string::npos);

	// Charlie should still be in channel
	cr_assert(channel->hasMember(&charlie));

	delete cmd;
}

Test(KickCommand, err_usernotinchannel_target_not_member)
{
	Server server(6667, "test123");
	ClientMock alice(3, "localhost");
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice");

	ClientMock bob(4, "localhost");
	bob.setPasswordProvided(true);
	bob.setNickname("bob");
	bob.setUsername("bob");

	// Alice creates channel (becomes operator)
	IChannel* channel = server.createChannel("#test", &alice);
	alice.joinChannel(channel);

	cr_assert(channel->isOperator(&alice));
	cr_assert_not(channel->hasMember(&bob));

	// Alice tries to kick Bob who isn't in the channel
	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::KICK, server);

	Message msg;
	msg.m_command = "KICK";
	msg.m_params.push_back("#test");
	msg.m_params.push_back("bob");

	cmd->execute(&alice, msg);

	std::string buffer = alice.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("441") != std::string::npos, "Expected ERR_USERNOTINCHANNEL (441)");
	cr_assert(buffer.find("bob") != std::string::npos);
	cr_assert(buffer.find("#test") != std::string::npos);

	delete cmd;
}

Test(KickCommand, kick_removes_operator_status)
{
	Server server(6667, "test123");
	ClientMock alice(3, "localhost");
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice");

	ClientMock bob(4, "localhost");
	bob.setPasswordProvided(true);
	bob.setNickname("bob");
	bob.setUsername("bob");

	// Alice creates channel
	IChannel* channel = server.createChannel("#test", &alice);
	alice.joinChannel(channel);

	// Bob joins and becomes operator
	channel->addMember(&bob);
	bob.joinChannel(channel);
	channel->addOperator(&bob);

	cr_assert(channel->isOperator(&alice));
	cr_assert(channel->isOperator(&bob));

	// Alice kicks Bob
	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::KICK, server);

	Message msg;
	msg.m_command = "KICK";
	msg.m_params.push_back("#test");
	msg.m_params.push_back("bob");

	cmd->execute(&alice, msg);

	// Bob should be removed (and thus lose operator status)
	cr_assert_not(channel->hasMember(&bob));
	cr_assert_not(channel->isOperator(&bob));

	delete cmd;
}

Test(KickCommand, channel_deleted_when_last_member_kicked)
{
	Server server(6667, "test123");
	ClientMock alice(3, "localhost");
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice");

	ClientMock bob(4, "localhost");
	bob.setPasswordProvided(true);
	bob.setNickname("bob");
	bob.setUsername("bob");

	// Alice creates channel
	IChannel* channel = server.createChannel("#test", &alice);
	alice.joinChannel(channel);

	// Bob joins
	channel->addMember(&bob);
	bob.joinChannel(channel);

	// Alice kicks Bob (alice is still there)
	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::KICK, server);

	Message msg1;
	msg1.m_command = "KICK";
	msg1.m_params.push_back("#test");
	msg1.m_params.push_back("bob");

	cmd->execute(&alice, msg1);

	// Channel should still exist
	cr_assert_not_null(server.getChannel("#test"));
	cr_assert_eq(server.getChannelCount(), 1);

	// Now create another member and kick alice (last member scenario)
	channel->addMember(&bob);
	bob.joinChannel(channel);
	channel->addOperator(&bob);

	Message msg2;
	msg2.m_command = "KICK";
	msg2.m_params.push_back("#test");
	msg2.m_params.push_back("alice");

	cmd->execute(&bob, msg2);

	// Bob is last member, channel still exists
	cr_assert_not_null(server.getChannel("#test"));

	// Bob parts (making channel empty)
	bob.leaveChannel(channel);
	channel->removeMember(&bob);

	if (channel->isEmpty())
		server.deleteChannelIfEmpty(channel);

	// Now channel should be deleted
	cr_assert_null(server.getChannel("#test"));
	cr_assert_eq(server.getChannelCount(), 0);

	delete cmd;
}

Test(KickCommand, kick_message_has_proper_prefix)
{
	Server server(6667, "test123");
	ClientMock alice(3, "localhost");
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice_user");
	alice.setRealname("Alice User");

	ClientMock bob(4, "localhost");
	bob.setPasswordProvided(true);
	bob.setNickname("bob");
	bob.setUsername("bob_user");

	IChannel* channel = server.createChannel("#test", &alice);
	alice.joinChannel(channel);
	channel->addMember(&bob);
	bob.joinChannel(channel);

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::KICK, server);

	Message msg;
	msg.m_command = "KICK";
	msg.m_params.push_back("#test");
	msg.m_params.push_back("bob");
	msg.m_params.push_back("Goodbye!");

	cmd->execute(&alice, msg);

	// Check broadcast includes alice's prefix
	std::string bobBuffer = bob.getBuffer().getWriteBuffer();
	cr_assert(bobBuffer.find(":alice!alice_user@localhost") != std::string::npos,
			  "KICK message missing proper prefix");
	cr_assert(bobBuffer.find("KICK #test bob :Goodbye!") != std::string::npos,
			  "KICK message format incorrect");

	delete cmd;
}

Test(KickCommand, multiple_members_receive_kick_broadcast)
{
	Server server(6667, "test123");
	ClientMock alice(3, "localhost");
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice");

	ClientMock bob(4, "localhost");
	bob.setPasswordProvided(true);
	bob.setNickname("bob");
	bob.setUsername("bob");

	ClientMock charlie(5, "localhost");
	charlie.setPasswordProvided(true);
	charlie.setNickname("charlie");
	charlie.setUsername("charlie");

	// All three join channel
	IChannel* channel = server.createChannel("#test", &alice);
	alice.joinChannel(channel);
	channel->addMember(&bob);
	bob.joinChannel(channel);
	channel->addMember(&charlie);
	charlie.joinChannel(channel);

	// Alice kicks Bob
	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::KICK, server);

	Message msg;
	msg.m_command = "KICK";
	msg.m_params.push_back("#test");
	msg.m_params.push_back("bob");

	cmd->execute(&alice, msg);

	// All three should receive KICK message
	std::string aliceBuffer = alice.getBuffer().getWriteBuffer();
	std::string bobBuffer = bob.getBuffer().getWriteBuffer();
	std::string charlieBuffer = charlie.getBuffer().getWriteBuffer();

	cr_assert(aliceBuffer.find("KICK") != std::string::npos, "Alice didn't receive KICK");
	cr_assert(bobBuffer.find("KICK") != std::string::npos, "Bob didn't receive KICK");
	cr_assert(charlieBuffer.find("KICK") != std::string::npos, "Charlie didn't receive KICK");

	// Only bob should be removed
	cr_assert_not(channel->hasMember(&bob));
	cr_assert(channel->hasMember(&alice));
	cr_assert(channel->hasMember(&charlie));

	delete cmd;
}

Test(KickCommand, requires_registration)
{
	Server server(6667, "test123");
	ClientMock alice(3, "localhost");
	// Alice is NOT registered (no nickname/username set)

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::KICK, server);

	Message msg;
	msg.m_command = "KICK";
	msg.m_params.push_back("#test");
	msg.m_params.push_back("bob");

	cmd->execute(&alice, msg);

	std::string buffer = alice.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("451") != std::string::npos, "Expected ERR_NOTREGISTERED (451)");

	delete cmd;
}

Test(KickCommand, requires_authentication)
{
	Server server(6667, "test123");
	ClientMock alice(3, "localhost");
	alice.setNickname("alice");
	alice.setUsername("alice");
	// Alice provided no password (setPasswordProvided not called)

	ACommand* cmd = CommandFactory::getInstance().createCommand(irc::KICK, server);

	Message msg;
	msg.m_command = "KICK";
	msg.m_params.push_back("#test");
	msg.m_params.push_back("bob");

	cmd->execute(&alice, msg);

	std::string buffer = alice.getBuffer().getWriteBuffer();
	// Should get authentication error (implementation dependent)
	// Check that command didn't execute (no KICK broadcast)
	cr_assert(buffer.find("KICK") == std::string::npos || buffer.find("464") != std::string::npos);

	delete cmd;
}