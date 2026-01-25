#include "mocks/MockOutput.hpp"
#include "protocol/NumericReply.hpp"
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

TestSuite(NumericReply, .init = setup, .fini = teardown);

// ============================================================================
// RFC 1459 Section 2.4: Numeric Replies
// Format: :<server> <3-digit-code> <target> <params>
// ============================================================================

// ----------------------------------------------------------------------------
// Basic Constructor and Format Tests
// ----------------------------------------------------------------------------

Test(NumericReply, constructor_basic)
{
	NumericReply reply(1, "alice", "Welcome message");
	std::string result = reply.toString();

	cr_assert(result.find("001") != std::string::npos, "Should contain code 001");
	cr_assert(result.find("alice") != std::string::npos, "Should contain target");
	cr_assert(result.find("Welcome message") != std::string::npos, "Should contain message");
}

Test(NumericReply, toString_ends_with_crlf)
{
	NumericReply reply(1, "bob", "Test");
	std::string result = reply.toString();

	cr_assert(result.size() >= 2);
	cr_assert_eq(result.substr(result.size() - 2), std::string("\r\n"));
}

Test(NumericReply, three_digit_code_padding)
{
	NumericReply reply1(1, "alice", "Test");
	NumericReply reply2(42, "alice", "Test");
	NumericReply reply3(123, "alice", "Test");
	NumericReply reply4(999, "alice", "Test");

	cr_assert(reply1.toString().find("001") != std::string::npos, "Should pad to 001");
	cr_assert(reply2.toString().find("042") != std::string::npos, "Should pad to 042");
	cr_assert(reply3.toString().find("123") != std::string::npos, "Should be 123");
	cr_assert(reply4.toString().find("999") != std::string::npos, "Should be 999");
}

Test(NumericReply, format_structure)
{
	NumericReply reply(401, "alice", "bob :No such nick/channel");
	std::string result = reply.toString();

	// Format should be: <code> <target> <message>\r\n
	cr_assert(result.substr(0, 3) == "401");
	cr_assert(result.find("alice") != std::string::npos);
}

// ----------------------------------------------------------------------------
// Welcome Replies (001-004)
// ----------------------------------------------------------------------------

Test(NumericReply, welcome_001)
{
	NumericReply reply = NumericReply::welcome("alice");
	std::string result = reply.toString();

	cr_assert(result.find("001") != std::string::npos, "RPL_WELCOME is 001");
	cr_assert(result.find("alice") != std::string::npos);
	cr_assert(result.find("Welcome") != std::string::npos);
}

// ----------------------------------------------------------------------------
// Channel Replies (331-366)
// ----------------------------------------------------------------------------

Test(NumericReply, noTopic_331)
{
	NumericReply reply = NumericReply::noTopic("alice", "#test");
	std::string result = reply.toString();

	cr_assert(result.find("331") != std::string::npos, "RPL_NOTOPIC is 331");
	cr_assert(result.find("alice") != std::string::npos);
	cr_assert(result.find("#test") != std::string::npos);
	cr_assert(result.find("No topic") != std::string::npos);
}

Test(NumericReply, topic_332)
{
	NumericReply reply = NumericReply::topic("alice", "#test", "Welcome to the test channel!");
	std::string result = reply.toString();

	cr_assert(result.find("332") != std::string::npos, "RPL_TOPIC is 332");
	cr_assert(result.find("alice") != std::string::npos);
	cr_assert(result.find("#test") != std::string::npos);
	cr_assert(result.find("Welcome to the test channel!") != std::string::npos);
}

Test(NumericReply, topic_with_special_chars)
{
	NumericReply reply = NumericReply::topic("alice", "#test", "Topic with :colons: and spaces");
	std::string result = reply.toString();

	cr_assert(result.find("Topic with :colons: and spaces") != std::string::npos);
}

Test(NumericReply, namReply_353)
{
	NumericReply reply = NumericReply::namReply("alice", "#test", "@alice bob charlie");
	std::string result = reply.toString();

	cr_assert(result.find("353") != std::string::npos, "RPL_NAMREPLY is 353");
	cr_assert(result.find("alice") != std::string::npos);
	cr_assert(result.find("#test") != std::string::npos);
	cr_assert(result.find("@alice bob charlie") != std::string::npos);
}

Test(NumericReply, namReply_with_operators)
{
	NumericReply reply = NumericReply::namReply("alice", "#test", "@op1 @op2 user1");
	std::string result = reply.toString();

	cr_assert(result.find("@op1 @op2 user1") != std::string::npos);
}

Test(NumericReply, endOfNames_366)
{
	NumericReply reply = NumericReply::endOfNames("alice", "#test");
	std::string result = reply.toString();

	cr_assert(result.find("366") != std::string::npos, "RPL_ENDOFNAMES is 366");
	cr_assert(result.find("alice") != std::string::npos);
	cr_assert(result.find("#test") != std::string::npos);
	cr_assert(result.find("End of") != std::string::npos);
}

// ----------------------------------------------------------------------------
// Error Replies - Nick/User (401, 431-433)
// ----------------------------------------------------------------------------

Test(NumericReply, noSuchNick_401)
{
	NumericReply reply = NumericReply::noSuchNick("alice", "ghostuser");
	std::string result = reply.toString();

	cr_assert(result.find("401") != std::string::npos, "ERR_NOSUCHNICK is 401");
	cr_assert(result.find("alice") != std::string::npos);
	cr_assert(result.find("ghostuser") != std::string::npos);
	cr_assert(result.find("No such nick") != std::string::npos);
}

Test(NumericReply, noNicknameGiven_431)
{
	NumericReply reply = NumericReply::noNicknameGiven("alice");
	std::string result = reply.toString();

	cr_assert(result.find("431") != std::string::npos, "ERR_NONICKNAMEGIVEN is 431");
	cr_assert(result.find("No nickname given") != std::string::npos);
}

Test(NumericReply, noNicknameGiven_empty_nick_uses_asterisk)
{
	NumericReply reply = NumericReply::noNicknameGiven("");
	std::string result = reply.toString();

	cr_assert(result.find("*") != std::string::npos, "Empty nick should use * placeholder");
}

Test(NumericReply, erroneusNickname_432)
{
	NumericReply reply = NumericReply::erroneusNickname("alice", "bad@nick");
	std::string result = reply.toString();

	cr_assert(result.find("432") != std::string::npos, "ERR_ERRONEUSNICKNAME is 432");
	cr_assert(result.find("alice") != std::string::npos);
	cr_assert(result.find("bad@nick") != std::string::npos);
	cr_assert(result.find("Erroneous nickname") != std::string::npos);
}

Test(NumericReply, nicknameInUse_433)
{
	NumericReply reply = NumericReply::nicknameInUse("*", "alice");
	std::string result = reply.toString();

	cr_assert(result.find("433") != std::string::npos, "ERR_NICKNAMEINUSE is 433");
	cr_assert(result.find("alice") != std::string::npos);
	cr_assert(result.find("already in use") != std::string::npos);
}

// ----------------------------------------------------------------------------
// Error Replies - Channel (403-404, 441-442, 471-475, 482)
// ----------------------------------------------------------------------------

Test(NumericReply, noSuchChannel_403)
{
	NumericReply reply = NumericReply::noSuchChannel("alice", "#nonexistent");
	std::string result = reply.toString();

	cr_assert(result.find("403") != std::string::npos, "ERR_NOSUCHCHANNEL is 403");
	cr_assert(result.find("alice") != std::string::npos);
	cr_assert(result.find("#nonexistent") != std::string::npos);
	cr_assert(result.find("No such channel") != std::string::npos);
}

Test(NumericReply, cannotSendToChan_404)
{
	NumericReply reply = NumericReply::cannotSendToChan("alice", "#moderated");
	std::string result = reply.toString();

	cr_assert(result.find("404") != std::string::npos, "ERR_CANNOTSENDTOCHAN is 404");
	cr_assert(result.find("alice") != std::string::npos);
	cr_assert(result.find("#moderated") != std::string::npos);
	cr_assert(result.find("Cannot send to channel") != std::string::npos);
}

Test(NumericReply, userNotInChannel_441)
{
	NumericReply reply = NumericReply::userNotInChannel("alice", "bob", "#test");
	std::string result = reply.toString();

	cr_assert(result.find("441") != std::string::npos, "ERR_USERNOTINCHANNEL is 441");
	cr_assert(result.find("alice") != std::string::npos);
	cr_assert(result.find("bob") != std::string::npos);
	cr_assert(result.find("#test") != std::string::npos);
	cr_assert(result.find("aren't on that channel") != std::string::npos);
}

Test(NumericReply, notOnChannel_442)
{
	NumericReply reply = NumericReply::notOnChannel("alice", "#private");
	std::string result = reply.toString();

	cr_assert(result.find("442") != std::string::npos, "ERR_NOTONCHANNEL is 442");
	cr_assert(result.find("alice") != std::string::npos);
	cr_assert(result.find("#private") != std::string::npos);
	cr_assert(result.find("not on that channel") != std::string::npos);
}

Test(NumericReply, channelIsFull_471)
{
	NumericReply reply = NumericReply::channelIsFull("alice", "#full");
	std::string result = reply.toString();

	cr_assert(result.find("471") != std::string::npos, "ERR_CHANNELISFULL is 471");
	cr_assert(result.find("alice") != std::string::npos);
	cr_assert(result.find("#full") != std::string::npos);
	cr_assert(result.find("+l") != std::string::npos, "Should mention +l mode");
}

Test(NumericReply, inviteOnlyChan_473)
{
	NumericReply reply = NumericReply::inviteOnlyChan("alice", "#private");
	std::string result = reply.toString();

	cr_assert(result.find("473") != std::string::npos, "ERR_INVITEONLYCHAN is 473");
	cr_assert(result.find("alice") != std::string::npos);
	cr_assert(result.find("#private") != std::string::npos);
	cr_assert(result.find("+i") != std::string::npos, "Should mention +i mode");
}

Test(NumericReply, badChannelKey_475)
{
	NumericReply reply = NumericReply::badChannelKey("alice", "#secret");
	std::string result = reply.toString();

	cr_assert(result.find("475") != std::string::npos, "ERR_BADCHANNELKEY is 475");
	cr_assert(result.find("alice") != std::string::npos);
	cr_assert(result.find("#secret") != std::string::npos);
	cr_assert(result.find("+k") != std::string::npos, "Should mention +k mode");
}

Test(NumericReply, chanOpPrivsNeeded_482)
{
	NumericReply reply = NumericReply::chanOpPrivsNeeded("alice", "#test");
	std::string result = reply.toString();

	cr_assert(result.find("482") != std::string::npos, "ERR_CHANOPRIVSNEEDED is 482");
	cr_assert(result.find("alice") != std::string::npos);
	cr_assert(result.find("#test") != std::string::npos);
	cr_assert(result.find("operator") != std::string::npos);
}

// ----------------------------------------------------------------------------
// Error Replies - Server/Protocol (409, 411-412, 451, 461-462, 464)
// ----------------------------------------------------------------------------

Test(NumericReply, noOrigin_409)
{
	NumericReply reply = NumericReply::noOrigin("alice");
	std::string result = reply.toString();

	cr_assert(result.find("409") != std::string::npos, "ERR_NOORIGIN is 409");
	cr_assert(result.find("alice") != std::string::npos);
	cr_assert(result.find("No origin") != std::string::npos);
}

Test(NumericReply, noRecipient_411)
{
	NumericReply reply = NumericReply::noRecipient("alice", "PRIVMSG");
	std::string result = reply.toString();

	cr_assert(result.find("411") != std::string::npos, "ERR_NORECIPIENT is 411");
	cr_assert(result.find("alice") != std::string::npos);
	cr_assert(result.find("No recipient") != std::string::npos);
	cr_assert(result.find("PRIVMSG") != std::string::npos);
}

Test(NumericReply, noTextToSend_412)
{
	NumericReply reply = NumericReply::noTextToSend("alice");
	std::string result = reply.toString();

	cr_assert(result.find("412") != std::string::npos, "ERR_NOTEXTTOSEND is 412");
	cr_assert(result.find("alice") != std::string::npos);
	cr_assert(result.find("No text to send") != std::string::npos);
}

Test(NumericReply, notRegistered_451)
{
	NumericReply reply = NumericReply::notRegistered("alice");
	std::string result = reply.toString();

	cr_assert(result.find("451") != std::string::npos, "ERR_NOTREGISTERED is 451");
	cr_assert(result.find("not registered") != std::string::npos);
}

Test(NumericReply, notRegistered_empty_nick_uses_asterisk)
{
	NumericReply reply = NumericReply::notRegistered("");
	std::string result = reply.toString();

	cr_assert(result.find("*") != std::string::npos, "Empty nick should use * placeholder");
}

Test(NumericReply, needMoreParams_461)
{
	NumericReply reply = NumericReply::needMoreParams("alice", "JOIN");
	std::string result = reply.toString();

	cr_assert(result.find("461") != std::string::npos, "ERR_NEEDMOREPARAMS is 461");
	cr_assert(result.find("alice") != std::string::npos);
	cr_assert(result.find("JOIN") != std::string::npos);
	cr_assert(result.find("Not enough parameters") != std::string::npos);
}

Test(NumericReply, alreadyRegistered_462)
{
	NumericReply reply = NumericReply::alreadyRegistered("alice");
	std::string result = reply.toString();

	cr_assert(result.find("462") != std::string::npos, "ERR_ALREADYREGISTRED is 462");
	cr_assert(result.find("alice") != std::string::npos);
	cr_assert(result.find("reregister") != std::string::npos);
}

Test(NumericReply, passwordMismatch_464)
{
	NumericReply reply = NumericReply::passwordMismatch("alice");
	std::string result = reply.toString();

	cr_assert(result.find("464") != std::string::npos, "ERR_PASSWDMISMATCH is 464");
	cr_assert(result.find("Password") != std::string::npos);
}

Test(NumericReply, passwordMismatch_empty_nick_uses_asterisk)
{
	NumericReply reply = NumericReply::passwordMismatch("");
	std::string result = reply.toString();

	cr_assert(result.find("*") != std::string::npos, "Empty nick should use * placeholder");
}

// ----------------------------------------------------------------------------
// Edge Cases
// ----------------------------------------------------------------------------

Test(NumericReply, channel_name_with_special_chars)
{
	NumericReply reply = NumericReply::noSuchChannel("alice", "#test-channel_123");
	std::string result = reply.toString();

	cr_assert(result.find("#test-channel_123") != std::string::npos);
}

Test(NumericReply, ampersand_channel)
{
	NumericReply reply = NumericReply::noSuchChannel("alice", "&local");
	std::string result = reply.toString();

	cr_assert(result.find("&local") != std::string::npos);
}

Test(NumericReply, long_nickname)
{
	NumericReply reply = NumericReply::welcome("verylongnickname");
	std::string result = reply.toString();

	cr_assert(result.find("verylongnickname") != std::string::npos);
}

Test(NumericReply, topic_with_empty_string)
{
	NumericReply reply = NumericReply::topic("alice", "#test", "");
	std::string result = reply.toString();

	cr_assert(result.find("332") != std::string::npos);
	cr_assert(result.find("#test") != std::string::npos);
}

Test(NumericReply, namReply_empty_list)
{
	NumericReply reply = NumericReply::namReply("alice", "#empty", "");
	std::string result = reply.toString();

	cr_assert(result.find("353") != std::string::npos);
	cr_assert(result.find("#empty") != std::string::npos);
}
