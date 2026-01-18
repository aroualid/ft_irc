#pragma once

#include "commands/ACommand.hpp"

class NoticeCommand : public ACommand
{
private:
	void sendToTarget(IClient* client, const std::string& target, const std::string& text);

	NoticeCommand(IServer& server);

public:
	virtual ~NoticeCommand();

	// override execute to skip error replies (NOTICE never sends errors per RFC)
	void execute(IClient* client, const Message& message);
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
		return "NOTICE";
	}

	static ACommand* create(IServer& server);
};
