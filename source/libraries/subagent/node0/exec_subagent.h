#ifndef NODE0_EXEC_SUBAGENT_H
#define NODE0_EXEC_SUBAGENT_H

#include "support/configCosmos.h"
#include "agent/agentclass.h"
#include "agent/command_queue.h"

namespace ProjectName
{
    namespace Node0
    {
        namespace SubAgent
        {
            ////////////////////////////////////
            // Exec Subagent
            ////////////////////////////////////
            class Exec
            {
            public:
                Exec();
                //! Initializes the exec subagent
                int32_t Init(Agent *agent);
                //! The main loop of the thread
                void Loop();
                //! A reference to the parent agent
                Agent *agent;
                //! The channel number of this subagent's channel
                int32_t mychannel = 0;
                //! Commands are queued here to be run when conditions are met
                CommandQueue cmd_queue;
                //! Commands to be executed immediately are stored here
                string immediate_dir;
                //! Any temporary files for the exec subagent are saved in here
                string temp_dir;

            private:
                //! List of events
                vector<eventstruc> eventdict;

                //! For handling packets for this thread
                void handle_channel_packet(PacketComm& packet);

                //! Saves time to a file for updating system clock on reboot
                void save_time();
                
            };


            ////////////////////////////////////
            // Static stuff
            ////////////////////////////////////

            // Externs defined in the .cpp file
            //! Thread for exec subagent
            extern thread exec_thread;
            extern Exec* exec_subagent;

            // EXEC agent requests
            int32_t request_get_queue_size(string &request, string &response, Agent* agent);
            int32_t request_get_event(string &request, string &response, Agent* agent);
            int32_t request_get_command(string &request, string &response, Agent* agent);
            int32_t request_del_command(string &request, string &response, Agent* agent);
            int32_t request_del_command_id(string &request, string &response, Agent* agent);
            int32_t request_add_command(string &request, string &response, Agent* agent);
            int32_t request_save_command(string &request, string &response, Agent *);
        }
    }
}

#endif // NODE0_EXEC_SUBAGENT_H
