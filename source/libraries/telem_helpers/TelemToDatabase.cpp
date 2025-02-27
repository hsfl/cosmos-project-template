#include "TelemToDatabase.h"
#include "physics/simulatorclass.h"
#include "support/jsonlib.h"
#include "support/timelib.h"

int32_t TelemToDatabase::initialize(string cosmos_web_addr, cosmosstruc* cinfo)
{
    int32_t iretn =  open_cosmos_web_sockets(cosmos_web_addr);
    if (iretn < 0)
    {
        return iretn;
    }
    sockets_initialized = true;
    reset_db(cinfo);
    return 0;
}

/**
 * @brief Sends a single node's telems to cosmos web
 * 
 * @param cinfo cosmosstruc of some node to store telems for
 * @param beacon Beacon object containing the telemetry
 * @return int32_t 0 on success, negative on failure
 */
int32_t TelemToDatabase::send_telem_to_cosmos_web(cosmosstruc* cinfo, const Beacon& beacon)
{
    if (!sockets_initialized)
    {
        return COSMOS_GENERAL_ERROR_NOTSTARTED;
    }

    // Silly workaround until a better way of knowing who the other end is is implemented
    string node_name = "node";
    for (auto node : cinfo->realm.node_ids)
    {
        if (node.first != cinfo->node.name)
        {
            node_name = node.first;
            break;
        }
    }

    switch(beacon.type)
    {
    case Beacon::TypeId::ADCSStateBeacon:
        {
            json11::Json jobj = json11::Json::object({
                {"node_name", node_name },
                {"node_loc", json11::Json::object({
                    {"pos", json11::Json::object({
                        {"eci", json11::Json::object({
                            { "utc", cinfo->node.loc.pos.eci.utc },
                            { "s", cinfo->node.loc.pos.eci.s },
                            { "v", cinfo->node.loc.pos.eci.v }
                        })}
                    })},
                    {"att", json11::Json::object({
                        {"icrf", json11::Json::object({
                            { "utc", cinfo->node.loc.att.icrf.utc },
                            { "s", cinfo->node.loc.att.icrf.s },
                            { "v", cinfo->node.loc.att.icrf.v }
                        })}
                    })}
                })},
            });
            int32_t iretn = socket_sendto(cosmos_web_telegraf_channel, jobj.dump());
            if (iretn < 0) { return iretn; }
        }
        break;
    // TODO:
    // Other beacon types not handles yet(?)
    // This whole thing needs to exist elsewhere
    default:
        break;
    }

    return 0;
}

/**
 * @brief Reset the database
 */
void TelemToDatabase::reset_db(cosmosstruc* cinfo)
{
    if (!sockets_initialized)
    {
        return;
    }
    socket_sendto(cosmos_web_api_channel, "{\"swchstruc\": true, \"battstruc\": true, \"bcregstruc\": true, \"cpustruc\": true, \"device\": true, \"device_type\": true, \"locstruc\": true, \"magstruc\": true, \"node\": true, \"tsenstruc\": true, \"rwstruc\": true, \"mtrstruc\": true, \"attstruc_icrf\": true, \"cosmos_event\": true, \"event_type\": true, \"gyrostruc\": true, \"locstruc_eci\": true, \"target\": true, \"cosmos_event\": true }");

    // Silly workaround until a better way of knowing who the other end is is implemented
    uint16_t id = 0;
    string node_name = "node";
    for (auto node : cinfo->realm.node_ids)
    {
        if (node.first != cinfo->node.name)
        {
            node_name = node.first;
            id = node.second;
            break;
        }
    }

    // Repopulate node table
    json11::Json jobj = json11::Json::object({
        {"node", json11::Json::object({
            { "node_id", id },
            { "node_name", node_name },
            { "node_type", cinfo->node.type },
            { "agent_name", cinfo->agent0.name },
            { "utc", cinfo->node.utc },
            { "utcstart", cinfo->node.utcstart }
        })}
    });
    socket_sendto(cosmos_web_telegraf_channel, jobj.dump());
    // Add device info
    jobj = json11::Json::object({
        {"node_name", node_name },
        {"device", [id, cinfo]() -> vector<json11::Json>
            {
                vector<json11::Json> ret;
                for (auto device : cinfo->device) {
                    ret.push_back({
                        json11::Json::object({
                            { "type", device->type },
                            { "cidx", device->cidx },
                            { "didx", device->didx },
                            { "name", device->name }
                        })
                    });
                }
                return ret;
            }()
        }
    });
    socket_sendto(cosmos_web_telegraf_channel, jobj.dump());
    // Groundstations & targets
    jobj = json11::Json::object({
        {"target", [cinfo]() -> vector<json11::Json>
            {
                vector<json11::Json> ret;
                uint16_t target_id = 0;
                for (auto target : cinfo->target) {
                    ret.push_back({
                        json11::Json::object({
                            { "id"  , target_id++ },
                            { "name", target.name },
                            { "type", target.type },
                            { "lat" , target.loc.pos.geod.s.lat },
                            { "lon" , target.loc.pos.geod.s.lon },
                            { "h" , target.loc.pos.geod.s.h },
                            { "area" , target.area }
                        })
                    });
                }
                return ret;
            }()
        }
    });
    socket_sendto(cosmos_web_telegraf_channel, jobj.dump());
    // A bit silly, but reset requires some wait time at the moment 
    // cout << "Resetting db..." << endl;
    secondsleep(1.);
}

int32_t TelemToDatabase::open_cosmos_web_sockets(string cosmos_web_addr)
{
    // No telems are to be emitted if no address was specified
    if (cosmos_web_addr.empty())
    {
        return COSMOS_GENERAL_ERROR_NOTSTARTED;
    }
    string ipv4_address = cosmos_web_addr;
    // If no . or : are found, then it may be a hostname instead, in which case convert to an ipv4 address
    if (cosmos_web_addr.find(".") == std::string::npos && cosmos_web_addr.find(":") == std::string::npos)
    {
        string response;
        int32_t iretn = hostnameToIP(cosmos_web_addr, ipv4_address, response);
        if (iretn < 0)
        {
            cout << "Encountered error in hostnameToIP: " << response << endl;
            exit(0);
        }
    }

    int32_t iretn = socket_open(&cosmos_web_telegraf_channel, NetworkType::UDP, ipv4_address.c_str(), TELEGRAF_PORT2, SOCKET_TALK, SOCKET_BLOCKING, 2000000 );
    if ((iretn) < 0)
    {
        cout << "Failed to open socket cosmos_web_telegraf_channel: " << cosmos_error_string(iretn) << endl;
        exit(0);
    }
    iretn = socket_open(&cosmos_web_api_channel, NetworkType::UDP, ipv4_address.c_str(), API_PORT, SOCKET_TALK, SOCKET_BLOCKING, 2000000);
    if ((iretn) < 0)
    {
        cout << "Failed to open socket cosmos_web_api_channel: " << cosmos_error_string(iretn) << endl;
        exit(0);
    }
    return iretn;
}
