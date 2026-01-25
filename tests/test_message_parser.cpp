#include "mocks/MockOutput.hpp"
#include "protocol/Message.hpp"
#include "protocol/MessageParser.hpp"
#include <criterion/criterion.h>
#include <string>

// Test fixture
static void setup(void)
{
	MockOutput::getInstance().clear();
}

static void teardown(void)
{
	MockOutput::getInstance().clear();
}

TestSuite(MessageParser, .init = setup, .fini = teardown);

// ============================================================================
// RFC 1459 Section 2.3: Message Format
// <message>  ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
// <prefix>   ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
// <command>  ::= <letter> { <letter> } | <number> <number> <number>
// <params>   ::= <SPACE> [ ':' <trailing> | <middle> <params> ]
// ============================================================================

// ----------------------------------------------------------------------------
// Basic Parsing Tests
// ----------------------------------------------------------------------------

Test(MessageParser, parse_simple_command)
{
	Message msg = MessageParser::parse("NICK alice\r\n");

	cr_assert_str_eq(msg.m_command.c_str(), "NICK");
	cr_assert_eq(msg.m_params.size(), 1);
	cr_assert_str_eq(msg.m_params[0].c_str(), "alice");
	cr_assert(msg.m_prefix.empty());
}

Test(MessageParser, parse_command_no_params)
{
	Message msg = MessageParser::parse("QUIT\r\n");

	cr_assert_str_eq(msg.m_command.c_str(), "QUIT");
	cr_assert_eq(msg.m_params.size(), 0);
}

Test(MessageParser, parse_command_multiple_params)
{
	Message msg = MessageParser::parse("USER alice 0 * :Alice Wonderland\r\n");

	cr_assert_str_eq(msg.m_command.c_str(), "USER");
	cr_assert_eq(msg.m_params.size(), 4);
	cr_assert_str_eq(msg.m_params[0].c_str(), "alice");
	cr_assert_str_eq(msg.m_params[1].c_str(), "0");
	cr_assert_str_eq(msg.m_params[2].c_str(), "*");
	cr_assert_str_eq(msg.m_params[3].c_str(), "Alice Wonderland");
}

Test(MessageParser, parse_command_with_prefix)
{
	Message msg = MessageParser::parse(":server.example.com NOTICE alice :Hello\r\n");

	cr_assert_str_eq(msg.m_prefix.c_str(), "server.example.com");
	cr_assert_str_eq(msg.m_command.c_str(), "NOTICE");
	cr_assert_eq(msg.m_params.size(), 2);
	cr_assert_str_eq(msg.m_params[0].c_str(), "alice");
	cr_assert_str_eq(msg.m_params[1].c_str(), "Hello");
}

Test(MessageParser, parse_prefix_with_user_host)
{
	Message msg = MessageParser::parse(":alice!alice@localhost JOIN #test\r\n");

	cr_assert_str_eq(msg.m_prefix.c_str(), "alice!alice@localhost");
	cr_assert_str_eq(msg.m_command.c_str(), "JOIN");
	cr_assert_eq(msg.m_params.size(), 1);
	cr_assert_str_eq(msg.m_params[0].c_str(), "#test");
}

// ----------------------------------------------------------------------------
// Trailing Parameter Tests (RFC 1459: params starting with ':')
// ----------------------------------------------------------------------------

Test(MessageParser, parse_trailing_param_with_spaces)
{
	Message msg = MessageParser::parse("PRIVMSG #channel :This is a long message\r\n");

	cr_assert_str_eq(msg.m_command.c_str(), "PRIVMSG");
	cr_assert_eq(msg.m_params.size(), 2);
	cr_assert_str_eq(msg.m_params[0].c_str(), "#channel");
	cr_assert_str_eq(msg.m_params[1].c_str(), "This is a long message");
}

Test(MessageParser, parse_empty_trailing_param)
{
	Message msg = MessageParser::parse("AWAY :\r\n");

	cr_assert_str_eq(msg.m_command.c_str(), "AWAY");
	cr_assert_eq(msg.m_params.size(), 1);
	cr_assert_str_eq(msg.m_params[0].c_str(), "");
}

Test(MessageParser, parse_trailing_with_multiple_colons)
{
	Message msg = MessageParser::parse("PRIVMSG #test :Hello :world: how are you?\r\n");

	cr_assert_str_eq(msg.m_command.c_str(), "PRIVMSG");
	cr_assert_eq(msg.m_params.size(), 2);
	cr_assert_str_eq(msg.m_params[0].c_str(), "#test");
	cr_assert_str_eq(msg.m_params[1].c_str(), "Hello :world: how are you?");
}

Test(MessageParser, parse_trailing_with_leading_spaces)
{
	Message msg = MessageParser::parse("PRIVMSG #test :  leading spaces\r\n");

	cr_assert_eq(msg.m_params.size(), 2);
	cr_assert_str_eq(msg.m_params[1].c_str(), "  leading spaces");
}

Test(MessageParser, parse_only_colon_trailing)
{
	Message msg = MessageParser::parse("TOPIC #test :\r\n");

	cr_assert_eq(msg.m_params.size(), 2);
	cr_assert_str_eq(msg.m_params[0].c_str(), "#test");
	cr_assert_str_eq(msg.m_params[1].c_str(), "");
}

// ----------------------------------------------------------------------------
// Line Ending Tests (RFC allows CRLF or just LF)
// ----------------------------------------------------------------------------

Test(MessageParser, parse_with_crlf)
{
	Message msg = MessageParser::parse("NICK alice\r\n");

	cr_assert_str_eq(msg.m_command.c_str(), "NICK");
	cr_assert(msg.isValid());
}

Test(MessageParser, parse_with_lf_only)
{
	Message msg = MessageParser::parse("NICK alice\n");

	cr_assert_str_eq(msg.m_command.c_str(), "NICK");
	cr_assert(msg.isValid());
}

Test(MessageParser, parse_without_line_ending)
{
	Message msg = MessageParser::parse("NICK alice");

	cr_assert_str_eq(msg.m_command.c_str(), "NICK");
	cr_assert(msg.isValid());
}

// ----------------------------------------------------------------------------
// Command Case Insensitivity (RFC 1459: commands are case-insensitive)
// ----------------------------------------------------------------------------

Test(MessageParser, parse_lowercase_command)
{
	Message msg = MessageParser::parse("nick alice\r\n");

	cr_assert_str_eq(msg.m_command.c_str(), "NICK", "Command should be uppercased");
}

Test(MessageParser, parse_mixed_case_command)
{
	Message msg = MessageParser::parse("NiCk alice\r\n");

	cr_assert_str_eq(msg.m_command.c_str(), "NICK", "Command should be uppercased");
}

Test(MessageParser, parse_numeric_command)
{
	Message msg = MessageParser::parse("001 alice :Welcome\r\n");

	cr_assert_str_eq(msg.m_command.c_str(), "001");
	cr_assert_eq(msg.m_params.size(), 2);
}

// ----------------------------------------------------------------------------
// RFC 1459 Section 2.3: Message Length Limits
// Max 512 bytes including CRLF (510 for content)
// ----------------------------------------------------------------------------

Test(MessageParser, parse_max_length_510_chars)
{
	std::string content(508, 'A');
	std::string msg_str = "X " + content + "\r\n"; // X + space + 508 A's = 510
	cr_assert_eq(msg_str.size(), 512);

	Message msg = MessageParser::parse(msg_str);

	cr_assert_str_eq(msg.m_command.c_str(), "X");
	cr_assert(msg.isValid());
}

Test(MessageParser, parse_exactly_512_bytes)
{
	std::string content(505, 'A');
	std::string msg_str = "TEST " + content + "\r\n"; // TEST(4) + space(1) + 505 + \r\n(2) = 512
	cr_assert_eq(msg_str.size(), 512);

	Message msg = MessageParser::parse(msg_str);

	cr_assert(msg.isValid());
}

Test(MessageParser, parse_over_510_content_rejected)
{
	std::string content(511, 'A');
	std::string msg_str = content + "\r\n"; // 511 content + 2 CRLF = 513

	Message msg = MessageParser::parse(msg_str);

	cr_assert_not(msg.isValid(), "Messages over 510 chars content should be rejected");
}

Test(MessageParser, isValid_max_512_bytes)
{
	std::string content(510, 'A');
	std::string msg_str = content + "\r\n";

	cr_assert(MessageParser::isValid(msg_str));
}

Test(MessageParser, isValid_over_512_bytes)
{
	std::string content(513, 'A');

	cr_assert_not(MessageParser::isValid(content));
}

// ----------------------------------------------------------------------------
// RFC 1459 Section 2.3: Max 15 Parameters
// ----------------------------------------------------------------------------

Test(MessageParser, parse_max_15_params)
{
	std::string msg_str = "TEST p1 p2 p3 p4 p5 p6 p7 p8 p9 p10 p11 p12 p13 p14 p15\r\n";
	Message msg = MessageParser::parse(msg_str);

	cr_assert_eq(msg.m_params.size(), 15);
}

Test(MessageParser, parse_over_15_params_truncated)
{
	std::string msg_str = "TEST p1 p2 p3 p4 p5 p6 p7 p8 p9 p10 p11 p12 p13 p14 p15 p16 p17\r\n";
	Message msg = MessageParser::parse(msg_str);

	cr_assert_eq(msg.m_params.size(), 15, "Params over 15 should be ignored");
	cr_assert_str_eq(msg.m_params[14].c_str(), "p15");
}

Test(MessageParser, parse_trailing_as_15th_param)
{
	std::string msg_str = "TEST p1 p2 p3 p4 p5 p6 p7 p8 p9 p10 p11 p12 p13 p14 :trailing param\r\n";
	Message msg = MessageParser::parse(msg_str);

	cr_assert_eq(msg.m_params.size(), 15);
	cr_assert_str_eq(msg.m_params[14].c_str(), "trailing param");
}

// ----------------------------------------------------------------------------
// Edge Cases and Invalid Input
// ----------------------------------------------------------------------------

Test(MessageParser, parse_empty_string)
{
	Message msg = MessageParser::parse("");

	cr_assert_not(msg.isValid());
}

Test(MessageParser, parse_only_crlf)
{
	Message msg = MessageParser::parse("\r\n");

	cr_assert_not(msg.isValid());
}

Test(MessageParser, parse_only_prefix_no_command)
{
	Message msg = MessageParser::parse(":server.com\r\n");

	cr_assert_not(msg.isValid(), "Message with only prefix and no command is invalid");
}

Test(MessageParser, parse_whitespace_only)
{
	Message msg = MessageParser::parse("   \r\n");

	cr_assert_not(msg.isValid());
}

Test(MessageParser, parse_multiple_spaces_between_params)
{
	Message msg = MessageParser::parse("NICK   alice\r\n");

	cr_assert_str_eq(msg.m_command.c_str(), "NICK");
	// Multiple spaces should still parse (stringstream handles this)
	cr_assert(msg.m_params.size() >= 1);
}

Test(MessageParser, isValid_empty_message)
{
	cr_assert_not(MessageParser::isValid(""));
	cr_assert_not(MessageParser::isValid("\r\n"));
}

Test(MessageParser, isValid_valid_messages)
{
	cr_assert(MessageParser::isValid("NICK alice\r\n"));
	cr_assert(MessageParser::isValid("USER alice 0 * :Alice\r\n"));
	cr_assert(MessageParser::isValid("QUIT\r\n"));
}

// ----------------------------------------------------------------------------
// Serialization Tests
// ----------------------------------------------------------------------------

Test(MessageParser, serialize_simple_message)
{
	Message msg;
	msg.m_command = "NICK";
	msg.m_params.push_back("alice");

	std::string serialized = MessageParser::serialize(msg);

	cr_assert_str_eq(serialized.c_str(), "NICK alice\r\n");
}

Test(MessageParser, serialize_with_prefix)
{
	Message msg;
	msg.m_prefix = "alice!alice@localhost";
	msg.m_command = "JOIN";
	msg.m_params.push_back("#test");

	std::string serialized = MessageParser::serialize(msg);

	cr_assert_str_eq(serialized.c_str(), ":alice!alice@localhost JOIN #test\r\n");
}

Test(MessageParser, serialize_trailing_with_spaces)
{
	Message msg;
	msg.m_command = "PRIVMSG";
	msg.m_params.push_back("#test");
	msg.m_params.push_back("Hello world");

	std::string serialized = MessageParser::serialize(msg);

	cr_assert_str_eq(serialized.c_str(), "PRIVMSG #test :Hello world\r\n");
}

Test(MessageParser, serialize_empty_trailing)
{
	Message msg;
	msg.m_command = "AWAY";
	msg.m_params.push_back("");

	std::string serialized = MessageParser::serialize(msg);

	cr_assert_str_eq(serialized.c_str(), "AWAY :\r\n");
}

Test(MessageParser, serialize_multiple_params_no_trailing)
{
	Message msg;
	msg.m_command = "MODE";
	msg.m_params.push_back("#test");
	msg.m_params.push_back("+o");
	msg.m_params.push_back("alice");

	std::string serialized = MessageParser::serialize(msg);

	cr_assert_str_eq(serialized.c_str(), "MODE #test +o alice\r\n");
}

Test(MessageParser, serialize_empty_command)
{
	Message msg;
	msg.m_params.push_back("param");

	std::string serialized = MessageParser::serialize(msg);

	cr_assert_str_eq(serialized.c_str(), "", "Empty command should serialize to empty string");
}

Test(MessageParser, serialize_no_params)
{
	Message msg;
	msg.m_command = "QUIT";

	std::string serialized = MessageParser::serialize(msg);

	cr_assert_str_eq(serialized.c_str(), "QUIT\r\n");
}

// ----------------------------------------------------------------------------
// Round-trip Tests (parse -> serialize -> parse)
// ----------------------------------------------------------------------------

Test(MessageParser, round_trip_simple)
{
	std::string original = "NICK alice\r\n";
	Message msg = MessageParser::parse(original);
	std::string serialized = MessageParser::serialize(msg);

	cr_assert_str_eq(serialized.c_str(), original.c_str());
}

Test(MessageParser, round_trip_with_prefix)
{
	// Note: :Hello without spaces normalizes to Hello (no colon needed)
	// So we test with a message that requires the colon
	std::string original = ":server.com NOTICE alice :Hello world\r\n";
	Message msg = MessageParser::parse(original);
	std::string serialized = MessageParser::serialize(msg);

	cr_assert_str_eq(serialized.c_str(), original.c_str());
}

Test(MessageParser, round_trip_with_trailing)
{
	std::string original = "PRIVMSG #test :Hello world\r\n";
	Message msg = MessageParser::parse(original);
	std::string serialized = MessageParser::serialize(msg);

	cr_assert_str_eq(serialized.c_str(), original.c_str());
}

Test(MessageParser, round_trip_complex)
{
	std::string original = ":alice!alice@host PRIVMSG #channel :Hello everyone!\r\n";
	Message msg = MessageParser::parse(original);
	std::string serialized = MessageParser::serialize(msg);

	cr_assert_str_eq(serialized.c_str(), original.c_str());
}
