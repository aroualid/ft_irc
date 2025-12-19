#include <criterion/criterion.h>
#include "protocol/IrcUtils.hpp"

Test(IrcUtils, iequals_basic_comparison)
{
	cr_assert(IrcUtils::iequals("HELLO", "hello"));
	cr_assert(IrcUtils::iequals("nick", "NICK"));
	cr_assert(IrcUtils::iequals("User", "uSeR"));
}

Test(IrcUtils, iequals_different_strings)
{
	cr_assert_not(IrcUtils::iequals("HELLO", "WORLD"));
	cr_assert_not(IrcUtils::iequals("nick", "name"));
}

Test(IrcUtils, iequals_empty_strings)
{
	cr_assert(IrcUtils::iequals("", ""));
	cr_assert_not(IrcUtils::iequals("", "test"));
	cr_assert_not(IrcUtils::iequals("test", ""));
}

Test(IrcUtils, iequals_special_irc_characters)
{
	// RFC 1459, Section 2.2: Character equivalence
	cr_assert(IrcUtils::iequals("[", "{"));
	cr_assert(IrcUtils::iequals("]", "}"));
	cr_assert(IrcUtils::iequals("\\", "|"));
}

Test(IrcUtils, iequals_mixed_special_chars)
{
	// RFC 1459: Nickname "test[nick]" equals "TEST{NICK}"
	cr_assert(IrcUtils::iequals("test[nick]", "TEST{NICK}"));
	cr_assert(IrcUtils::iequals("path\\file", "PATH|FILE"));
}

Test(IrcUtils, toLower_char_basic)
{
	cr_assert_eq(IrcUtils::toLower('A'), 'a');
	cr_assert_eq(IrcUtils::toLower('Z'), 'z');
	cr_assert_eq(IrcUtils::toLower('a'), 'a');
	cr_assert_eq(IrcUtils::toLower('z'), 'z');
}

Test(IrcUtils, toLower_char_special_irc_mappings)
{
	// RFC 1459, Section 2.2: "Because of IRC's scandanavian origin,
	// the characters {}| are considered to be the lower case equivalents
	// of the characters []\, respectively."
	cr_assert_eq(IrcUtils::toLower('['), '{');
	cr_assert_eq(IrcUtils::toLower(']'), '}');
	cr_assert_eq(IrcUtils::toLower('\\'), '|');
}

Test(IrcUtils, toLower_char_non_alpha)
{
	cr_assert_eq(IrcUtils::toLower('0'), '0');
	cr_assert_eq(IrcUtils::toLower('9'), '9');
	cr_assert_eq(IrcUtils::toLower('-'), '-');
	cr_assert_eq(IrcUtils::toLower('_'), '_');
}

Test(IrcUtils, toLower_string_basic)
{
	std::string result1 = IrcUtils::toLower("HELLO");
	std::string result2 = IrcUtils::toLower("World");
	std::string result3 = IrcUtils::toLower("TEST123");

	cr_assert_str_eq(result1.c_str(), "hello");
	cr_assert_str_eq(result2.c_str(), "world");
	cr_assert_str_eq(result3.c_str(), "test123");
}

Test(IrcUtils, toLower_string_empty)
{
	std::string result = IrcUtils::toLower("");
	cr_assert_str_eq(result.c_str(), "");
}

Test(IrcUtils, toLower_string_with_special_irc_chars)
{
	std::string result1 = IrcUtils::toLower("NICK[TEST]");
	std::string result2 = IrcUtils::toLower("PATH\\TO\\FILE");

	cr_assert_str_eq(result1.c_str(), "nick{test}");
	cr_assert_str_eq(result2.c_str(), "path|to|file");
}
