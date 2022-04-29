/********************************************************************
* Copyright (C) 2015 by Interstel Technologies, Inc.
*   and Hawaii Space Flight Laboratory.
*
* This file is part of the COSMOS/core that is the central
* module for COSMOS. For more information on COSMOS go to
* <http://cosmos-project.com>
*
* The COSMOS/core software is licenced under the
* GNU Lesser General Public License (LGPL) version 3 licence.
*
* You should have received a copy of the
* GNU Lesser General Public License
* If not, go to <http://www.gnu.org/licenses/>
*
* COSMOS/core is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3 of
* the License, or (at your option) any later version.
*
* COSMOS/core is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* Refer to the "licences" folder for further information on the
* condititons and terms to use this software.
********************************************************************/

// TODO: change include paths so that we can make reference to cosmos using a full path
// example
// #include <cosmos/core/agent/agentclass.h>

#include "agent/agentclass.h"
#include "support/configCosmos.h"
#include "agent/agentclass.h"
#include "device/serial/serialclass.h"
#include <iostream>
#include <fstream>

using namespace std;
Agent *agent;
ofstream file;


int main(int argc, char** argv)
{
    cout << "Agent Template" << endl;
    Agent *agent;
    string nodename = "cubesat1";
    string agentname = "template"; //name of the agent that the request is directed to

    agent = new Agent(nodename, agentname);

    // add state of health (soh)
    string soh = "{\"device_batt_current_000\"}";

    // examples
    // SOH for IMU temperature
    soh = "{\"device_imu_temp_000\"}";

    // SOH for location information (utc, position in ECI, attitude in ICRF)
    soh = "{\"node_loc_utc\","
          "\"node_loc_pos_eci\","
          "\"node_loc_att_icrf\"}" ;

    // TODO: create append function for soh to make it easier to add strings
    // example: soh.append("node_loc_pos_eci");

    // set the soh string
    agent->set_sohstring(soh.c_str());

    Convert::cartpos pos_eci;
    //    agent->set_sohstring(soh);

    // Start executing the agent
    while(agent->running())
    {
        // Start executing the agent
        pos_eci.utc = currentmjd(0);
        agent->cinfo->node.loc.pos.eci = pos_eci;
//        agent->cinfo->device[0].imu.temp = 123;

        //sleep for 1 sec
        COSMOS_SLEEP(0.1);
    }


    return 0;
}

