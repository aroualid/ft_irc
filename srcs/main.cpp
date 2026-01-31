#include "Logger.hpp"
#include "bot/SixSevenBot.hpp"
#include "bot/NielBot.hpp"
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

#ifdef DEBUG_MODE
	Logger::getInstance().setMinLevel(Logger::DEBUG);
#else
	Logger::getInstance().setMinLevel(Logger::NOTICE);
#endif

	try
	{
		Config cfg = Config::checkArgs(argc, argv);
		Server srv(cfg);

		#ifdef BONUS
		SixSevenBot *sixSevenBot = new SixSevenBot(srv);
		srv.registerBot(sixSevenBot);
		sixSevenBot->joinChannel("#eighty-nine");
	  
    MiaouBot *miaouBot = new MiaouBot(srv);
	  srv.registerBot(miaouBot);
		miaouBot->joinChannel("#cat");

		NielBot *nielBot = new NielBot(srv);
		srv.registerBot(nielBot);
		nielBot->joinChannel("#42");
		#endif
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
