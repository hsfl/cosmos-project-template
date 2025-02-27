#include "agent_obc.h"

using namespace ProjectName::Node0::Agent;
using namespace ProjectName::Node0::SubAgent;
using namespace ProjectName::Common;

int main(int argc, char *argv[])
{
    handle_cmd_line_args(argc, argv);

    ////////////////////////////////////
    // Initialization
    ////////////////////////////////////
    // Initialize agent
    init_agent("node0");
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
    exec_thread.join();
    websocket_thread.join();
    agent->shutdown();

    return 0;
}

////////////////////////////////////
// Main Loop
////////////////////////////////////
void ProjectName::Node0::Agent::Loop()
{
    int32_t iretn = 0;
    // Reuse packet object
    PacketComm packet;

    // Start performing the body of the agent
    while(agent->running())
    {
        // These happen periodically
        check_events();
        update_telemetry();
        send_beacons();

        // Comm - Internal
        // Handle packets in the main-thread queue (0)
        while ((iretn = agent->channel_pull(0, packet)) > 0)
        {
            agent->monitor_unwrapped(0, packet, "Pull");
            // Handle if the packet destination is this node
            if (packet.header.nodedest == node0_node_id)
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
            // Otherwise, if the packet destination is the ground, forward it there
            else if (packet.header.nodedest == ground_node_id)
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
void ProjectName::Node0::Agent::check_events()
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

void ProjectName::Node0::Agent::update_telemetry()
{
    static ElapsedTime timer;
    // Updating telemetry every 5 seconds
    if (timer.split() < 5.)
    {
        return;
    }
    // Let main thread handle cpu and disk telemetry
    double mjd = currentmjd();
    agent->cinfo->node.utc = mjd;
    agent->cinfo->node.deci = decisec(agent->cinfo->node.utc);
    agent->cinfo->devspec.cpu[cpu_didx].utc = mjd;
    agent->cinfo->devspec.cpu[cpu_didx].uptime = deviceCpu.getUptime();
    agent->cinfo->devspec.cpu[cpu_didx].load = static_cast <float>(deviceCpu.getLoad());
    agent->cinfo->devspec.cpu[cpu_didx].gib = static_cast <float>(deviceCpu.getVirtualMemoryUsed()/1073741824.);
    dinfo = deviceDisk.getInfo();
    for (uint16_t i=0; i<dinfo.size(); ++i)
    {
        if (dinfo[i].mount == "/")
        {
            if (dinfo[i].size)
            {
                agent->cinfo->devspec.cpu[cpu_didx].storage = dinfo[i].used / static_cast<float>(dinfo[i].size);
            }
            else
            {
                agent->cinfo->devspec.cpu[cpu_didx].storage = 0.;
            }
        }
    }
    timer.reset();
}

void ProjectName::Node0::Agent::send_beacons()
{
    static ElapsedTime timer;
    // Send a beacon every 5 seconds
    if (timer.split() < 5.)
    {
        return;
    }
    // Beacons to be sent to ground if a connection is established.
    if (comm_connected_state)
    {
        PacketHandler::QueueBeacon(static_cast<uint8_t>(Beacon::TypeId::ADCSStateBeacon), 1, agent, ground_node_id, "COMM");
        PacketHandler::QueueBeacon(static_cast<uint8_t>(Beacon::TypeId::CPUBeacon), 1, agent, ground_node_id, "COMM");
        PacketHandler::QueueBeacon(static_cast<uint8_t>(Beacon::TypeId::TsenBeacon), 1, agent, ground_node_id, "COMM");
    }
    timer.reset();
}

////////////////////////////////////
// Various events or timed events
////////////////////////////////////
void ProjectName::Node0::Agent::on_comm_connected_event_switch(bool active)
{
    // If switch becomes inactive
    if (!active)
    {
        PacketHandler::QueueTransferRadio(agent->channel_number("COMM"), false, agent, node0_node_id);
        return;
    }

    // If switch becomes active
    PacketHandler::QueueTransferRadio(agent->channel_number("COMM"), true, agent, node0_node_id);
    return;
}

////////////////////////////////////
// Initialization functions
////////////////////////////////////
void ProjectName::Node0::Agent::handle_cmd_line_args(int argc, char *argv[])
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
        cout << "  -h, --help              Display this help message" << endl;
        cout << "  -d, --debug             Enable debug printing" << endl;
        cout << "  -r, --remote ADDRESS    Address or hostname of the flight agent" << endl;
        exit(0);
    }
}
void ProjectName::Node0::Agent::init_agent(string node_name)
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

    // Set channels
    ProjectName::Common::add_channels(agent);

    get_mission_elapsed_time(node_name);
    load_initial_info();

    // Initialize the packethandler, which helps handle and route packets
    packethandler.init(agent);

    agent->cinfo->agent0.aprd = 1;
    agent->start_active_loop();
}

void ProjectName::Node0::Agent::get_mission_elapsed_time(string node_name)
{
    // Information for Mission Elapsed Time
    string cosmospath = get_nodedir(node_name, true);
    FILE* fp = fopen((cosmospath + "/initial_date").c_str(), "r");
    calstruc date;
    if (fp != nullptr)
    {
        printf("Reading initial date\n");
        fflush(stdout);
        fscanf(fp, "%02d%02d%02d%02d%04d%*c%02d\n", &date.month, &date.dom, &date.hour, &date.minute, &date.year, &date.second);
        fclose(fp);
        agent->cinfo->node.utcstart = (cal2mjd(date));
    }
    else
    {
        printf("Setting initial date\n");
        fflush(stdout);
        date = mjd2cal(currentmjd());
        agent->cinfo->node.utcstart = (cal2mjd(date));
        fp = fopen((cosmospath + "/initial_date").c_str(), "w");
        fprintf(fp, "%02d%02d%02d%02d%04d.59\n", date.month, date.dom, date.hour, date.minute, date.year);
        fclose(fp);
    }
}

void ProjectName::Node0::Agent::load_initial_info()
{
    // OBC Devices
    string devname = "obc_cpu";
    int32_t iretn = json_finddev(agent->cinfo, devname);
    if (iretn < 0)
    {
        agent->debug_log.Printf("Error loading %s information - %s\n", devname.c_str(), cosmos_error_string(iretn).c_str());
        agent->shutdown();
        exit(iretn);
    }
    cpu_didx = iretn;

    agent->cinfo->devspec.cpu[cpu_didx].name = deviceCpu.getHostName();
    agent->cinfo->devspec.cpu[cpu_didx].utc = currentmjd();
    agent->cinfo->devspec.cpu[cpu_didx].uptime = deviceCpu.getUptime();
    agent->cinfo->devspec.cpu[cpu_didx].boot_count = deviceCpu.getBootCount();
    agent->cinfo->devspec.cpu[cpu_didx].load = static_cast <float>(deviceCpu.getLoad());
    agent->cinfo->devspec.cpu[cpu_didx].gib = static_cast <float>(deviceCpu.getVirtualMemoryUsed()/1073741824.);
    agent->cinfo->devspec.cpu[cpu_didx].maxgib = static_cast <float>(deviceCpu.getVirtualMemoryTotal()/1073741824.);
    agent->cinfo->devspec.cpu[cpu_didx].maxload = deviceCpu.getCpuCount();
    deviceCpu.numProcessors = agent->cinfo->devspec.cpu[cpu_didx].maxload;

    devname = "obc_disk";
    iretn = json_finddev(agent->cinfo, devname);
    if (iretn < 0)
    {
        agent->debug_log.Printf("Error loading %s information - %s\n", devname.c_str(), cosmos_error_string(iretn).c_str());
        agent->shutdown();
        exit(iretn);
    }
    uint16_t disk_didx = iretn;

    vector <DeviceDisk::info> dinfo = deviceDisk.getInfo();
    for (uint16_t i=0; i<dinfo.size(); ++i)
    {
        if (dinfo[i].mount == "/")
        {
            agent->cinfo->devspec.cpu[cpu_didx].storage = dinfo[i].used;
            agent->cinfo->devspec.disk[disk_didx].path = dinfo[i].mount.c_str();
        }
    }

    agent->cinfo->devspec.cpu[cpu_didx].utc = currentmjd();

    // Record agent version
    iretn = json_finddev(agent->cinfo, "agent_obc_version");
    if (iretn >= 0)
    {
        agent->cinfo->devspec.telem[iretn].vuint16 = AGENT_OBC_VERSION;
    }
}