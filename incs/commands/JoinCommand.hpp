#pragma once

#include "ACommand.hpp"
#include <vector>

class JoinCommand : public ACommand
{
private:
	bool isValidChannelName(const std::string& name) const;
	std::vector< std::string > splitByComma(const std::string& str) const;
	void joinSingleChannel(IClient* client, const std::string& channelName, const std::string& key);

	JoinCommand(IServer& server);

public:
	virtual ~JoinCommand();

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
		return 1;
	}
	std::string getName() const
	{
		return "JOIN";
	}

	static ACommand* create(IServer& server);
};
