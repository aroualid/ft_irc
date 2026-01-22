#include "modes/InviteOnlyMode.hpp" 

bool InviteOnlyMode::apply(IChannel* channel, bool set, const std::string& param, IClient* setter)
{
	(void)param;
    (void)setter;

    channel->setInviteOnly(set);
    return true;
}

bool InviteOnlyMode::validateParam(const std::string& param) const
{
    (void)param;
    return true;
}

