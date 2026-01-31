#pragma one

#include "bot/IBot.hpp"
#include "core/IServer.hpp"

class	NielBot : public IBot
{
	private:
		IServer* m_server;
		IClient* m_client;
		bool	contains42(const std::string& msg)const;
		void	sendReply(IChannel* channel);

	public:
		NielBot(IServer* server, IClient* botClient);
		virtual ~NielBot();

		void	onPrivateMessage(IClient* sender, const std::string& msg);
		void	onChannelMessage(IClient* sender, IChannel* channel, const std::string& msg);

		IClient* getClient();
};