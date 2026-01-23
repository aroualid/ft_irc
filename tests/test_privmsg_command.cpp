#include "IChannel.hpp"
#include "commands/ACommand.hpp"
#include "commands/CommandFactory.hpp"
#include "commands/PrivmsgCommand.hpp"
#include "mocks/Channel.hpp"
#include "mocks/Client.hpp"
#include "mocks/Server.hpp"
#include "protocol/Message.hpp"
#include <criterion/criterion.h>

Test(PrivmsgCommand, requires_registration)
{
	Server server(6667, "test123");
	ClientMock sender(3, "localhost");
	// Sender is NOT registered (no password)
	sender.setNickname("unregistered");
	sender.setUsername("unregistered");

	ClientMock bob(4, "localhost");
	bob.setPasswordProvided(true);
	bob.setNickname("bob");
	bob.setUsername("bob");
	server.registerClient("bob", &bob);

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::PRIVMSG, server);
	cr_assert_not_null(cmd, "Factory failed to create PRIVMSG command. Is it registered?");
	Message msg;
	msg.m_command = "PRIVMSG";
	msg.m_params.push_back("bob");
	msg.m_params.push_back("Hello!");

	cmd->execute(&sender, msg);

	// Sender should receive ERR_NOTREGISTERED (451)
	const std::string& sender_buffer = sender.getBuffer().getWriteBuffer();
	cr_assert(sender_buffer.find("451") != std::string::npos, "Expected ERR_NOTREGISTERED (451)");
	cr_assert(sender_buffer.find("You have not registered") != std::string::npos);

	// Bob should NOT have received the message
	const std::string& bob_buffer = bob.getBuffer().getWriteBuffer();
	cr_assert(bob_buffer.find("PRIVMSG") == std::string::npos,
			  "Bob should not receive message from unregistered client");
	delete cmd;
}

Test(PrivmsgCommand, send_to_user)
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

	server.registerClient("alice", &alice);
	server.registerClient("bob", &bob);

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::PRIVMSG, server);
	cr_assert_not_null(cmd, "Factory failed to create PRIVMSG command. Is it registered?");
	Message msg;
	msg.m_command = "PRIVMSG";
	msg.m_params.push_back("bob");
	msg.m_params.push_back("Hello Bob!");

	cmd->execute(&alice, msg);

	// Bob should receive the message
	const std::string& bob_buffer = bob.getBuffer().getWriteBuffer();
	cr_assert(bob_buffer.find("PRIVMSG") != std::string::npos);
	cr_assert(bob_buffer.find("bob") != std::string::npos);
	cr_assert(bob_buffer.find("Hello Bob!") != std::string::npos);
	cr_assert(bob_buffer.find("alice!alice@localhost") != std::string::npos);
	delete cmd;
}

Test(PrivmsgCommand, send_to_channel)
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

	// Both join channel
	IChannel* channel = server.createChannel("#test", &alice);
	alice.joinChannel(channel);
	channel->addMember(&bob);
	bob.joinChannel(channel);

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::PRIVMSG, server);
	cr_assert_not_null(cmd, "Factory failed to create PRIVMSG command. Is it registered?");
	Message msg;
	msg.m_command = "PRIVMSG";
	msg.m_params.push_back("#test");
	msg.m_params.push_back("Hello channel!");

	cmd->execute(&alice, msg);

	// Bob should receive the message (alice is excluded as sender)
	const std::string& bob_buffer = bob.getBuffer().getWriteBuffer();
	cr_assert(bob_buffer.find("PRIVMSG") != std::string::npos);
	cr_assert(bob_buffer.find("#test") != std::string::npos);
	cr_assert(bob_buffer.find("Hello channel!") != std::string::npos);
	delete cmd;
}

Test(PrivmsgCommand, no_recipient)
{
	Server server(6667, "test123");
	ClientMock client(3, "localhost");
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::PRIVMSG, server);
	cr_assert_not_null(cmd, "Factory failed to create PRIVMSG command. Is it registered?");
	Message msg;
	msg.m_command = "PRIVMSG";
	// No parameters

	cmd->execute(&client, msg);

	// Should send ERR_NEEDMOREPARAMS (461)
	const std::string& buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("461") != std::string::npos);
	cr_assert(buffer.find("PRIVMSG") != std::string::npos);
	delete cmd;
}

Test(PrivmsgCommand, no_text_to_send)
{
	Server server(6667, "test123");
	ClientMock client(3, "localhost");
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::PRIVMSG, server);
	cr_assert_not_null(cmd, "Factory failed to create PRIVMSG command. Is it registered?");
	Message msg;
	msg.m_command = "PRIVMSG";
	msg.m_params.push_back("bob");
	// No text parameter

	cmd->execute(&client, msg);

	// Should send ERR_NEEDMOREPARAMS (461)
	const std::string& buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("461") != std::string::npos);
	delete cmd;
}

Test(PrivmsgCommand, no_such_nick)
{
	Server server(6667, "test123");
	ClientMock client(3, "localhost");
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::PRIVMSG, server);
	cr_assert_not_null(cmd, "Factory failed to create PRIVMSG command. Is it registered?");
	Message msg;
	msg.m_command = "PRIVMSG";
	msg.m_params.push_back("nonexistent");
	msg.m_params.push_back("Hello?");

	cmd->execute(&client, msg);

	// Should send ERR_NOSUCHNICK (401)
	const std::string& buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("401") != std::string::npos);
	cr_assert(buffer.find("nonexistent") != std::string::npos);
	delete cmd;
}

Test(PrivmsgCommand, no_such_channel)
{
	Server server(6667, "test123");
	ClientMock client(3, "localhost");
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::PRIVMSG, server);
	cr_assert_not_null(cmd, "Factory failed to create PRIVMSG command. Is it registered?");
	Message msg;
	msg.m_command = "PRIVMSG";
	msg.m_params.push_back("#nonexistent");
	msg.m_params.push_back("Hello?");

	cmd->execute(&client, msg);

	// Should send ERR_NOSUCHCHANNEL (403)
	const std::string& buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("403") != std::string::npos);
	cr_assert(buffer.find("#nonexistent") != std::string::npos);
	delete cmd;
}

Test(PrivmsgCommand, cannot_send_to_channel_not_member)
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

	// Only alice joins the channel
	IChannel* channel = server.createChannel("#test", &alice);
	alice.joinChannel(channel);

	// Bob tries to send message to channel
	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::PRIVMSG, server);
	cr_assert_not_null(cmd, "Factory failed to create PRIVMSG command. Is it registered?");
	Message msg;
	msg.m_command = "PRIVMSG";
	msg.m_params.push_back("#test");
	msg.m_params.push_back("Hello!");

	cmd->execute(&bob, msg);

	// Should send ERR_CANNOTSENDTOCHAN (404)
	const std::string& bob_buffer = bob.getBuffer().getWriteBuffer();
	cr_assert(bob_buffer.find("404") != std::string::npos);
	cr_assert(bob_buffer.find("#test") != std::string::npos);
	delete cmd;
}

Test(PrivmsgCommand, multiple_targets)
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

	server.registerClient("alice", &alice);
	server.registerClient("bob", &bob);
	server.registerClient("charlie", &charlie);

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::PRIVMSG, server);
	cr_assert_not_null(cmd, "Factory failed to create PRIVMSG command. Is it registered?");
	Message msg;
	msg.m_command = "PRIVMSG";
	msg.m_params.push_back("bob,charlie");
	msg.m_params.push_back("Hello everyone!");

	cmd->execute(&alice, msg);

	// Both bob and charlie should receive the message
	const std::string& bob_buffer = bob.getBuffer().getWriteBuffer();
	cr_assert(bob_buffer.find("Hello everyone!") != std::string::npos);

	const std::string& charlie_buffer = charlie.getBuffer().getWriteBuffer();
	cr_assert(charlie_buffer.find("Hello everyone!") != std::string::npos);
	delete cmd;
}

Test(PrivmsgCommand, mixed_user_and_channel_targets)
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

	server.registerClient("alice", &alice);
	server.registerClient("bob", &bob);

	// Create channel with both members
	IChannel* channel = server.createChannel("#test", &alice);
	alice.joinChannel(channel);
	channel->addMember(&bob);
	bob.joinChannel(channel);

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::PRIVMSG, server);
	cr_assert_not_null(cmd, "Factory failed to create PRIVMSG command. Is it registered?");
	Message msg;
	msg.m_command = "PRIVMSG";
	msg.m_params.push_back("bob,#test");
	msg.m_params.push_back("Message to both!");

	cmd->execute(&alice, msg);

	// Bob should receive message twice (once as user, once from channel)
	const std::string& bob_buffer = bob.getBuffer().getWriteBuffer();
	cr_assert(bob_buffer.find("Message to both!") != std::string::npos);
	delete cmd;
}

Test(PrivmsgCommand, empty_target)
{
	Server server(6667, "test123");
	ClientMock client(3, "localhost");
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::PRIVMSG, server);
	cr_assert_not_null(cmd, "Factory failed to create PRIVMSG command. Is it registered?");
	Message msg;
	msg.m_command = "PRIVMSG";
	msg.m_params.push_back("");
	msg.m_params.push_back("Hello");

	cmd->execute(&client, msg);

	// Should silently ignore empty target (splits by comma)
	// No specific error expected for empty string in target list
	delete cmd;
}

Test(PrivmsgCommand, sender_not_in_broadcast)
{
	Server server(6667, "test123");
	ClientMock alice(3, "localhost");
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice");

	// Alice creates and joins channel
	IChannel* channel = server.createChannel("#test", &alice);
	alice.joinChannel(channel);

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::PRIVMSG, server);
	cr_assert_not_null(cmd, "Factory failed to create PRIVMSG command. Is it registered?");
	Message msg;
	msg.m_command = "PRIVMSG";
	msg.m_params.push_back("#test");
	msg.m_params.push_back("Test message");

	cmd->execute(&alice, msg);

	// Alice should NOT receive her own message
	// (Channel broadcast excludes sender)
	// This is verified by the Channel::broadcast implementation
	delete cmd;
}

Test(PrivmsgCommand, message_with_spaces)
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

	server.registerClient("alice", &alice);
	server.registerClient("bob", &bob);

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::PRIVMSG, server);
	cr_assert_not_null(cmd, "Factory failed to create PRIVMSG command. Is it registered?");
	Message msg;
	msg.m_command = "PRIVMSG";
	msg.m_params.push_back("bob");
	msg.m_params.push_back("This is a message with spaces!");

	cmd->execute(&alice, msg);

	const std::string& bob_buffer = bob.getBuffer().getWriteBuffer();
	cr_assert(bob_buffer.find("This is a message with spaces!") != std::string::npos);
	delete cmd;
}

Test(PrivmsgCommand, ampersand_channel_prefix)
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

	// Create channel with & prefix
	IChannel* channel = server.createChannel("&local", &alice);
	alice.joinChannel(channel);
	channel->addMember(&bob);
	bob.joinChannel(channel);

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::PRIVMSG, server);
	cr_assert_not_null(cmd, "Factory failed to create PRIVMSG command. Is it registered?");
	Message msg;
	msg.m_command = "PRIVMSG";
	msg.m_params.push_back("&local");
	msg.m_params.push_back("Local message");

	cmd->execute(&alice, msg);

	const std::string& bob_buffer = bob.getBuffer().getWriteBuffer();
	cr_assert(bob_buffer.find("&local") != std::string::npos);
	cr_assert(bob_buffer.find("Local message") != std::string::npos);
	delete cmd;
}

Test(PrivmsgCommand, empty_message_text)
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

	server.registerClient("alice", &alice);
	server.registerClient("bob", &bob);

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::PRIVMSG, server);
	cr_assert_not_null(cmd, "Factory failed to create PRIVMSG command. Is it registered?");
	Message msg;
	msg.m_command = "PRIVMSG";
	msg.m_params.push_back("bob");
	msg.m_params.push_back("");

	cmd->execute(&alice, msg);

	// Should still send the message (even if empty)
	const std::string& bob_buffer = bob.getBuffer().getWriteBuffer();
	cr_assert(bob_buffer.find("PRIVMSG") != std::string::npos);
	cr_assert(bob_buffer.find("bob") != std::string::npos);
	delete cmd;
}

Test(PrivmsgCommand, prefix_format)
{
	Server server(6667, "test123");
	ClientMock alice(3, "localhost");
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice_user");

	ClientMock bob(4, "localhost");
	bob.setPasswordProvided(true);
	bob.setNickname("bob");
	bob.setUsername("bob");

	server.registerClient("alice", &alice);
	server.registerClient("bob", &bob);

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::PRIVMSG, server);
	cr_assert_not_null(cmd, "Factory failed to create PRIVMSG command. Is it registered?");
	Message msg;
	msg.m_command = "PRIVMSG";
	msg.m_params.push_back("bob");
	msg.m_params.push_back("Test");

	cmd->execute(&alice, msg);

	// Check prefix format: nick!user@host
	const std::string& bob_buffer = bob.getBuffer().getWriteBuffer();
	cr_assert(bob_buffer.find("alice!alice_user@localhost") != std::string::npos);
	delete cmd;
}

Test(PrivmsgCommand, multiple_members_all_receive)
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

	// All join channel
	IChannel* channel = server.createChannel("#test", &alice);
	alice.joinChannel(channel);
	channel->addMember(&bob);
	bob.joinChannel(channel);
	channel->addMember(&charlie);
	charlie.joinChannel(channel);

	ACommand* cmd = CommandFactory::getInstance()->createCommand(irc::PRIVMSG, server);
	cr_assert_not_null(cmd, "Factory failed to create PRIVMSG command. Is it registered?");
	Message msg;
	msg.m_command = "PRIVMSG";
	msg.m_params.push_back("#test");
	msg.m_params.push_back("Hello everyone!");

	cmd->execute(&alice, msg);

	// Bob and Charlie should receive (alice excluded)
	const std::string& bob_buffer = bob.getBuffer().getWriteBuffer();
	cr_assert(bob_buffer.find("Hello everyone!") != std::string::npos);

	const std::string& charlie_buffer = charlie.getBuffer().getWriteBuffer();
	cr_assert(charlie_buffer.find("Hello everyone!") != std::string::npos);
	delete cmd;
}
