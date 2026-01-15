#pragma once

#include "ACommand.hpp"
#include "IClient.hpp"
#include "IServer.hpp"
#include "protocol/Message.hpp"
#include <cstddef>

class PartCommand : public ACommand
{
private:
	PartCommand(IServer& server);

public:
	virtual ~PartCommand();

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
		return 1;
	}
	std::string getName() const
	{
		return "PART";
	}

	static ACommand* create(IServer& server);
};
