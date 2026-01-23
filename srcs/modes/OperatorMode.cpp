#include "modes/OperatorMode.hpp"
#include "core/IChannel.hpp"

bool OperatorMode::apply(IChannel* channel, bool set, const std::string& param, IClient* setter)
{
	(void)setter;
	IClient* client = channel->getMemberByNickname(param);

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
