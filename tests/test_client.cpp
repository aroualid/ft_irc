#include "mocks/Client.hpp"
#include "mocks/MessageBuffer.hpp"
#include <criterion/criterion.h>
#include "core/Channel.hpp"


TestSuite(Client);

Test(Client, fd)
{
	Client client1(3, "localhost");
	cr_assert(client1.getFd() == 3);
}

Test(Client, Nick)
{
	Client client1(3, "localhost");
	client1.setNickname(std::string("Didier"));	
	cr_assert(client1.getNickname() == std::string("Didier"));
}

Test(Client, noNick)
{
Client client1(3, "localhost");
	cr_assert(client1.getNickname() != std::string("Didier"));
}

Test(Client, User)
{
	Client client1(3, "localhost");
	client1.setUsername(std::string("Didier"));	
	cr_assert(client1.getUsername() == std::string("Didier"));
}

Test(Client, noUser)
{
	Client client1(3, "localhost");
	cr_assert(client1.getUsername() != std::string("Didier"));
}

Test(Client, Real)
{
	Client client1(3, "localhost");
	client1.setRealname(std::string("Didier"));	
	cr_assert(client1.getRealname() == std::string("Didier"));
}

Test(Client, noReal)
{
	Client client1(3, "localhost");
	cr_assert(client1.getRealname() != std::string("Didier"));
}

Test(Client, Host)
{
	Client client1(3, "localhost");
	cr_assert(client1.getHostname() == std::string("localhost"));
}

Test(Client, noHost)
{
	Client client1(3, "localhost");
	cr_assert(client1.getHostname() != std::string("Didier"));
}

Test (Client, Password)
{
	Client client1(3, "localhost");
	client1.setPasswordProvided(true);
	cr_assert(client1.isPasswordProvided());
}

Test (Client, noPassword)
{
	Client client1(3, "localhost");
	cr_assert(!client1.isPasswordProvided());
}

Test (Client, Auth)
{
	Client client1(3, "localhost");
	client1.setPasswordProvided(true);
	cr_assert(client1.isAuthenticated());
}

Test (Client, noAuth)
{
	Client client1(3, "localhost");
	cr_assert(!client1.isAuthenticated());
}


Test (Client, Registerd)
{
	Client client1(3, "localhost");
	client1.setPasswordProvided(true);
	client1.setUsername(std::string("Didier"));	
	client1.setNickname(std::string("Didier"));	
	client1.setPasswordProvided(true);
	cr_assert(client1.isRegistered());
}

Test (Client, noRegisterd)
{
	Client client1(3, "localhost");
	client1.setUsername(std::string("Didier"));	
	client1.setPasswordProvided(true);
	cr_assert(!client1.isRegistered());
}

Test (Client, seePrefix)
{
	Client client1(3, "localhost");
	client1.setUsername(std::string("Didier"));	
	client1.setNickname(std::string("Michel"));	
	client1.setPasswordProvided(true);
	cr_assert(client1.getPrefix() == std::string(client1.getNickname() + "!" + client1.getUsername() + "@localhost"));
}
