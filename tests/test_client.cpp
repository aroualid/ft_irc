#include "core/Channel.hpp"
#include "mocks/Client.hpp"
#include "mocks/Server.hpp"
#include "mocks/MessageBuffer.hpp"
#include <criterion/criterion.h>

TestSuite(Client);

Test(Client, fd)
{
	Server server(6667, "testpass");
	ClientMock client1(3, "localhost", server);
	cr_assert(client1.getFd() == 3);
}

Test(Client, Nick)
{
	Server server(6667, "testpass");
	ClientMock client1(3, "localhost", server);
	client1.setNickname(std::string("Didier"));
	cr_assert(client1.getNickname() == std::string("Didier"));
}

Test(Client, noNick)
{
	Server server(6667, "testpass");
	ClientMock client1(3, "localhost", server);
	cr_assert(client1.getNickname() != std::string("Didier"));
}

Test(Client, User)
{
	Server server(6667, "testpass");
	ClientMock client1(3, "localhost", server);
	client1.setUsername(std::string("Didier"));
	cr_assert(client1.getUsername() == std::string("Didier"));
}

Test(Client, noUser)
{
	Server server(6667, "testpass");
	ClientMock client1(3, "localhost", server);
	cr_assert(client1.getUsername() != std::string("Didier"));
}

Test(Client, Real)
{
	Server server(6667, "testpass");
	ClientMock client1(3, "localhost", server);
	client1.setRealname(std::string("Didier"));
	cr_assert(client1.getRealname() == std::string("Didier"));
}

Test(Client, noReal)
{
	Server server(6667, "testpass");
	ClientMock client1(3, "localhost", server);
	cr_assert(client1.getRealname() != std::string("Didier"));
}

Test(Client, Host)
{
	Server server(6667, "testpass");
	ClientMock client1(3, "localhost", server);
	cr_assert(client1.getHostname() == std::string("localhost"));
}

Test(Client, noHost)
{
	Server server(6667, "testpass");
	ClientMock client1(3, "localhost", server);
	cr_assert(client1.getHostname() != std::string("Didier"));
}

Test(Client, Password)
{
	Server server(6667, "testpass");
	ClientMock client1(3, "localhost", server);
	client1.setPasswordProvided(true);
	cr_assert(client1.isPasswordProvided());
}

Test(Client, noPassword)
{
	Server server(6667, "testpass");
	ClientMock client1(3, "localhost", server);
	cr_assert(!client1.isPasswordProvided());
}

Test(Client, Auth)
{
	Server server(6667, "testpass");
	ClientMock client1(3, "localhost", server);
	client1.setPasswordProvided(true);
	cr_assert(client1.isPasswordProvided());
}

Test(Client, noAuth)
{
	Server server(6667, "testpass");
	ClientMock client1(3, "localhost", server);
	cr_assert(!client1.isPasswordProvided());
}

Test(Client, Registerd)
{
	Server server(6667, "testpass");
	ClientMock client1(3, "localhost", server);
	client1.setPasswordProvided(true);
	client1.setUsername(std::string("Didier"));
	client1.setNickname(std::string("Didier"));
	client1.setPasswordProvided(true);
	cr_assert(client1.isRegistered());
}

Test(Client, noRegisterd)
{
	Server server(6667, "testpass");
	ClientMock client1(3, "localhost", server);
	client1.setUsername(std::string("Didier"));
	client1.setPasswordProvided(true);
	cr_assert(!client1.isRegistered());
}

Test(Client, seePrefix)
{
	Server server(6667, "testpass");
	ClientMock client1(3, "localhost", server);
	client1.setUsername(std::string("Didier"));
	client1.setNickname(std::string("Michel"));
	client1.setPasswordProvided(true);
	cr_assert(client1.getPrefix() ==
			  std::string(client1.getNickname() + "!" + client1.getUsername() + "@localhost"));
}