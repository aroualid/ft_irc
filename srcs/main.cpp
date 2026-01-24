#include "core/Server.hpp"

int main(int argc, char** argv)
{
	try
	{
		Config cfg = Config::checkArgs(argc, argv);
		Server srv(cfg);
		srv.run();
		return (0);
	}

	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return (1);
	}
}
