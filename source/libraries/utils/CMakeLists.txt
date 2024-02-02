##################################################################
# Add Library - Install List
##################################################################
add_library(ProjectNameUtility STATIC utils.cpp utils.h)
add_library(ProjectNameCommon STATIC common.cpp common.h)

if(${DEPLOY_SOURCE_FILES_TO_REMOTE} MATCHES "yes")
    add_deployment_source_directory(${CMAKE_CURRENT_SOURCE_DIR} "src")
endif()

message("In Common from ${CMAKE_CURRENT_SOURCE_DIR}")
