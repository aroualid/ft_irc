#include "modes/OperatorMode.hpp" 
#include "core/IChannel.hpp"

bool OperatorMode::apply(IChannel* channel, bool set, const std::string& param, IClient* setter)
{
    (void)param;

	if (channel->hasMember(setter))
	{
		if (set)
		{
			if (!channel->isOperator(setter))
				channel->addOperator(setter);
			else
				return false;
		}
		else if (!set)
		{
			if (channel->isOperator(setter))
				channel->removeOperator(setter);
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
