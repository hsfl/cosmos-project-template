#ifndef OBC_AGENT_H
#define OBC_AGENT_H

#include "support/configCosmos.h"
#include "agent/agentclass.h"

#include "device/cpu/devicecpu.h"
#include "device/disk/devicedisk.h"
#include "support/beacon.h"
#include "support/packetcomm.h"
#include "support/packethandler.h"

#include "utils/common.h"
#include "subagent/node0/obc_subagents.h"

namespace ProjectName
{
    namespace Node0
    {
        namespace Agent
        {
            //! Version number. Use high byte for major version, low byte for minor version.
            const uint16_t AGENT_OBC_VERSION = 0x0100;

            //! The main agent object to use for this program.
            Cosmos::Support::Agent *agent;
            //! Agent logging type. Use 2 for debugging, 0 to disable.
            uint16_t debug_level = 0;
            //! Purely for debug log purposes.
            string file_name_arg0 = "";

            // Flags, flipped on triggering conditions. Set these.
            //! Flag to check if communication is active between ground.
            bool comm_connected_flag = false;
            // Events states, flipped on certain combinations of flags. Not directly set.
            //! State of connection to ground.
            bool comm_connected_state = false;

            // Various classes to collect cpu and disk telemetry
            DeviceDisk deviceDisk;
            DeviceCpu deviceCpu;
            vector <DeviceDisk::info> dinfo;
            // Device indexes for telemetry updating
            //! Main CPU didx
            uint16_t cpu_didx = 0;

            //! Address or hostname of the ground agent
            string remote_address = "127.0.0.1";

            //! Class that helps handle incoming packets
            PacketHandler packethandler;

            /**
             * @brief Handle command line arguments
             *
             * @param argv argc from main
             * @param argv Command line arguments from main()
             */
            void handle_cmd_line_args(int argc, char *argv[]);

            /**
             * @brief Initializes the agent
             *
             * @param node_name Name to use for this node
             */
            void init_agent(string node_name);
            //! Reads in initial date from file to calculate mission elapsed time
            void get_mission_elapsed_time(string node_name);
            //! Loads in some initial info into the cosmosstruc when program first starts
            void load_initial_info();

            //! Loop of the main thread
            void Loop();

            //! Updates flags and events
            void check_events();

            //! Periodically sends beacons
            void send_beacons();

            //! Telemetry managed by the main loop
            void update_telemetry();

            //! Callback function for when the comm_connected event becomes active or inactive
            void on_comm_connected_event_switch(bool active);
        }
    }
}

#endif // OBC_AGENT_H
