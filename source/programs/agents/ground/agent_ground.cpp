#include "agent_ground.h"

using namespace ProjectName::Ground::Agent;
using namespace ProjectName::Ground::SubAgent;
using namespace ProjectName::Common;
using namespace ProjectName::Utils;

int main(int argc, char *argv[])
{
    handle_cmd_line_args(argc, argv);

    ////////////////////////////////////
    // Initialization
    ////////////////////////////////////
    // Initialize agent
    init_agent("ground");
    // Initialize subagents
    init_subagents(agent, remote_address);

    ////////////////////////////////////
    // Main Loop
    ////////////////////////////////////
    Loop();

    ////////////////////////////////////
    // Cleanup
    ////////////////////////////////////
    int32_t iretn = agent->wait(Agent::State::SHUTDOWN);
    if (iretn < 0)
    {
        printf("Error in agent->wait() %d\n", iretn);
        exit(iretn);
    }
    // file_thread.join();
    websocket_thread.join();
    agent->shutdown();

    return 0;
}

////////////////////////////////////
// Main Loop
////////////////////////////////////
void ProjectName::Ground::Agent::Loop()
{
    int32_t iretn = 0;
    // Reuse packet object
    PacketComm packet;

    // Start performing the body of the agent
    while(agent->running())
    {
        check_events();
        // ping_inactive_nodes();

        // Comm - Internal
        // Handle packets in the main-thread queue (0)
        while ((iretn = agent->channel_pull(0, packet)) > 0)
        {
            agent->monitor_unwrapped(0, packet, "Pull");
            // Handle if the packet destination is this node
            if (packet.header.nodedest == ground_node_id)
            {
                string response;
                packethandler.process(packet, response);
                // Send back a response if a response was created when handling the packet
                if (response.size())
                {
                    if (packet.header.chanin != 0)
                    {
                        agent->push_response(packet.header.chanin, 0, packet.header.nodeorig, 0, response);
                        agent->monitor_unwrapped(packet.header.chanin, packet, "Respond");
                    }
                }
            }
            // Otherwise, if the packet destination is node0, forward it there
            else if (packet.header.nodedest == node0_node_id)
            {
                agent->channel_push("COMM", packet);
            }
        }

        std::this_thread::yield();
    }
}

////////////////////////////////////
// Functions in the Main Loop
////////////////////////////////////
void ProjectName::Ground::Agent::check_events()
{
    static ElapsedTime timer;
    // Checking events every 1 second
    if (timer.split() < 1.)
    {
        return;
    }
    // Update flag status
    comm_connected_flag = (agent->channel_enabled("COMM") == 1);

    // Check event states
    fire_event({comm_connected_flag}, comm_connected_state, &on_comm_connected_event_switch);

    timer.reset();
}

void ProjectName::Ground::Agent::ping_inactive_nodes()
{
    static ElapsedTime timer;
    // Send a ping every 10 seconds if no communication has been active
    if (comm_connected_state || timer.split() < 10.)
    {
        return;
    }
    PacketHandler::QueuePing(agent, node0_node_id, "COMM", "COMM");
    timer.reset();
}

////////////////////////////////////
// Various events or timed events
////////////////////////////////////
void ProjectName::Ground::Agent::on_comm_connected_event_switch(bool active)
{
    // If switch becomes inactive
    if (!active)
    {
        PacketHandler::QueueTransferRadio(agent->channel_number("COMM"), false, agent, ground_node_id);
        return;
    }

    // If switch becomes active
    PacketHandler::QueueTransferRadio(agent->channel_number("COMM"), true, agent, ground_node_id);
    return;
}

////////////////////////////////////
// Agent requests
////////////////////////////////////
//! This is a sample agent request, all agent requests have these params
//! \param request The string passed to the agent when this agent request was called
//! \param response The string shown to the user when the agent request returns
//! \param agent A pointer to this agent
//! \return 0
int32_t ProjectName::Ground::Agent::example_agent_request(string& request, string& response, Support::Agent* agent)
{
    // The first element is always the name of the agent request,
    // any additional arguments to the agent request will be elements 1 and beyond
    std::vector<string> args = string_split(request);
    cout << "I am " << agent->cinfo->node.name << ":" << agent->cinfo->agent0.name << endl;
    cout << "Agent request called: " << args[0] << endl;
    cout << "Number of arguments: " << args.size() << endl;
    cout << "Arguments:" << endl;
    for (size_t i = 0; i < args.size(); ++i)
    {
        cout << std::setw(4) << std::to_string(i) << ": "<< args[i] << endl;
    }
    cout << endl;

    // Response from agent request
    response = "This is the response message";

    return 0;
}

////////////////////////////////////
// Initialization functions
////////////////////////////////////
void ProjectName::Ground::Agent::handle_cmd_line_args(int argc, char *argv[])
{
    bool display_help = false;
    file_name_arg0 = argv[0];
    for (size_t i=1; i < argc; ++i)
    {
        string arg = argv[i];
        if (arg == "-h" || arg == "--help")
        {
            display_help = true;
            break;
        }
        else if (arg == "-d" || arg == "--debug")
        {
            debug_level = 2;
        }
        else if ((arg == "-r" || arg == "--remote") && i+1 < argc)
        {
            remote_address = argv[++i];
        }
        else if ((arg == "-w" || arg == "--cosmos_web_addr") && i+1 < argc)
        {
            cosmos_web_addr = argv[++i];
        }
        else
        {
            cout << "Error parsing argument: " << arg << endl;
            display_help = true;
            break;
        }
    }
    if (display_help)
    {
        cout << "Usage: " << argv[0] << " [options]" << endl;
        cout << "Options:" << endl;
        cout << "  -h, --help                       Display this help message" << endl;
        cout << "  -d, --debug                      Enable debug printing" << endl;
        cout << "  -r, --remote ADDRESS             Address or hostname of the flight agent" << endl;
        cout << "  -w, --cosmos_web_addr ADDRESS    Address or hostname of the Telegraf instance" << endl;
        exit(0);
    }
}

void ProjectName::Ground::Agent::init_agent(string node_name)
{
    // A Realm can be thought of as a project.
    // A Node can be thought of as a distinct entity capable of independent action in the scope of the project.
    // An Agent is one of potentially many processes running on the node.
    agent = new Support::Agent("demo", node_name, "main", 0., 10000, false, 0, NetworkType::UDP, debug_level);

    // Check if agent was successfully started
    int32_t iretn = 0;
    if ((iretn = agent->wait()) < 0) {
        agent->debug_log.Printf("%16.10f %s Failed to start Agent %s on Node %s Dated %s : %s\n",currentmjd(), mjd2iso8601(currentmjd()).c_str(), agent->getAgent().c_str(), agent->getNode().c_str(), utc2iso8601(data_ctime(file_name_arg0)).c_str(), cosmos_error_string(iretn).c_str());
        exit(iretn);
    } else {
        agent->debug_log.Printf("%16.10f %s Started Agent %s on Node %s Dated %s\n",currentmjd(), mjd2iso8601(currentmjd()).c_str(), agent->getAgent().c_str(), agent->getNode().c_str(), utc2iso8601(data_ctime(file_name_arg0)).c_str());
    }

    // TODO: not ideal but must be done here to visualize correctly
    agent->cinfo->node.type = NODE_TYPE_SATELLITE;

    // Open up a socket for sending beacon data to Telegraf
    open_cosmos_web_socket(cosmos_web_telegraf_channel_dev, cosmos_web_addr, TELEGRAF_PORT_DEV);
    telemHelper.initialize(cosmos_web_addr, agent->cinfo);
    // TODO: probably not desireable in the long run to put this here
    telemHelper.reset_db(agent->cinfo);

    // Set channels
    add_channels(agent);


    // Add agent requests
    agent->add_request("example_req", example_agent_request, "example_req [args...]", "Prints out any number of args you pass to this");

    // Initialize the packethandler, which helps handle and route packets
    packethandler.init(agent);
    // Override default handling of incoming beacon-type packets
    packethandler.add_func(PacketComm::TypeId::DataObcBeacon, DecodeBeacon);

    agent->cinfo->agent0.aprd = 1;
    agent->start_active_loop();
}

////////////////////////////////////
// Miscellaneous
////////////////////////////////////
// Overriding PacketHandler's default handling of DecodeBeacon, for sending data to Telegraf
int32_t ProjectName::Ground::Agent::DecodeBeacon(PacketComm& packet, string &response, Support::Agent* agent)
{
    Beacon beacon;
    beacon.Init();
    // Unmarshal binary beacon data
    int32_t iretn = beacon.Decode(packet.data, agent->cinfo);
    if (iretn < 0)
    {
        return iretn;
    }

    // Convert beacon data to a json string
    iretn = beacon.EncodeJson(beacon.type, agent->cinfo, response);
    if (iretn < 0)
    {
        return iretn;
    }

    // Send json beacon data to Telegraf dev port, which will be fed into the database
    socket_sendto(cosmos_web_telegraf_channel_dev, response);

    telemHelper.send_telem_to_cosmos_web(agent->cinfo, beacon);

    return iretn;
}
