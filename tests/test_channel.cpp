#include "core/Channel.hpp"
#include "mocks/Client.hpp"
#include <criterion/criterion.h>

TestSuite(Channel);

Test(Channel, applyMode)
{
	cr_log_info("Testing Channel applyMode");
	std::string feur = "didier";
	Channel canal(feur);
	Client client1(3, "localhost");

	didier::addMember(
}


