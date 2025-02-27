#ifndef NODE0_SENSOR_SUBAGENT_H
#define NODE0_SENSOR_SUBAGENT_H

#include "support/configCosmos.h"
#include "agent/agentclass.h"
#include "agent/command_queue.h"

namespace ProjectName
{
    namespace Node0
    {
        namespace SubAgent
        {
            /////////////////////////////////////////
            // Sensor Subagent
            // For requesting sensor measurements
            // from an externally running propagator
            // agent.
            /////////////////////////////////////////
            class SensorSubagent
            {
            public:
                SensorSubagent();
                //! Initializes the sensor subagent
                int32_t Init(Agent *agent);
                //! The main loop of the thread
                void Loop();
                //! A reference to the parent agent
                Agent *agent;

            private:
                //! Use agent discovery mechanism to find the propagator agent
                bool find_propagator();
                //! Requests truth data from the propagator agent
                void request_measurements();
                
                //! Agent discovery return object of the propagator agent
                beatstruc propagator_info;

                //! Sample rate of the sensor, in Hz
                double sample_rate = 1.0;
            };


            ////////////////////////////////////
            // Static stuff
            ////////////////////////////////////

            // Externs defined in obc_subagents.cpp
            //! Thread for sensor subagent
            extern thread sensor_thread;
            extern SensorSubagent* sensor_subagent;

            // Sensor subagent requests
            // int32_t request_set_position(string &request, string &response, Agent *agent);
        }
    }
}

#endif // NODE0_SENSOR_SUBAGENT_H
