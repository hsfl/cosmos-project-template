set(LIBNAME GroundSubAgents)

FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
FILE(GLOB INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)


##################################################################
# Add Library - Install List
##################################################################
add_library(GroundSubAgents STATIC ${SOURCES} ${INCLUDES})
target_link_libraries(GroundSubAgents CosmosAgent CosmosPacket CosmosModule CosmosTransfer)

if(${DEPLOY_SOURCE_FILES_TO_REMOTE} MATCHES "yes")
    add_deployment_source_directory(${CMAKE_CURRENT_SOURCE_DIR} "src")
endif()

message("Build ${LIBNAME} from ${CMAKE_CURRENT_SOURCE_DIR}")
