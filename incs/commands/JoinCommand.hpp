#pragma once

#include "ACommand.hpp"

class JoinCommand : public ACommand
{
private:
	void joinSingleChannel(IClient* client, const std::string& channelName, const std::string& key);

	JoinCommand(IServer& server);

	void doExecute(IClient* client, const Message& message);

public:
	virtual ~JoinCommand();

	bool requiresRegistration() const
	{
		return true;
	}

	std::size_t minParams() const
	{
		return 1;
	}

	const char* getName() const
	{
		return "JOIN";
	}

	static ACommand* create(IServer& server);
};
