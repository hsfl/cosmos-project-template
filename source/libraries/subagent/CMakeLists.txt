# No need to build ground agent for rpi deployment.
# Comment out the if check in the case where you do
# intend to deploy the ground agent on a rpi.
IF (NOT ${CROSS_TYPE} MATCHES "rpi")
    add_subdirectory(ground)
ENDIF()

# Other node agents
add_subdirectory(node0)
