#include "sensor_subagent.h"
#include "support/convertlib.h"

namespace ProjectName
{
namespace Node0
{
namespace SubAgent
{
    SensorSubagent::SensorSubagent()
    {

    }

    int32_t SensorSubagent::Init(Agent* agent)
    {
        this->agent = agent;

        return 0;
    }

    void SensorSubagent::Loop()
    {
        agent->debug_log.Printf("Starting Sensor Loop\n");

        while(agent->running())
        {
            if (!find_propagator())
            {
                secondsleep(5.);
            }

            request_measurements();

            secondsleep(sample_rate);
            std::this_thread::yield();
        }
        return;
    }

    bool SensorSubagent::find_propagator()
    {
        // Return early if a previous call has already found the propagator
        // TODO: add agent activity timeout for if the propagator agent has been shutdown
        if (propagator_info.exists)
        {
            return true;
        }
        propagator_info = agent->find_agent("any", "propagate");
        if (!propagator_info.exists)
        {
            return false;
        }
        agent->debug_log.Printf("Found Propagator agent on node: %s\n", propagator_info.node.c_str());
        string response;
        agent->send_request(propagator_info, "get_node_json " + agent->cinfo->node.name, response, 1.);
        agent->cinfo->json.node = response.substr(response.find("\n")+1);
        agent->send_request(propagator_info, "get_pieces_json " + agent->cinfo->node.name, response, 1.);
        agent->cinfo->json.pieces =  response.substr(response.find("\n")+1);
        agent->send_request(propagator_info, "get_devgen_json " + agent->cinfo->node.name, response, 1.);
        agent->cinfo->json.devgen = response.substr(response.find("\n")+1);
        agent->send_request(propagator_info, "get_devspec_json " + agent->cinfo->node.name, response, 1.);
        agent->cinfo->json.devspec = response.substr(response.find("\n")+1);
        json_setup_node(agent->cinfo->json, agent->cinfo);

        return true;
    }

    void SensorSubagent::request_measurements()
    {
        // Get current position and attitude from Propagator (see propagatorv3.cpp in core/agents)
        string response;
        string estring;
        agent->send_request(propagator_info, "get_location " + agent->cinfo->node.name, response, 1.);
        vector <string> args = string_split(response, "\n");
        agent->debug_log.Printf("Measurement: %s\n", response.c_str());
        if (args.size() > 1)
        {
            json11::Json jargs = json11::Json::parse(args[1], estring);
            if (estring.empty())
            {
                if (!jargs["node"].is_null() && agent->cinfo->node.name == jargs["node"].string_value())
                {
                    if (!jargs["utcoffset"].is_null())
                    {
                        agent->cinfo->node.utcoffset = jargs["utcoffset"].number_value();
                    }
                    if (!jargs["pos"].is_null())
                    {
                        agent->cinfo->node.loc.pos.eci.from_json(jargs["pos"].dump());
                        agent->cinfo->node.loc.pos.eci.pass++;
                        Convert::pos_eci(agent->cinfo->node.loc);
                    }
                    if (!jargs["att"].is_null())
                    {
                        agent->cinfo->node.loc.att.icrf.from_json(jargs["att"].dump());
                    }
                }
            }
        }
        else
        {
            // No valid means propagator may have gone missing
            propagator_info.exists = false;
        }
    }
}
}
}
