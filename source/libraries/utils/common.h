#ifndef PROJECTNAME_COMMON_H
#define PROJECTNAME_COMMON_H

#include "agent/agentclass.h"

namespace ProjectName
{
    namespace Common
    {
        /**
         * @brief Adds the shared channels of the realm to the agent
         *
         * @param agent Pointer to parent agent
         */
        void add_channels(Cosmos::Support::Agent* agent);

        //! Typedef for a callback function to call when a switch becomes active
        typedef void (*on_event_switch)(bool active);

        /**
         * @brief Primitive triggering of an event based on flags
         *
         * @param flags Pointers to flag bools
         * @param event_state Event state that is determined by the flags
         * @param event Callback to run only once when the event state changes. Argument of the callback is the new state.
         */
        void fire_event(const std::vector<bool>& flags, bool& event_state, on_event_switch event);
    }
}

#endif // PROJECTNAME_COMMON_H
