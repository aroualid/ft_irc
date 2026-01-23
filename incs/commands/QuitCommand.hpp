#pragma once

#include "IClient.hpp"
#include "IServer.hpp"
#include "commands/ACommand.hpp"

class QuitCommand : public ACommand
{
private:
	QuitCommand(IServer& server);

	void broadcastQuitToChannels(IClient* client, const std::string& quitMessage) const;

public:
	virtual ~QuitCommand();

	void doExecute(IClient* client, const Message& message);

	bool requiresRegistration() const
	{
		return false;
	}
	bool requiresAuthentication() const
	{
		return false;
	}
	std::size_t minParams() const
	{
		return 0;
	}
	std::string getName() const
	{
		return "QUIT";
	}

	static ACommand* create(IServer& server);
};
