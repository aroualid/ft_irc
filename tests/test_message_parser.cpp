#include <criterion/criterion.h>
#include "protocol/MessageParser.hpp"
#include "protocol/Message.hpp"

Test(MessageParser, parse_simple_command)
{
	// RFC 1459: Simple command with one parameter
	Message msg = MessageParser::parse("NICK alice\r\n");
	cr_assert_str_eq(msg.m_command.c_str(), "NICK");
	cr_assert_eq(msg.m_params.size(), 1);
	cr_assert_str_eq(msg.m_params[0].c_str(), "alice");
	cr_assert(msg.m_prefix.empty());
}

Test(MessageParser, parse_command_with_multiple_params)
{
	// RFC 1459: Trailing param with spaces must have ':' prefix
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

Test(MessageParser, parse_command_trailing_param_with_spaces)
{
	Message msg = MessageParser::parse("PRIVMSG #channel :This is a long message\r\n");
	cr_assert_str_eq(msg.m_command.c_str(), "PRIVMSG");
	cr_assert_eq(msg.m_params.size(), 2);
	cr_assert_str_eq(msg.m_params[0].c_str(), "#channel");
	cr_assert_str_eq(msg.m_params[1].c_str(), "This is a long message");
}

Test(MessageParser, parse_command_no_params)
{
	// RFC 1459: Command with no parameters is valid
	Message msg = MessageParser::parse("PING\r\n");
	cr_assert_str_eq(msg.m_command.c_str(), "PING");
	cr_assert_eq(msg.m_params.size(), 0);
}

Test(MessageParser, parse_command_without_crlf)
{
	Message msg = MessageParser::parse("NICK alice");
	cr_assert_str_eq(msg.m_command.c_str(), "NICK");
	cr_assert_eq(msg.m_params.size(), 1);
	cr_assert_str_eq(msg.m_params[0].c_str(), "alice");
}

Test(MessageParser, serialize_simple_message)
{
	Message msg;
	msg.m_command = "NICK";
	msg.m_params.push_back("alice");

	// RFC 1459: Single param without spaces doesn't need ':'
	std::string serialized = MessageParser::serialize(msg);
	cr_assert_str_eq(serialized.c_str(), "NICK alice\r\n");
}

Test(MessageParser, serialize_message_with_prefix)
{
	Message msg;
	msg.m_prefix = "server.example.com";
	msg.m_command = "NOTICE";
	msg.m_params.push_back("alice");
	msg.m_params.push_back("Hello");

	// RFC 1459: Last param without spaces doesn't require ':'
	std::string serialized = MessageParser::serialize(msg);
	cr_assert_str_eq(serialized.c_str(), ":server.example.com NOTICE alice Hello\r\n");
}

Test(MessageParser, serialize_message_multiple_params)
{
	Message msg;
	msg.m_command = "USER";
	msg.m_params.push_back("alice");
	msg.m_params.push_back("0");
	msg.m_params.push_back("*");
	msg.m_params.push_back("Alice Wonderland");

	// RFC 1459: Last param with spaces MUST have ':' prefix
	std::string serialized = MessageParser::serialize(msg);
	cr_assert_str_eq(serialized.c_str(), "USER alice 0 * :Alice Wonderland\r\n");
}

Test(MessageParser, isValid_valid_message)
{
	// RFC 1459: Messages must end with CR-LF
	cr_assert(MessageParser::isValid("NICK alice\r\n"));
	cr_assert(MessageParser::isValid("USER alice 0 * :Alice\r\n"));
}

Test(MessageParser, isValid_empty_message)
{
	// RFC 1459: Empty messages are invalid
	cr_assert_not(MessageParser::isValid(""));
	cr_assert_not(MessageParser::isValid("\r\n"));
}

Test(MessageParser, max_message_length)
{
	// RFC 1459: Max 512 chars including CR-LF (510 for command/params)
	std::string long_msg(510, 'A');
	long_msg += "\r\n";
	cr_assert(MessageParser::isValid(long_msg));

	std::string too_long(513, 'A');
	cr_assert_not(MessageParser::isValid(too_long));
}

Test(MessageParser, max_params)
{
	// RFC 1459: Max 15 parameters
	Message msg;
	msg.m_command = "TEST";
	for (int i = 0; i < 15; i++)
		msg.m_params.push_back("param");

	std::string serialized = MessageParser::serialize(msg);
	Message reparsed = MessageParser::parse(serialized);
	cr_assert_eq(reparsed.m_params.size(), 15);
}

Test(MessageParser, round_trip_parse_and_serialize)
{
	// RFC 1459: Parse and serialize should be symmetrical
	std::string original = "PRIVMSG #test :Hello world\r\n";
	Message msg = MessageParser::parse(original);
	std::string serialized = MessageParser::serialize(msg);
	cr_assert_str_eq(serialized.c_str(), original.c_str());
}

Test(MessageParser, serialize_empty_trailing_param)
{
	// RFC 1459: Empty trailing param MUST have ':' prefix
	Message msg;
	msg.m_command = "AWAY";
	msg.m_params.push_back("");

	std::string serialized = MessageParser::serialize(msg);
	cr_assert_str_eq(serialized.c_str(), "AWAY :\r\n");
}
