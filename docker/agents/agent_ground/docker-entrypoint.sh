#!/bin/bash

# This script is the entrypoint for the demo ground agent container.
# Any arguments passed to this script will be passed to the agent.

# Set the umask to 002 so that files and directories created by the agent are modifiable by
# others in the cosmosuser (26767) group.
# This is useful when mounting volumes from the host machine to the container.
umask 002

# Pass script arguments to agent
agent_ground "$@"
