#ifndef PROJECTNAME_UTILITY_H
#define PROJECTNAME_UTILITY_H

#include <vector>
#include "support/socketlib.h"

namespace ProjectName
{
    namespace Utils
    {
        // Opens sockets for the database
        int32_t open_cosmos_web_socket(socket_channel& cosmos_web_telegraf_channel_dev, string cosmos_web_addr, int TELEGRAF_PORT_DEV);
    }
}

#endif // PROJECTNAME_UTILITY_H
