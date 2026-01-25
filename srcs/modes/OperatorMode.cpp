#include "modes/OperatorMode.hpp"
#include "core/IChannel.hpp"
#include "core/IServer.hpp"

bool OperatorMode::apply(IChannel* channel, bool set, const std::string& param, IClient* setter)
{
	IServer* serv = setter->getServer();
	IClient* client = serv->getClientByNickname(param);

	if (channel->hasMember(client))
	{
		if (set)
		{
			if (!channel->isOperator(client))
				channel->addOperator(client);
			else
				return false;
		}
		else if (!set)
		{
			if (channel->isOperator(client))
				channel->removeOperator(client);
			else
				return false;
		}
	}
	else
		return false;
	return true;
}

bool OperatorMode::validateParam(const std::string& param) const
{
	(void)param;
	return true;
}
