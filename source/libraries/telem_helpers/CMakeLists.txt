set(LIBNAME TelemHelpers)

FILE(GLOB INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)


##################################################################
# Add Library - Install List
##################################################################
add_library(TelemHelpers STATIC ${SOURCES} ${INCLUDES})
target_link_libraries(TelemHelpers CosmosTime CosmosNetwork CosmosNamespace CosmosBeacon)

install(TARGETS TelemHelpers DESTINATION lib)

message("Build ${LIBNAME} from ${CMAKE_CURRENT_SOURCE_DIR}")
