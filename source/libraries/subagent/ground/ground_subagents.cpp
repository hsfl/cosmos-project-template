#include "ground_subagents.h"

namespace ProjectName
{
    namespace Ground
    {
        namespace SubAgent
        {
            // Externs declared in obc_subagents.h and defined here
            NODE_ID_TYPE node0_node_id = 0;
            NODE_ID_TYPE ground_node_id = 0;
            // thread file_thread;
            // Cosmos::Module::FileModule* file_module;
            thread websocket_thread;
            Cosmos::Module::WebsocketModule* websocket_module;

            int32_t init_subagents(Agent *agent, string remote_address)
            {
                int32_t iretn = 0;

                // Look up the node ids for nodes that will be referred to later
                node0_node_id = lookup_node_id(agent->cinfo, "node0");
                ground_node_id = lookup_node_id(agent->cinfo, "ground");

                // Start threads

                // File subagent
                // For file transfers
                // {
                //     file_module = new Cosmos::Module::FileModule();
                //     iretn = file_module->Init(agent, { "node0" });
                //     if (iretn < 0)
                //     {
                //         printf("%f FILE: Init Error - Not Starting Loop: %s\n",agent->uptime.split(), cosmos_error_string(iretn).c_str());
                //         fflush(stdout);
                //     }
                //     else
                //     {
                //         file_thread = thread([=] { file_module->Loop(); });
                //         secondsleep(3.);
                //         printf("%f FILE: Thread started\n", agent->uptime.split());
                //         fflush(stdout);
                //     }
                //     // Set radios to use and in the order of the use priority, highest to lowest
                //     uint8_t COMM = agent->channel_number("COMM");
                //     file_module->set_radios({COMM});
                // }

                // Websocket subagent
                // For communicating with PacketComm packets with websockets
                {
                    websocket_module = new Cosmos::Module::WebsocketModule(Cosmos::Module::WebsocketModule::PacketizeFunction::Raw, Cosmos::Module::WebsocketModule::PacketizeFunction::Raw);
                    iretn = websocket_module->Init(agent, remote_address, 10070, 10071, "COMM");
                    if (iretn < 0)
                    {
                        printf("%f COMM: Init Error - Not Starting Loop: %s\n",agent->uptime.split(), cosmos_error_string(iretn).c_str());
                        fflush(stdout);
                    }
                    else
                    {
                        websocket_thread = std::thread([=] { websocket_module->Loop(); });
                        secondsleep(3.);
                        printf("%f COMM: Thread started\n", agent->uptime.split());
                        fflush(stdout);
                    }
                }

                printf("All threads started\n");
                fflush(stdout);

                return 0;
            }
        }
    }
}
