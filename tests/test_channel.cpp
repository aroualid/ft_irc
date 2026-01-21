#include "core/Channel.hpp"
#include "mocks/Client.hpp"
#include "mocks/MessageBuffer.hpp"

#include <criterion/criterion.h>

TestSuite(Channel);

// addMember
Test(Channel, newUser)
{
	cr_log_info("Testing Channel addMember");
	std::string feur = "didier";
	Channel canal(feur);
	Client client1(3, "localhost");
	Client client2(4, "localhost");
	canal.setKey("bonjour");
	client1.setNickname(std::string("Didier"));	
	client2.setNickname(std::string("Didier"));	
	client1.setUsername(std::string("Didier"));	
	client2.setUsername(std::string("Didier"));	
	client1.setPasswordProvided(true);
	client2.setPasswordProvided(true);
	canal.addMember(&client2, "bonjour");
	cr_assert(canal.addMember(&client1, "bonjour"));
}

Test(Channel, alreadyUser)
{
	cr_log_info("Testing Channel addMember");
	std::string feur = "didier";
	Channel canal(feur);
	Client client1(3, "localhost");
	canal.setKey("bonjour");
	client1.setNickname(std::string("Didier"));	
	client1.setUsername(std::string("Didier"));	
	client1.setPasswordProvided(true);
	canal.addMember(&client1, "bonjour");
	cr_assert(!canal.addMember(&client1, "bonjour"));
}

Test(Channel, falsePassword)
{
	cr_log_info("Testing Channel addMember");
	std::string feur = "didier";
	Channel canal(feur);
	Client client1(3, "localhost");
	canal.setKey("bonjour");
	client1.setPasswordProvided(true);
	client1.setNickname(std::string("Didier"));	
	client1.setUsername(std::string("Didier"));	
	cr_assert(!canal.addMember(&client1, "test"));
}


Test(Channel, goodPassword)
{
	cr_log_info("Testing Channel addMember");
	std::string feur = "didier";
	Channel canal(feur);
	Client client1(3, "localhost");
	canal.setKey("bonjour");
	client1.setPasswordProvided(true);
	client1.setNickname(std::string("Didier"));	
	client1.setUsername(std::string("Didier"));	
	cr_assert(canal.addMember(&client1, "bonjour"));
}


Test(Channel, noUserLimit)
{
	cr_log_info("Testing Channel addMember");
	std::string feur = "didier";
	Channel canal(feur);
	Client client1(3, "localhost");
	Client client2(4, "localhost");
	Client client3(5, "localhost");
	Client client4(6, "localhost");
	canal.setKey("bonjour");
	client1.setPasswordProvided(true);
	client2.setPasswordProvided(true);
	client3.setPasswordProvided(true);
	client4.setPasswordProvided(true);
	client1.setNickname(std::string("Didier"));	
	client2.setNickname(std::string("Didier"));	
	client3.setNickname(std::string("Didier"));	
	client4.setNickname(std::string("Didier"));	
	client1.setUsername(std::string("Didier"));	
	client2.setUsername(std::string("Didier"));	
	client3.setUsername(std::string("Didier"));	
	client4.setUsername(std::string("Didier"));	
	canal.addMember(&client2, "bonjour");
	canal.addMember(&client3, "bonjour");
	canal.addMember(&client4, "bonjour");
	cr_assert(canal.addMember(&client1, "bonjour"));
}

Test(Channel, userLimitIsOn )
{
	cr_log_info("Testing Channel addMember");
	std::string feur = "didier";
	Channel canal(feur);
	Client client1(3, "localhost");
	Client client2(4, "localhost");
	Client client3(5, "localhost");
	Client client4(6, "localhost");
	canal.setKey("bonjour");
	client1.setPasswordProvided(true);
	client2.setPasswordProvided(true);
	client3.setPasswordProvided(true);
	client4.setPasswordProvided(true);
	client1.setNickname(std::string("Didier"));	
	client2.setNickname(std::string("Didier"));	
	client3.setNickname(std::string("Didier"));	
	client4.setNickname(std::string("Didier"));	
	client1.setUsername(std::string("Didier"));	
	client2.setUsername(std::string("Didier"));	
	client3.setUsername(std::string("Didier"));	
	client4.setUsername(std::string("Didier"));	
	canal.setUserLimit(3);
	canal.addMember(&client2, "bonjour");
	canal.addMember(&client3, "bonjour");
	canal.addMember(&client4, "bonjour");
	cr_assert(!canal.addMember(&client1, "bonjour"));
}

Test(Channel, userIsInvite)
{
	cr_log_info("Testing Channel addMember");
	std::string feur = "didier";
	Channel canal(feur);
	Client client1(3, "localhost");
	canal.setKey("bonjour");
	client1.setPasswordProvided(true);
	canal.setInviteOnly(true);
	canal.addInvite(&client1);
	client1.setNickname(std::string("Didier"));	
	client1.setUsername(std::string("Didier"));	
	cr_assert(canal.addMember(&client1, "bonjour"));
}

Test(Channel, userIsNoInvite)
{
	cr_log_info("Testing Channel addMember");
	std::string feur = "didier";
	Channel canal(feur);
	Client client1(3, "localhost");
	canal.setKey("bonjour");
	client1.setPasswordProvided(true);
	canal.setInviteOnly(true);
	client1.setNickname(std::string("Didier"));	
	client1.setUsername(std::string("Didier"));	
	cr_assert(!canal.addMember(&client1, "bonjour"));
}


Test(Channel, HasUser)
{
	std::string feur = "didier";
	Channel canal(feur);
	Client client1(3, "localhost");
	canal.setKey("bonjour");
	client1.setPasswordProvided(true);
	client1.setNickname(std::string("Didier"));	
	client1.setUsername(std::string("Didier"));	
	canal.addMember(&client1, "bonjour");
	cr_assert(canal.hasMember(&client1));
}

Test(Channel, HasRemoveUser)
{
	std::string feur = "didier";
	Channel canal(feur);
	Client client1(3, "localhost");
	canal.setKey("bonjour");
	client1.setPasswordProvided(true);
	client1.setNickname(std::string("Didier"));	
	client1.setUsername(std::string("Didier"));	
	canal.addMember(&client1, "bonjour");
	canal.removeMember(&client1);
	cr_assert(!canal.hasMember(&client1));
}

Test(Channel, applyModeNoOp)
{
	std::string feur = "didier";
	Channel canal(feur);
	Client client1(3, "localhost");
	canal.setKey("bonjour");
	client1.setPasswordProvided(true);
	canal.addMember(&client1, "bonjour");
	cr_assert(!canal.applyMode('o', "true", "", &client1));
}

Test(Channel, applyFalseMode)
{
	std::string feur = "didier";
	Channel canal(feur);
	Client client1(3, "localhost");
	canal.setKey("bonjour");
	client1.setPasswordProvided(true);
	client1.setNickname(std::string("Didier"));	
	client1.setUsername(std::string("Didier"));	
	canal.addMember(&client1, "bonjour");
	canal.addOperator(&client1);
	cr_assert(!canal.applyMode('z', "true", "", &client1));
}

Test(Channel, applyModeWithoutParam)
{
	std::string feur = "didier";
	Channel canal(feur);
	Client client1(3, "localhost");
	canal.setKey("bonjour");
	client1.setNickname(std::string("Didier"));	
	client1.setUsername(std::string("Didier"));	
	client1.setPasswordProvided(true);
	canal.addMember(&client1, "bonjour");
	canal.addOperator(&client1);
	cr_assert(!canal.applyMode('l', "true", "", &client1));
}


Test(Channel, applyModeOp)
{
	std::string feur = "didier";
	Channel canal(feur);
	Client client1(3, "localhost");
	canal.setKey("bonjour");
	client1.setNickname(std::string("Didier"));	
	client1.setUsername(std::string("Didier"));	
	client1.setPasswordProvided(true);
	canal.addMember(&client1, "bonjour");
	canal.addOperator(&client1);
	cr_assert(canal.applyMode('o', "true", "", &client1));
}

Test(Channel, applyTrueMode)
{
	std::string feur = "didier";
	Channel canal(feur);
	Client client1(3, "localhost");
	canal.setKey("bonjour");
	client1.setNickname(std::string("Didier"));	
	client1.setPasswordProvided(true);
	client1.setUsername(std::string("Didier"));	
	canal.addMember(&client1, "bonjour");
	canal.addOperator(&client1);
	cr_assert(canal.applyMode('t', "true", "", &client1));
}

Test(Channel, applyModeWithParam)
{
	std::string feur = "didier";
	Channel canal(feur);
	Client client1(3, "localhost");
	client1.setNickname(std::string("Didier"));	
	client1.setPasswordProvided(true);
	client1.setUsername(std::string("Didier"));	
	canal.setKey("bonjour");
	canal.addMember(&client1, "bonjour");
	canal.addOperator(&client1);
	cr_assert(canal.applyMode('l', "true", "12", &client1));
}


Test(Channel, getModeStringAll)
{
	std::string feur = "didier";
	Channel canal(feur);
	canal.setInviteOnly(true);
	canal.setTopicRestricted(true);
	canal.setUserLimit(12);
	canal.setKey("bonjour");
	cr_assert(canal.getModeString() == std::string("+itkl bonjour 12"));
}

Test(Channel, getModeStringOneFalse)
{
	std::string feur = "didier";
	Channel canal(feur);
	canal.setInviteOnly(true);
	canal.setTopicRestricted(false);
	canal.setUserLimit(12);
	canal.setKey("bonjour");
	cr_assert(canal.getModeString() == std::string("+ikl bonjour 12"));
}

Test(Channel, getModeStringAllFalse)
{
	std::string feur = "didier";
	Channel canal(feur);
	canal.setInviteOnly(false);
	canal.setTopicRestricted(false);
	canal.setUserLimit(-1);
	canal.setKey("");
	cr_assert(canal.getModeString() == std::string(""));
}

Test(Channel, getMemberListNoOp)
{
	std::string feur = "didier";
	Channel canal(feur);
	Client client1(3, "localhost");
	Client client2(4, "localhost");
	Client client3(5, "localhost");
	Client client4(6, "localhost");
	canal.setKey("bonjour");
	client1.setPasswordProvided(true);
	client2.setPasswordProvided(true);
	client3.setPasswordProvided(true);
	client4.setPasswordProvided(true);
	client1.setNickname(std::string("a"));
	client2.setNickname(std::string ("b"));
	client3.setNickname(std::string ("c"));
	client4.setNickname(std::string ("d"));
	client1.setUsername(std::string("Didier"));	
	client2.setUsername(std::string("Didier"));	
	client3.setUsername(std::string("Didier"));	
	client4.setUsername(std::string("Didier"));	
	canal.addMember(&client1, "bonjour");
	canal.addMember(&client2, "bonjour");
	canal.addMember(&client3, "bonjour");
	canal.addMember(&client4, "bonjour");
	cr_assert(canal.getMemberList() == std::string("d c b @a"));
}

Test(Channel, getMemberListOneOp)
{
	std::string feur = "didier";
	Channel canal(feur);
	Client client1(3, "localhost");
	Client client2(4, "localhost");
	Client client3(5, "localhost");
	Client client4(6, "localhost");
	canal.setKey("bonjour");
	client1.setPasswordProvided(true);
	client2.setPasswordProvided(true);
	client3.setPasswordProvided(true);
	client4.setPasswordProvided(true);
	client1.setNickname(std::string("a"));
	client2.setNickname(std::string ("b"));
	client3.setNickname(std::string ("c"));
	client4.setNickname(std::string ("d"));
	client1.setUsername(std::string("Didier"));	
	client2.setUsername(std::string("Didier"));	
	client3.setUsername(std::string("Didier"));	
	client4.setUsername(std::string("Didier"));	
	canal.addMember(&client1, "bonjour");
	canal.addMember(&client2, "bonjour");
	canal.addMember(&client3, "bonjour");
	canal.addMember(&client4, "bonjour");
	canal.addOperator(&client2);
	cr_assert(canal.getMemberList() == std::string("d c @b @a"));
}

Test(Channel, isNotEmpty)
{
	std::string feur = "didier";
	Channel canal(feur);
	Client client1(3, "localhost");
	Client client2(4, "localhost");
	Client client3(5, "localhost");
	Client client4(6, "localhost");
	client1.setPasswordProvided(true);
	client2.setPasswordProvided(true);
	client3.setPasswordProvided(true);
	client4.setPasswordProvided(true);
	client1.setNickname(std::string("Didier"));	
	client2.setNickname(std::string("Didier"));	
	client3.setNickname(std::string("Didier"));	
	client4.setNickname(std::string("Didier"));	
	client1.setUsername(std::string("Didier"));	
	client2.setUsername(std::string("Didier"));	
	client3.setUsername(std::string("Didier"));	
	client4.setUsername(std::string("Didier"));	
	canal.setKey("bonjour");
	canal.addMember(&client1, "bonjour");
	canal.addMember(&client2, "bonjour");
	canal.addMember(&client3, "bonjour");
	canal.addMember(&client4, "bonjour");
	cr_assert(!canal.isEmpty());
}

Test(Channel, isEmpty)
{
	std::string feur = "didier";
	Channel canal(feur);
	Client client1(3, "localhost");
	Client client2(4, "localhost");
	Client client3(5, "localhost");
	Client client4(6, "localhost");
	client1.setPasswordProvided(true);
	client2.setPasswordProvided(true);
	client3.setPasswordProvided(true);
	client4.setPasswordProvided(true);
	client1.setNickname(std::string("Didier"));	
	client2.setNickname(std::string("Didier"));	
	client3.setNickname(std::string("Didier"));	
	client4.setNickname(std::string("Didier"));	
	client1.setUsername(std::string("Didier"));	
	client2.setUsername(std::string("Didier"));	
	client3.setUsername(std::string("Didier"));	
	client4.setUsername(std::string("Didier"));	
	cr_assert(canal.isEmpty());
}

Test(Channel, broadcast)
{
	std::string feur = "didier";
	Channel canal(feur);
	Client client1(3, "localhost");
	Client client2(4, "localhost");
	Client client3(5, "localhost");
	Client client4(6, "localhost");
	client1.setPasswordProvided(true);
	client2.setPasswordProvided(true);
	client3.setPasswordProvided(true);
	client4.setPasswordProvided(true);
	client1.setNickname(std::string("Didier"));	
	client2.setNickname(std::string("Didier"));	
	client3.setNickname(std::string("Didier"));	
	client4.setNickname(std::string("Didier"));	
	client1.setUsername(std::string("Didier"));	
	client2.setUsername(std::string("Didier"));	
	client3.setUsername(std::string("Didier"));	
	client4.setUsername(std::string("Didier"));	
	canal.setKey("bonjour");
	canal.addMember(&client1, "bonjour");
	canal.addMember(&client2, "bonjour");
	canal.addMember(&client3, "bonjour");
	canal.addMember(&client4, "bonjour");
	canal.broadcast(std::string("hey"), &client1);
	std::string buffer = client2.getBuffer().getWriteBuffer();
	cr_assert(client1.getBuffer().getWriteBuffer() == ""); 
	cr_assert(client2.getBuffer().getWriteBuffer() == ("hey")); 
	cr_assert(client3.getBuffer().getWriteBuffer() == "hey"); 
	cr_assert(client4.getBuffer().getWriteBuffer() == "hey"); 

}
