#include <criterion/criterion.h>
#include "protocol/NumericReply.hpp"

Test(NumericReply, constructor_basic)
{
	NumericReply reply(1, "alice", "Welcome message");
	std::string result = reply.toString();

	cr_assert_str_eq(result.substr(0, 3).c_str(), "001");
	cr_assert(result.find("alice") != std::string::npos);
	cr_assert(result.find("Welcome message") != std::string::npos);
}

Test(NumericReply, toString_format_has_crlf)
{
	NumericReply reply(1, "bob", "Test");
	std::string result = reply.toString();

	cr_assert(result.find("\r\n") != std::string::npos);
	cr_assert_eq(result.substr(result.size() - 2), std::string("\r\n"));
}

Test(NumericReply, welcome_factory_method)
{
	NumericReply reply = NumericReply::welcome("alice");
	std::string result = reply.toString();

	cr_assert_str_eq(result.substr(0, 3).c_str(), "001");
	cr_assert(result.find("alice") != std::string::npos);
	cr_assert(result.find("Welcome") != std::string::npos);
}

Test(NumericReply, noNicknameGiven_factory_method)
{
	NumericReply reply = NumericReply::noNicknameGiven("alice");
	std::string result = reply.toString();

	cr_assert_str_eq(result.substr(0, 3).c_str(), "431");
	cr_assert(result.find("alice") != std::string::npos);
}

Test(NumericReply, erroneusNickname_factory_method)
{
	NumericReply reply = NumericReply::erroneusNickname("alice", "bad@nick");
	std::string result = reply.toString();

	cr_assert_str_eq(result.substr(0, 3).c_str(), "432");
	cr_assert(result.find("alice") != std::string::npos);
	cr_assert(result.find("bad@nick") != std::string::npos);
}

Test(NumericReply, nicknameInUse_factory_method)
{
	NumericReply reply = NumericReply::nicknameInUse("alice", "bob");
	std::string result = reply.toString();

	cr_assert_str_eq(result.substr(0, 3).c_str(), "433");
	cr_assert(result.find("alice") != std::string::npos);
	cr_assert(result.find("bob") != std::string::npos);
}

Test(NumericReply, needMoreParams_factory_method)
{
	NumericReply reply = NumericReply::needMoreParams("alice", "USER");
	std::string result = reply.toString();

	cr_assert_str_eq(result.substr(0, 3).c_str(), "461");
	cr_assert(result.find("alice") != std::string::npos);
	cr_assert(result.find("USER") != std::string::npos);
}

Test(NumericReply, alreadyRegistered_factory_method)
{
	NumericReply reply = NumericReply::alreadyRegistered("alice");
	std::string result = reply.toString();

	cr_assert_str_eq(result.substr(0, 3).c_str(), "462");
	cr_assert(result.find("alice") != std::string::npos);
}

Test(NumericReply, passwordMismatch_factory_method)
{
	NumericReply reply = NumericReply::passwordMismatch("alice");
	std::string result = reply.toString();

	cr_assert_str_eq(result.substr(0, 3).c_str(), "464");
	cr_assert(result.find("alice") != std::string::npos);
}

Test(NumericReply, three_digit_code_padding)
{
	NumericReply reply1(1, "alice", "Test");
	NumericReply reply2(42, "alice", "Test");
	NumericReply reply3(999, "alice", "Test");

	cr_assert_str_eq(reply1.toString().substr(0, 3).c_str(), "001");
	cr_assert_str_eq(reply2.toString().substr(0, 3).c_str(), "042");
	cr_assert_str_eq(reply3.toString().substr(0, 3).c_str(), "999");
}
