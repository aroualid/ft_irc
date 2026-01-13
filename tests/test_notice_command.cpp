#include <criterion/criterion.h>
#include "mocks/Server.hpp"
#include "mocks/Client.hpp"
#include "mocks/Channel.hpp"
#include "commands/NoticeCommand.hpp"
#include "protocol/Message.hpp"

Test(NoticeCommand, send_to_user)
{
	Server server(6667, "test123");
	Client alice(3, "localhost");
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice");

	Client bob(4, "localhost");
	bob.setPasswordProvided(true);
	bob.setNickname("bob");
	bob.setUsername("bob");

	server.registerClient("alice", &alice);
	server.registerClient("bob", &bob);

	NoticeCommand cmd(server);
	Message msg;
	msg.m_command = "NOTICE";
	msg.m_params.push_back("bob");
	msg.m_params.push_back("Notice for Bob!");

	cmd.execute(&alice, msg);

	// Bob should receive the notice
	const std::string& bob_buffer = bob.getBuffer().getWriteBuffer();
	cr_assert(bob_buffer.find("NOTICE") != std::string::npos);
	cr_assert(bob_buffer.find("bob") != std::string::npos);
	cr_assert(bob_buffer.find("Notice for Bob!") != std::string::npos);
	cr_assert(bob_buffer.find("alice!alice@localhost") != std::string::npos);
}

Test(NoticeCommand, send_to_channel)
{
	Server server(6667, "test123");
	Client alice(3, "localhost");
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice");

	Client bob(4, "localhost");
	bob.setPasswordProvided(true);
	bob.setNickname("bob");
	bob.setUsername("bob");

	// Both join channel
	IChannel* channel = server.createChannel("#test", &alice);
	alice.joinChannel("#test");
	channel->addMember(&bob);
	bob.joinChannel("#test");

	NoticeCommand cmd(server);
	Message msg;
	msg.m_command = "NOTICE";
	msg.m_params.push_back("#test");
	msg.m_params.push_back("Notice to channel!");

	cmd.execute(&alice, msg);

	// Bob should receive the notice
	const std::string& bob_buffer = bob.getBuffer().getWriteBuffer();
	cr_assert(bob_buffer.find("NOTICE") != std::string::npos);
	cr_assert(bob_buffer.find("#test") != std::string::npos);
	cr_assert(bob_buffer.find("Notice to channel!") != std::string::npos);
}

Test(NoticeCommand, no_parameters_no_error)
{
	Server server(6667, "test123");
	Client client(3, "localhost");
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	NoticeCommand cmd(server);
	Message msg;
	msg.m_command = "NOTICE";
	// No parameters

	cmd.execute(&client, msg);

	// NOTICE must NOT send error replies
	const std::string& buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.find("461") == std::string::npos); // No ERR_NEEDMOREPARAMS
}

Test(NoticeCommand, no_text_no_error)
{
	Server server(6667, "test123");
	Client client(3, "localhost");
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	NoticeCommand cmd(server);
	Message msg;
	msg.m_command = "NOTICE";
	msg.m_params.push_back("bob");
	// No text parameter

	cmd.execute(&client, msg);

	// NOTICE must NOT send error replies
	const std::string& buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.empty() || buffer.find("461") == std::string::npos);
}

Test(NoticeCommand, no_such_nick_no_error)
{
	Server server(6667, "test123");
	Client client(3, "localhost");
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	NoticeCommand cmd(server);
	Message msg;
	msg.m_command = "NOTICE";
	msg.m_params.push_back("nonexistent");
	msg.m_params.push_back("Hello?");

	cmd.execute(&client, msg);

	// NOTICE must NOT send ERR_NOSUCHNICK
	const std::string& buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.empty() || buffer.find("401") == std::string::npos);
}

Test(NoticeCommand, no_such_channel_no_error)
{
	Server server(6667, "test123");
	Client client(3, "localhost");
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	NoticeCommand cmd(server);
	Message msg;
	msg.m_command = "NOTICE";
	msg.m_params.push_back("#nonexistent");
	msg.m_params.push_back("Hello?");

	cmd.execute(&client, msg);

	// NOTICE must NOT send ERR_NOSUCHCHANNEL
	const std::string& buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.empty() || buffer.find("403") == std::string::npos);
}

Test(NoticeCommand, cannot_send_to_channel_no_error)
{
	Server server(6667, "test123");
	Client alice(3, "localhost");
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice");

	Client bob(4, "localhost");
	bob.setPasswordProvided(true);
	bob.setNickname("bob");
	bob.setUsername("bob");

	// Only alice joins the channel
	server.createChannel("#test", &alice);
	alice.joinChannel("#test");

	// Bob tries to send notice to channel
	NoticeCommand cmd(server);
	Message msg;
	msg.m_command = "NOTICE";
	msg.m_params.push_back("#test");
	msg.m_params.push_back("Hello!");

	cmd.execute(&bob, msg);

	// NOTICE must NOT send ERR_CANNOTSENDTOCHAN
	const std::string& bob_buffer = bob.getBuffer().getWriteBuffer();
	cr_assert(bob_buffer.empty() || bob_buffer.find("404") == std::string::npos);
}

Test(NoticeCommand, multiple_targets)
{
	Server server(6667, "test123");
	Client alice(3, "localhost");
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice");

	Client bob(4, "localhost");
	bob.setPasswordProvided(true);
	bob.setNickname("bob");
	bob.setUsername("bob");

	Client charlie(5, "localhost");
	charlie.setPasswordProvided(true);
	charlie.setNickname("charlie");
	charlie.setUsername("charlie");

	server.registerClient("alice", &alice);
	server.registerClient("bob", &bob);
	server.registerClient("charlie", &charlie);

	NoticeCommand cmd(server);
	Message msg;
	msg.m_command = "NOTICE";
	msg.m_params.push_back("bob,charlie");
	msg.m_params.push_back("Notice for all!");

	cmd.execute(&alice, msg);

	// Both bob and charlie should receive the notice
	const std::string& bob_buffer = bob.getBuffer().getWriteBuffer();
	cr_assert(bob_buffer.find("Notice for all!") != std::string::npos);

	const std::string& charlie_buffer = charlie.getBuffer().getWriteBuffer();
	cr_assert(charlie_buffer.find("Notice for all!") != std::string::npos);
}

Test(NoticeCommand, mixed_user_and_channel_targets)
{
	Server server(6667, "test123");
	Client alice(3, "localhost");
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice");

	Client bob(4, "localhost");
	bob.setPasswordProvided(true);
	bob.setNickname("bob");
	bob.setUsername("bob");

	server.registerClient("alice", &alice);
	server.registerClient("bob", &bob);

	// Create channel with both members
	IChannel* channel = server.createChannel("#test", &alice);
	alice.joinChannel("#test");
	channel->addMember(&bob);
	bob.joinChannel("#test");

	NoticeCommand cmd(server);
	Message msg;
	msg.m_command = "NOTICE";
	msg.m_params.push_back("bob,#test");
	msg.m_params.push_back("Notice to both!");

	cmd.execute(&alice, msg);

	// Bob should receive notice
	const std::string& bob_buffer = bob.getBuffer().getWriteBuffer();
	cr_assert(bob_buffer.find("Notice to both!") != std::string::npos);
}

Test(NoticeCommand, sender_not_in_broadcast)
{
	Server server(6667, "test123");
	Client alice(3, "localhost");
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice");

	// Alice creates and joins channel
	server.createChannel("#test", &alice);
	alice.joinChannel("#test");

	NoticeCommand cmd(server);
	Message msg;
	msg.m_command = "NOTICE";
	msg.m_params.push_back("#test");
	msg.m_params.push_back("Test notice");

	cmd.execute(&alice, msg);

	// Alice should NOT receive her own notice
}

Test(NoticeCommand, message_with_spaces)
{
	Server server(6667, "test123");
	Client alice(3, "localhost");
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice");

	Client bob(4, "localhost");
	bob.setPasswordProvided(true);
	bob.setNickname("bob");
	bob.setUsername("bob");

	server.registerClient("alice", &alice);
	server.registerClient("bob", &bob);

	NoticeCommand cmd(server);
	Message msg;
	msg.m_command = "NOTICE";
	msg.m_params.push_back("bob");
	msg.m_params.push_back("This is a notice with spaces!");

	cmd.execute(&alice, msg);

	const std::string& bob_buffer = bob.getBuffer().getWriteBuffer();
	cr_assert(bob_buffer.find("This is a notice with spaces!") != std::string::npos);
}

Test(NoticeCommand, ampersand_channel_prefix)
{
	Server server(6667, "test123");
	Client alice(3, "localhost");
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice");

	Client bob(4, "localhost");
	bob.setPasswordProvided(true);
	bob.setNickname("bob");
	bob.setUsername("bob");

	// Create channel with & prefix
	IChannel* channel = server.createChannel("&local", &alice);
	alice.joinChannel("&local");
	channel->addMember(&bob);
	bob.joinChannel("&local");

	NoticeCommand cmd(server);
	Message msg;
	msg.m_command = "NOTICE";
	msg.m_params.push_back("&local");
	msg.m_params.push_back("Local notice");

	cmd.execute(&alice, msg);

	const std::string& bob_buffer = bob.getBuffer().getWriteBuffer();
	cr_assert(bob_buffer.find("&local") != std::string::npos);
	cr_assert(bob_buffer.find("Local notice") != std::string::npos);
}

Test(NoticeCommand, empty_message_text)
{
	Server server(6667, "test123");
	Client alice(3, "localhost");
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice");

	Client bob(4, "localhost");
	bob.setPasswordProvided(true);
	bob.setNickname("bob");
	bob.setUsername("bob");

	server.registerClient("alice", &alice);
	server.registerClient("bob", &bob);

	NoticeCommand cmd(server);
	Message msg;
	msg.m_command = "NOTICE";
	msg.m_params.push_back("bob");
	msg.m_params.push_back("");

	cmd.execute(&alice, msg);

	// Should still send the notice (even if empty)
	const std::string& bob_buffer = bob.getBuffer().getWriteBuffer();
	cr_assert(bob_buffer.find("NOTICE") != std::string::npos);
	cr_assert(bob_buffer.find("bob") != std::string::npos);
}

Test(NoticeCommand, prefix_format)
{
	Server server(6667, "test123");
	Client alice(3, "localhost");
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice_user");

	Client bob(4, "localhost");
	bob.setPasswordProvided(true);
	bob.setNickname("bob");
	bob.setUsername("bob");

	server.registerClient("alice", &alice);
	server.registerClient("bob", &bob);

	NoticeCommand cmd(server);
	Message msg;
	msg.m_command = "NOTICE";
	msg.m_params.push_back("bob");
	msg.m_params.push_back("Test");

	cmd.execute(&alice, msg);

	// Check prefix format: nick!user@host
	const std::string& bob_buffer = bob.getBuffer().getWriteBuffer();
	cr_assert(bob_buffer.find("alice!alice_user@localhost") != std::string::npos);
}

Test(NoticeCommand, multiple_members_all_receive)
{
	Server server(6667, "test123");
	Client alice(3, "localhost");
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice");

	Client bob(4, "localhost");
	bob.setPasswordProvided(true);
	bob.setNickname("bob");
	bob.setUsername("bob");

	Client charlie(5, "localhost");
	charlie.setPasswordProvided(true);
	charlie.setNickname("charlie");
	charlie.setUsername("charlie");

	// All join channel
	IChannel* channel = server.createChannel("#test", &alice);
	alice.joinChannel("#test");
	channel->addMember(&bob);
	bob.joinChannel("#test");
	channel->addMember(&charlie);
	charlie.joinChannel("#test");

	NoticeCommand cmd(server);
	Message msg;
	msg.m_command = "NOTICE";
	msg.m_params.push_back("#test");
	msg.m_params.push_back("Notice to all!");

	cmd.execute(&alice, msg);

	// Bob and Charlie should receive (alice excluded)
	const std::string& bob_buffer = bob.getBuffer().getWriteBuffer();
	cr_assert(bob_buffer.find("Notice to all!") != std::string::npos);

	const std::string& charlie_buffer = charlie.getBuffer().getWriteBuffer();
	cr_assert(charlie_buffer.find("Notice to all!") != std::string::npos);
}

Test(NoticeCommand, no_automatic_replies)
{
	// This test verifies that NOTICE doesn't send errors
	// which prevents infinite loops with auto-responders
	Server server(6667, "test123");
	Client alice(3, "localhost");
	alice.setPasswordProvided(true);
	alice.setNickname("alice");
	alice.setUsername("alice");

	NoticeCommand cmd(server);
	Message msg;
	msg.m_command = "NOTICE";
	msg.m_params.push_back("nonexistent");
	msg.m_params.push_back("Test");

	cmd.execute(&alice, msg);

	// Buffer should not contain any error codes
	const std::string& buffer = alice.getBuffer().getWriteBuffer();
	cr_assert(buffer.empty() ||
		(buffer.find("401") == std::string::npos && // No such nick
		 buffer.find("403") == std::string::npos && // No such channel
		 buffer.find("404") == std::string::npos && // Cannot send to chan
		 buffer.find("461") == std::string::npos)); // Need more params
}

Test(NoticeCommand, empty_target)
{
	Server server(6667, "test123");
	Client client(3, "localhost");
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	NoticeCommand cmd(server);
	Message msg;
	msg.m_command = "NOTICE";
	msg.m_params.push_back("");
	msg.m_params.push_back("Hello");

	cmd.execute(&client, msg);

	// Should silently ignore empty target (no error)
	const std::string& buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.empty() || buffer.find("40") == std::string::npos);
}

Test(NoticeCommand, silently_fails_on_invalid_targets)
{
	Server server(6667, "test123");
	Client client(3, "localhost");
	client.setPasswordProvided(true);
	client.setNickname("alice");
	client.setUsername("alice");

	NoticeCommand cmd(server);
	Message msg;
	msg.m_command = "NOTICE";
	msg.m_params.push_back("#nonexistent,baduser,#anotherbad");
	msg.m_params.push_back("Test");

	cmd.execute(&client, msg);

	// All targets fail, but NOTICE sends no errors
	const std::string& buffer = client.getBuffer().getWriteBuffer();
	cr_assert(buffer.empty() ||
		(buffer.find("401") == std::string::npos &&
		 buffer.find("403") == std::string::npos));
}
