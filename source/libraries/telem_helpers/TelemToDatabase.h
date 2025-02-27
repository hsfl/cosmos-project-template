#ifndef TELEMTODATABASE_H
#define TELEMTODATABASE_H

//*****************************************************
// A simple helper class for sending data to cosmos web

#include "support/beacon.h"
#include "support/jsondef.h"
#include "support/socketlib.h"

class TelemToDatabase
{
public:
    TelemToDatabase() {}

    /**
     * @brief Opens the ports for sending to cosmos web.
     * 
     * @param cosmos_web_addr Either a hostname or IPv4 address
     * @param cinfo Pointer to cosmosstruc of node
     * @return int32_t 0 on success, negative on error
     */
    int32_t initialize(string cosmos_web_addr, cosmosstruc* cinfo);
    void reset_db(cosmosstruc* cinfo);
    int32_t send_telem_to_cosmos_web(cosmosstruc* cinfo, const Beacon& beacon);

private:
    //! Socket object for access to telemetry endpoint
    socket_channel cosmos_web_telegraf_channel;
    //! Socket object for access to API for various other tasks
    socket_channel cosmos_web_api_channel;

    //! Internal use, set after sockets have been opened
    bool sockets_initialized = false;
    //! Port to telegraf telemetry endpoint, don't change
    int TELEGRAF_PORT2 = 10096;
    //! Port to telegraf API endpoint for backend, don't change
    int API_PORT = 10097;

    int32_t open_cosmos_web_sockets(string cosmos_web_addr);
};


#endif // TELEMTODATABASE_H
