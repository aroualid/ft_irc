#pragma once

#include "core/IClient.hpp"
#include "core/IServer.hpp"
#include "protocol/Message.hpp"
#include "protocol/NumericReply.hpp"
#include <string>

class IChannel;

class ACommand
{
private:
	ACommand(const ACommand& source);
	ACommand& operator=(const ACommand& other);

protected:
	IServer& m_server;

	void sendReply(IClient* client, const NumericReply& reply);
	bool validateParamCount(IClient* client, const Message& message, size_t min);

	static std::vector< std::string > splitByComma(const std::string& str);
	static bool isChannelName(const std::string& name);

	Message buildMessage(IClient* sender, const std::string& target, const std::string& text) const;
	void broadcastToChannel(IChannel* channel, IClient* sender, const Message& message);
	void sendToClient(IClient* target, IClient* sender, const Message& message);

	virtual void doExecute(IClient* client, const Message& message) = 0;

	ACommand(IServer& server);

public:
	virtual ~ACommand();

	virtual void execute(IClient* client, const Message& message);

	virtual bool requiresRegistration() const
	{
		return true;
	};

	virtual bool requiresAuthentication() const
	{
		return true;
	};

	virtual std::size_t minParams() const
	{
		return 0;
	};

	virtual std::string getName() const = 0;
};
