#pragma once

#include "commands/ACommand.hpp"

class PrivmsgCommand : public ACommand
{
private:
	void sendToTarget(IClient* client, const std::string& target, const std::string& text);

	PrivmsgCommand(IServer& server);

public:
	virtual ~PrivmsgCommand();

	void doExecute(IClient* client, const Message& message);

	bool requiresRegistration() const
	{
		return true;
	}
	bool requiresAuthentication() const
	{
		return true;
	}
	std::size_t minParams() const
	{
		return 2;
	}
	std::string getName() const
	{
		return "PRIVMSG";
	}

	static ACommand* create(IServer& server);
};
