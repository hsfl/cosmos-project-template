add_subdirectory(node0)
add_subdirectory(ground)

add_executable(agent "${COSMOS_SOURCE}/core/programs/agents/agent.cpp")
target_link_libraries(agent CosmosAgent)
install(TARGETS agent DESTINATION bin)
