#include "Logger.hpp"
#include "core/Server.hpp"
#include <csignal>
#include <cstdlib>
#include "bot/MiaouBot.hpp"

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

	Logger::getInstance().setMinLevel(Logger::DEBUG);

	try
	{
		Config cfg = Config::checkArgs(argc, argv);
		Server srv(cfg);	
	    MiaouBot* bot = new MiaouBot(srv, "larry");
	    srv.registerBot(bot);
		srv.run();
		return (0);
	}

	catch (const std::exception& e)
	{
		LOG_CRITICAL << "Something went TERRIBLY wrong: " << e.what();
		Logger::getInstance().log((Logger::Level)153)
			<< "Bailing out, you are on your own. Good luck" << std::endl;
		return (1);
	}
}
