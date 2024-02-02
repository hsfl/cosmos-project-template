#include "utils.h"

// Open sockets for sending data to cosmos web
int32_t ProjectName::Utils::open_cosmos_web_socket(socket_channel& cosmos_web_telegraf_channel_dev, const string cosmos_web_addr, int TELEGRAF_PORT_DEV)
{
    // No telems are to be emitted if no address was specified
    if (cosmos_web_addr.empty())
    {
        return 0;
    }
    string ipv4_addr = cosmos_web_addr;
    // Detect hostname vs IPv4 address, convert to latter if necessary
    if (cosmos_web_addr.find(".") == std::string::npos && cosmos_web_addr.find(":") == std::string::npos)
    {
        string response;
        int32_t iretn = hostnameToIP(TELEGRAF_ADDR, cosmos_web_addr, response);
        if (iretn < 0)
        {
            cout << "Encountered error in hostnameToIP: " << response << endl;
            exit(0);
        }
    }

    iretn = socket_open(&cosmos_web_telegraf_dev_channel, NetworkType::UDP, cosmos_web_addr.c_str(), TELEGRAF_PORT_DEV, SOCKET_TALK, SOCKET_BLOCKING, 2000000 );
    if ((iretn) < 0)
    {
        cout << "Failed to open socket cosmos_web_telegraf_dev_channel: " << cosmos_error_string(iretn) << endl;
        exit(0);
    }
    return iretn;
}
