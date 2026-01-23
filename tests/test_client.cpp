#include "mocks/Client.hpp"
#include "mocks/MessageBuffer.hpp"
#include <criterion/criterion.h>
#include "core/Channel.hpp"


TestSuite(Client);

Test(Client, fd)
{
	ClientMock client1(3, "localhost");
	cr_assert(client1.getFd() == 3);
}

Test(Client, Nick)
{
	ClientMock client1(3, "localhost");
	client1.setNickname(std::string("Didier"));	
	cr_assert(client1.getNickname() == std::string("Didier"));
}

Test(Client, noNick)
{
ClientMock client1(3, "localhost");
	cr_assert(client1.getNickname() != std::string("Didier"));
}

Test(Client, User)
{
	ClientMock client1(3, "localhost");
	client1.setUsername(std::string("Didier"));	
	cr_assert(client1.getUsername() == std::string("Didier"));
}

Test(Client, noUser)
{
	ClientMock client1(3, "localhost");
	cr_assert(client1.getUsername() != std::string("Didier"));
}

Test(Client, Real)
{
	ClientMock client1(3, "localhost");
	client1.setRealname(std::string("Didier"));	
	cr_assert(client1.getRealname() == std::string("Didier"));
}

Test(Client, noReal)
{
	ClientMock client1(3, "localhost");
	cr_assert(client1.getRealname() != std::string("Didier"));
}

Test(Client, Host)
{
	ClientMock client1(3, "localhost");
	cr_assert(client1.getHostname() == std::string("localhost"));
}

Test(Client, noHost)
{
	ClientMock client1(3, "localhost");
	cr_assert(client1.getHostname() != std::string("Didier"));
}

Test (Client, Password)
{
	ClientMock client1(3, "localhost");
	client1.setPasswordProvided(true);
	cr_assert(client1.isPasswordProvided());
}

Test (Client, noPassword)
{
	ClientMock client1(3, "localhost");
	cr_assert(!client1.isPasswordProvided());
}

Test (Client, Auth)
{
	ClientMock client1(3, "localhost");
	client1.setPasswordProvided(true);
	cr_assert(client1.isPasswordProvided());
}

Test (Client, noAuth)
{
	ClientMock client1(3, "localhost");
	cr_assert(!client1.isPasswordProvided());
}


Test (Client, Registerd)
{
	ClientMock client1(3, "localhost");
	client1.setPasswordProvided(true);
	client1.setUsername(std::string("Didier"));	
	client1.setNickname(std::string("Didier"));	
	client1.setPasswordProvided(true);
	cr_assert(client1.isRegistered());
}

Test (Client, noRegisterd)
{
	ClientMock client1(3, "localhost");
	client1.setUsername(std::string("Didier"));	
	client1.setPasswordProvided(true);
	cr_assert(!client1.isRegistered());
}

Test (Client, seePrefix)
{
	ClientMock client1(3, "localhost");
	client1.setUsername(std::string("Didier"));	
	client1.setNickname(std::string("Michel"));	
	client1.setPasswordProvided(true);
	cr_assert(client1.getPrefix() == std::string(client1.getNickname() + "!" + client1.getUsername() + "@localhost"));
}