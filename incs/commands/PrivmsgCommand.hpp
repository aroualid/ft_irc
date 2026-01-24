#pragma once

#include "commands/ACommand.hpp"

class PrivmsgCommand : public ACommand
{
private:
	void sendToTarget(IClient* client, const std::string& target, const std::string& text);

	PrivmsgCommand(IServer& server);

	void doExecute(IClient* client, const Message& message);

public:
	virtual ~PrivmsgCommand();

	bool requiresRegistration() const
	{
		return true;
	}

	std::size_t minParams() const
	{
		return 2;
	}

	const char* getName() const
	{
		return "PRIVMSG";
	}

	static ACommand* create(IServer& server);
};
