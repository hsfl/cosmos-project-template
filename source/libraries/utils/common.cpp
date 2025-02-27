#include "common.h"

void ProjectName::Common::add_channels(Cosmos::Support::Agent* agent)
{
    agent->channel_add("COMM", Support::Channel::PACKETCOMM_DATA_SIZE, Support::Channel::PACKETCOMM_PACKETIZED_SIZE, 18000., 1000);
}

void ProjectName::Common::fire_event(const std::vector<bool>& flags, bool& event_state, void (*event)(bool))
{
    bool state_active = true;
    // All flags must be true for event switch to be active
    for (const bool flag: flags)
    {
        if (!flag)
        {
            state_active = false;
            break;
        }
    }
    // If switch was flipped, fire event
    if (event_state != state_active)
    {
        event_state = state_active;
        event(state_active);
    }
}
