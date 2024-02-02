#include "exec_subagent.h"

namespace ProjectName
{
    namespace Node0
    {
        namespace SubAgent
        {
            ProjectName::Node0::SubAgent::Exec::Exec()
            {

            }

            int32_t Exec::Init(Agent* agent)
            {
                this->agent = agent;

                // Set the immediate, incoming, outgoing, and temp directories
                immediate_dir = data_base_path(agent->getNode(), "immediate", "exec") + "/";
                if (immediate_dir.empty())
                {
                    cout<<"unable to create directory: <"<<(agent->getNode()+"/immediate")+"/exec"<<"> ... exiting."<<endl;
                    exit(1);
                }

                temp_dir = data_base_path(agent->getNode(), "temp", "exec") + "/";
                if (temp_dir.empty())
                {
                    cout<<"unable to create directory: <"<<(agent->getNode()+"/temp")+"/exec"<<"> ... exiting."<<endl;
                    exit(1);
                }

                load_dictionary(eventdict, agent->cinfo, "events.dict");

                // Event related request functions
                agent->add_request("getqueuesize", request_get_queue_size, "", "returns the current size of the command queue");
                agent->add_request("delcommand", request_del_command, "entry-string", "deletes the specified command event from the queue according to its JSON string");
                agent->add_request("delcommandid", request_del_command_id, "entry #", "deletes the specified command event from the queue according to its position");
                agent->add_request("getcommand", request_get_command, "[ entry # ]", "returns the requested command queue entry (or all if none specified)");
                agent->add_request("addcommand", request_add_command, "{\"event_name\":\"\"}{\"event_utc\":0}{\"event_utcexec\":0}{\"event_flag\":0}{\"event_type\":0}{\"event_data\":\"\"}{\"event_condition\":\"\"}", "adds the specified command event to the queue");
                agent->add_request("getevent", request_get_event, "[ entry # ]", "returns the requested event queue entry (or all if none specified)");
                agent->add_request("savecommand", request_save_command, "{file}", "Save current command list to optional file name (default: .queue)");

                // Reload existing queue
                cmd_queue.restore_commands(temp_dir);

                // Move all temp files to the outgoing/ground folder for sending to the ground
                log_move_agent_temp("ground", "exec", true);

                //! Keep the channel number of this subagent's channel for easy reuse in the loop
                mychannel = agent->channel_number("EXEC");

                return 0;
            }

            void Exec::Loop()
            {
                agent->debug_log.Printf("Starting Exec Loop\n");

                // Reused variables in the loop
                //! Used to store return values from functions
                int32_t iretn = 0;
                //! Used to store packets that are pulled off the exec channel queue
                PacketComm packet;

                while(agent->running())
                {
                    // Telemetry
                    agent->cinfo->node.utc = currentmjd();

                    // Comm - Internal
                    if ((iretn = agent->channel_pull(mychannel, packet)) > 0)
                    {
                        handle_channel_packet(packet);
                    }

                    // Handle commands in command queue
                    cmd_queue.join_event_threads();
                    cmd_queue.run_commands(agent, agent->getNode(), currentmjd());

                    save_time();

                    secondsleep(.1); //std::this_thread::yield();
                }
                return;
            }

            void Exec::handle_channel_packet(PacketComm& packet)
            {
                switch (packet.header.type)
                {
                case PacketComm::TypeId::CommandExecLoadCommand:
                    {
                        cmd_queue.load_commands(immediate_dir);
                        if (packet.data.size())
                        {
                            string incoming_dir = get_cosmosnodes() + "/" + string(packet.data.begin(), packet.data.end()) + "/incoming/exec/";
                            cmd_queue.load_commands(incoming_dir);
                        }
                    }
                    break;
                case PacketComm::TypeId::CommandExecAddCommand:
                    {
                        if (packet.data.size())
                        {
                            Event cmd;
                            cmd.set_command(string(packet.data.begin(), packet.data.end()));
                            if(cmd.is_command())
                            {
                                cmd_queue.add_command(cmd);
                            }
                        }
                    }
                    break;
                default:
                    break;
                }
            }

            void Exec::save_time()
            {
                static ElapsedTime timer;
                // Periodically save the most recent time to file,
                // so that system clock can be set to last saved time upon reboot.
                if (timer.split() < 60.)
                {
                    return;
                }
                FILE *fp = fopen((get_cosmosnodes() + agent->cinfo->node.name + "/last_date").c_str(), "w");
                if (fp != nullptr)
                {
                    timer.reset();
                    calstruc date = mjd2cal(currentmjd());
                    fprintf(fp, "%02d%02d%02d%02d%04d.59\n", date.month, date.dom, date.hour, date.minute, date.year);
                    fclose(fp);
                }
            }

            /////////////////////////
            // Agent requests
            /////////////////////////
            int32_t request_get_queue_size(string &, string &response, Agent *)
            {
                response += std::to_string(exec_subagent->cmd_queue.get_command_size());
                return 0;
            }

            int32_t request_get_event(string &request, string &response, Agent *)
            {
                std::ostringstream ss;
                if(exec_subagent->cmd_queue.get_event_size()==0)    {
                    ss << "[Empty]";
                }
                else {
                    int j;
                    int32_t iretn = sscanf(request.c_str(),"%*s %d",&j);

                    // if valid index then return event
                    if (iretn == 1) {
                        if(j >= 0 && j < static_cast<int>(exec_subagent->cmd_queue.get_event_size()))
                            ss << exec_subagent->cmd_queue.get_event(j);
                        else
                            ss << "<" << j << "> is not a valid event queue index (current range between 0 and "
                               << exec_subagent->cmd_queue.get_event_size()-1 << ")";
                    }

                    // if no index given, return the entire queue
                    else if (iretn ==  -1) {
                        for(int i = 0; i < static_cast<int>(exec_subagent->cmd_queue.get_event_size()); ++i) {
                            Event cmd = exec_subagent->cmd_queue.get_event(i);
                            ss << "[" << i << "]" << "[" << mjd2iso8601(cmd.getUtc()) << "]" << cmd << endl;
                        }
                    }
                    // if the user supplied something that couldn't be turned into an integer
                    else if (iretn == 0) { ss << "Usage:\tget_event [ index ]\t"; }
                }

                response += ss.str();
                return 0;
            }

            int32_t request_save_command(string &request, string &response, Agent *)
            {
                vector<string> args = string_split(request, " ");
                if (args.size() == 2)
                {
                    exec_subagent->cmd_queue.save_commands(exec_subagent->temp_dir, args[1]);
                }
                else
                {
                    exec_subagent->cmd_queue.save_commands(exec_subagent->temp_dir);
                }
                return 0;
            }

            int32_t request_get_command(string &request, string &response, Agent *)
            {
                std::ostringstream ss;
                if(exec_subagent->cmd_queue.get_command_size()==0)    {
                    ss << "[Empty]";
                }
                else {
                    int j;
                    int32_t iretn = sscanf(request.c_str(),"%*s %d",&j);

                    // if valid index then return command
                    if (iretn == 1) {
                        if(j >= 0 && j < static_cast<int>(exec_subagent->cmd_queue.get_command_size()))
                            ss << exec_subagent->cmd_queue.get_command(j);
                        else
                            ss << "<" << j << "> is not a valid command queue index (current range between 0 and "
                               << exec_subagent->cmd_queue.get_command_size()-1 << ")";
                    }

                    // if no index given, return the entire queue
                    else if (iretn ==  -1) {
                        for(int i = 0; i < static_cast<int>(exec_subagent->cmd_queue.get_command_size()); ++i) {
                            Event cmd = exec_subagent->cmd_queue.get_command(i);
                            ss << "[" << i << "]" << "[" << mjd2iso8601(cmd.getUtc()) << "]" << cmd << endl;
                        }
                    }
                    // if the user supplied something that couldn't be turned into an integer
                    else if (iretn == 0) { ss << "Usage:\tget_command [ index ]\t"; }
                }

                response += ss.str();
                return 0;
            }

            // Delete Queue Entry - by #
            int32_t request_del_command_id(string &request, string &response, Agent *)
            {
                Event cmd;
                std::ostringstream ss;
                if(exec_subagent->cmd_queue.get_command_size()==0)    {
                    ss << "the command queue is empty";
                }
                else {
                    int j;
                    int32_t iretn = sscanf(request.c_str(),"%*s %d",&j);

                    // if valid index then return command
                    if (iretn == 1) {
                        if(j >= 0 && j < static_cast<int>(exec_subagent->cmd_queue.get_command_size())) {
                            response += std::to_string(exec_subagent->cmd_queue.del_command(j)) + " commands deleted from the queue";
                        } else {
                            ss << "<" << j << "> is not a valid command queue index (current range between 0 and " << exec_subagent->cmd_queue.get_command_size()-1 << ")";
                        }
                    }
                    // if the user supplied something that couldn't be turned into an integer
                    else if (iretn == 0)    {
                        ss << "Usage:\tdel_command_id [ index ]\t";
                    }
                }

                response += ss.str();
                return 0;
            }

            // Delete Queue Entry - by date and contents
            int32_t request_del_command(string &request, string &response, Agent *)
            {
                Event cmd;
                string line(request);

                // Check for valid request
                if (line.find(" ") == string::npos)
                {
                    return GENERAL_ERROR_UNDEFINED;
                }

                line.erase(0, line.find(" ")+1);
                cmd.set_command(line);

                //delete command
                int n = exec_subagent->cmd_queue.del_command(cmd);

                if(!cmd.is_command()) {
                    response +=  "Not a valid command: " + line;
                }
                else {
                    response += std::to_string(n) + " commands deleted from the queue";
                }

                return 0;
            }

            // Add Queue Entry
            int32_t request_add_command(string &request, string &response, Agent *)
            {
                Event cmd;
                string line(request);

                // Check for valid request
                if (line.find(" ") == string::npos)
                {
                    return GENERAL_ERROR_UNDEFINED;
                }

                line.erase(0, line.find(" ")+1);
                cmd.set_command(line);

                // add command
                if(cmd.is_command())
                {
                    exec_subagent->cmd_queue.add_command(cmd);
                    response +=  "Command added to queue: " + line;
                }
                else {
                    response +=  "Not a valid command: " + line;
                }

                // sort the queue
                exec_subagent->cmd_queue.sort();
                return 0;
            }
        }
    }
}
