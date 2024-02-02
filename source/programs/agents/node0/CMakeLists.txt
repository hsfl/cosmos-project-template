add_executable(agent_obc agent_obc.cpp agent_obc.h)
target_link_libraries(agent_obc ObcSubAgents ProjectNameUtility ProjectNameCommon)
install(TARGETS agent_obc DESTINATION bin)
