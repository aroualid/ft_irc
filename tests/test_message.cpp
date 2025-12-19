#include <criterion/criterion.h>
#include "protocol/Message.hpp"

Test(Message, isValid_with_command)
{
	Message msg;
	msg.m_command = "NICK";
	cr_assert(msg.isValid());
}

Test(Message, isValid_empty_command)
{
	Message msg;
	msg.m_command = "";
	cr_assert_not(msg.isValid());
}

Test(Message, isValid_with_prefix_and_command)
{
	Message msg;
	msg.m_prefix = "server.example.com";
	msg.m_command = "PRIVMSG";
	cr_assert(msg.isValid());
}

Test(Message, isValid_with_params)
{
	Message msg;
	msg.m_command = "PRIVMSG";
	msg.m_params.push_back("alice");
	msg.m_params.push_back("hello world");
	cr_assert(msg.isValid());
}

Test(Message, default_constructor_creates_invalid_message)
{
	Message msg;
	cr_assert_not(msg.isValid());
}
