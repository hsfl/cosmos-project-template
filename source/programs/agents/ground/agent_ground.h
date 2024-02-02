#ifndef GROUND_AGENT_H
#define GROUND_AGENT_H

#include "support/configCosmos.h"
#include "agent/agentclass.h"

#include "support/beacon.h"
#include "support/packetcomm.h"
#include "support/packethandler.h"

#include "utils/common.h"
#include "utils/utils.h"
#include "subagent/ground/ground_subagents.h"

namespace ProjectName
{
    namespace Ground
    {
        namespace Agent
        {
            Cosmos::Support::Agent *agent;

            // Flags, flipped on triggering conditions. Set these.
            //! Flag to check if communication is active between ground.
            bool comm_connected_flag = false;
            // Events states, flipped on certain combinations of flags. Not directly set.
            //! State of connection to ground.
            bool comm_connected_state = false;

            //! Opens a socket to send telemetry to the InfluxDB database
            socket_channel cosmos_web_telegraf_channel_dev;
            //! Port in Telegraf that routes to InfluxDB
            const int TELEGRAF_PORT_DEV = 10095;
            //! Hostname of Telegraf container
            const string COSMOS_WEB_ADDR = "cosmos_telegraf";
            //! Address of Telegraf instance. Use if running locally, or change to IP address of where Telegraf is running.
            // const string COSMOS_WEB_ADDR = "127.0.0.1";

            //! Class that helps handle incoming packets
            PacketHandler packethandler;

            /**
             * @brief Initializes the agent
             *
             * @param argv Command line arguments from main()
             * @param node_name Name to use for this node
             * @param debug Debug level to default to when this agent is run
             */
            void init_agent(char *argv[], string node_name, uint16_t debug=0);

            //! Loop of the main thread
            void Loop();

            //! Updates flags and events
            void check_events();

            //! Periodically ping inactive nodes to get a response
            void ping_inactive_nodes();

            //! Callback function for when the comm_connected event becomes active or inactive
            void on_comm_connected_event_switch(bool active);

            //! An override of PacketHandler's DecodeBeacon function for sending telemetry to database
            int32_t DecodeBeacon(PacketComm& packet, string &response, Cosmos::Support::Agent* agent);

            // Agent requests
            // An example agent request for reference
            int32_t example_agent_request(string& request, string& response, Cosmos::Support::Agent* agent);
        }
    }
}

#endif // GROUND_AGENT_H
