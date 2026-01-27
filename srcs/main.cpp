#include "core/Server.hpp"
#include <csignal>
#include <cstdlib>

volatile sig_atomic_t g_shutdown = 0;

void signalHandler(int signum)
{
	(void)signum;
	g_shutdown = 1;
}

int main(int argc, char** argv)
{
	std::signal(SIGINT, signalHandler);
	std::signal(SIGTERM, signalHandler);

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
