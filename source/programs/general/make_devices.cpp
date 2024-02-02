#include "support/configCosmos.h"
#include "support/cosmos-errno.h"
#include "agent/agentclass.h"

// This program will create .ini files for the device structs that will populate the cosmosstruc for the specified node.
// Files will be placed in the node's folder in ~/cosmos/nodes/NODE_NAME/
// Make sure to run this same program to create .ini files for both the remote and ground nodes!
// If device setup files do not match, telemetry coming from the remote node may not be ingested properly by the ground node.

// If you update this file with new devices or you have removed devices, be sure to clear out the .ini files
// in the ~/cosmos/nodes/NODE_NAME/ folder first. And repeat for every other node.

// Usage: make_devices [NODE_NAME] [AGENT_NAME]
int main(int argc, char *argv[])
{
    int32_t iretn = 0;
    Agent* agent;
    if (argc == 3)
    {
        agent = new Agent("", argv[1], argv[2]);
    }
    else if (argc == 2)
    {
        agent = new Agent("", argv[1]);
    }
    else
    {
        agent = new Agent();
    }

    // Placeholder
    iretn = json_createpiece(agent->cinfo, "base_telem_none", DeviceType::TELEM);
    agent->cinfo->devspec.telem[json_finddev(agent->cinfo, "base_telem_none")].vtype = JSON_TYPE_NONE;

    // Miscellaneous
    iretn = json_createpiece(agent->cinfo, "agent_obc_version", DeviceType::TELEM);
    agent->cinfo->devspec.telem[json_finddev(agent->cinfo, "agent_obc_version")].vtype = JSON_TYPE_UINT16;

    // OBC
    iretn = json_createpiece(agent->cinfo, "obc_cpu", DeviceType::CPU);
    iretn = json_createpiece(agent->cinfo, "obc_disk", DeviceType::DISK);

    json_dump_node(agent->cinfo);



    FILE *fp = fopen((get_cosmosnodes() + "/nodeids.ini").c_str(), "w");
    if (fp != nullptr)
    {
        fprintf(fp, "1 ground\n");
        fprintf(fp, "10 node0\n");
        fclose(fp);
        printf("nodesids.ini place in %s\n", get_cosmosnodes().c_str());
    }
    else
    {
        printf("Could not place nodeids.ini in %s\n", get_cosmosnodes().c_str());
    }
}
