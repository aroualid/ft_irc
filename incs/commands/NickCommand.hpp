#pragma once

#include "ACommand.hpp"

class NickCommand : public ACommand
{
private:
	bool isValidNickname(const std::string& nickname) const;

public:
	NickCommand(Server* server);
	virtual ~NickCommand();

	void execute(Client* client, const Message& message);

	bool requiresRegistration() const
	{
		return false;
	}
	bool requiresAuthentication() const
	{
		return true;
	}
	std::size_t minParams() const
	{
		return 1;
	}
	std::string getName() const
	{
		return "NICK";
	}
};
