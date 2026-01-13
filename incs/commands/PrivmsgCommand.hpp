#pragma once

#include "commands/ACommand.hpp"
#include <vector>

class PrivmsgCommand : public ACommand
{
private:
	std::vector< std::string > splitByComma(const std::string& str) const;
	void sendToTarget(IClient* client, const std::string& target, const std::string& text);

public:
	PrivmsgCommand(IServer& server);
	virtual ~PrivmsgCommand();

	void execute(IClient* client, const Message& message);

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
};
