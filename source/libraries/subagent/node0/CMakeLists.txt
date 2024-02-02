set(LIBNAME ObcSubAgents)

FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
FILE(GLOB INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)


##################################################################
# Add Library - Install List
##################################################################
add_library(ObcSubAgents STATIC ${SOURCES} ${INCLUDES})
target_link_libraries(ObcSubAgents CosmosAgent CosmosPacket CosmosModule CosmosTransfer CosmosCommand)

if(${DEPLOY_SOURCE_FILES_TO_REMOTE} MATCHES "yes")
    add_deployment_source_directory(${CMAKE_CURRENT_SOURCE_DIR} "src")
endif()

message("Build ${LIBNAME} from ${CMAKE_CURRENT_SOURCE_DIR}")
