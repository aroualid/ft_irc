#pragma once

#include "IClient.hpp"
#include "IServer.hpp"
#include "commands/ACommand.hpp"

class QuitCommand : public ACommand
{
private:
	QuitCommand(IServer& server);

	void broadcastQuitToChannels(IClient* client, const std::string& quitMessage) const;

	void doExecute(IClient* client, const Message& message);

public:
	virtual ~QuitCommand();

	bool requiresRegistration() const
	{
		return false;
	}

	std::size_t minParams() const
	{
		return 0;
	}

	const char* getName() const
	{
		return "QUIT";
	}

	static ACommand* create(IServer& server);
};
