cmake_minimum_required(VERSION 3.1)
SET(PROJECT_NAME cosmos-project-template)
set (CMAKE_CXX_STANDARD 11)
project(${PROJECT_NAME})
# -----------------------------------------------
set(COSMOS_SOURCE ${PROJECT_SOURCE_DIR}/../../..)
message ("COSMOS_SOURCE: ${COSMOS_SOURCE}")

# compile on Linux host
IF(${CMAKE_HOST_SYSTEM_NAME} MATCHES "Linux")

    MESSAGE("Compiling on Linux")
    SET( "${COSMOS_SOURCE}/core/")

    # Linux specific code
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fopenmp")

    #Check for ARM Build
    IF (${CROSS_TYPE} MATCHES "arm")

        # ARM Build Detected

        # Set installation directory
        IF("$ENV{COSMOS}" STREQUAL "")
            SET(COSMOS "~/cosmos/arm")
        ELSE()
            SET(COSMOS "$ENV{COSMOS}/arm")
        ENDIF("$ENV{COSMOS}" STREQUAL "")

        # Set Toolchain File
        SET(CMAKE_TOOLCHAIN_FILE "${PROJECT_SOURCE_DIR}/build/toolchain_arm")

        # Add preprocessor definition to help source files determine code to use for ARMv7 specific implementation
        add_definitions(-DCROSS_TYPE_arm) #20150825JC: Added definition

        # Add include directories for locally compiled arm openssl
        include_directories(${PROJECT_SOURCE_DIR}/thirdparty/arm/include)

    ELSE()

        # Non-ARM Build Detected

        IF("$ENV{COSMOS}" STREQUAL "")
            SET(COSMOS "/usr/local/cosmos")
        ELSE()
            SET(COSMOS "$ENV{COSMOS}")
        ENDIF("$ENV{COSMOS}" STREQUAL "")

    ENDIF()

ENDIF(${CMAKE_HOST_SYSTEM_NAME} MATCHES "Linux")

# -----------------------------------------------
# compile on Mac host
IF(${CMAKE_HOST_SYSTEM_NAME} MATCHES "Darwin")

    MESSAGE("Compiling on Mac")

    # Mac OS X specific code
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    IF("$ENV{COSMOS}" STREQUAL "")
        SET(COSMOS "/Applications/cosmos/macos")
    ELSE()
        SET(COSMOS "$ENV{COSMOS}/macos")
    ENDIF("$ENV{COSMOS}" STREQUAL "")

    SET(COSMOS_SOURCE_CORE "${COSMOS_SOURCE}/core/")
ENDIF(${CMAKE_HOST_SYSTEM_NAME} MATCHES "Darwin")


# -----------------------------------------------
# compile on Windows host
IF(${CMAKE_HOST_SYSTEM_NAME} MATCHES "Windows")

   MESSAGE("---------------------")
   MESSAGE("Compiling on Windows")

   # Windows 7 specific code
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

    #   Option(${CROSS_TYPE} MATCHES "arm" "Build for ARM architecture" OFF)
    IF (${CROSS_TYPE} MATCHES "arm")
        SET(COSMOS "c:/cosmos/arm")
        IF("$ENV{COSMOS}" STREQUAL "")
#            SET(COSMOS_SOURCE "c:/cosmos/src")
        ELSE()
#            SET(COSMOS_SOURCE "$ENV{COSMOS}/src")
        ENDIF("$ENV{COSMOS}" STREQUAL "")

        SET(CMAKE_TOOLCHAIN_FILE "${PROJECT_SOURCE_DIR}/cmake/toolchain_arm_win")

        # Add include directories for locally compiled arm openssl
        include_directories(${PROJECT_SOURCE_DIR}/thirdparty/arm/include)

    ELSE()
        set(WINDOWS_LIBS  wsock32 winmm ws2_32 iphlpapi)
        IF("$ENV{COSMOS}" STREQUAL "")
            SET(COSMOS "c:/cosmos")
        ELSE()
            SET(COSMOS "$ENV{COSMOS}")
        ENDIF("$ENV{COSMOS}" STREQUAL "")
    ENDIF()

    SET(COSMOS_SOURCE_CORE "${COSMOS_SOURCE}/core/")

    # Convert "/" to "\" IF we are in Windows
#    string(REPLACE "\\" "/" COSMOS_SOURCE ${COSMOS_SOURCE})

ENDIF(${CMAKE_HOST_SYSTEM_NAME} MATCHES "Windows")


# -----------------------------------------------

set(USE_COSMOS_FROM "SOURCE")
include(${COSMOS_SOURCE}/core/cmake/use_cosmos_from_source.cmake)

# let's use the cosmos/core libraries
set(COSMOS_LIBS
    ${WINDOWS_LIBS}
    CosmosSupport
    CosmosAgent
    CosmosDeviceGeneral
    CosmosDeviceSerial
    CosmosDeviceCpu
    CosmosDeviceDisk
    CosmosMath
    )

message("COSMOS: ${COSMOS}")

set(CMAKE_INSTALL_PREFIX "${COSMOS}/${PROJECT_NAME}" CACHE PATH "default install path" FORCE )
message("Install: ${CMAKE_INSTALL_PREFIX}")

include_directories(${COSMOS_INCLUDE_DIRS})
link_directories(${COSMOS_LIB_DIRS})

include_directories(${PROJECT_SOURCE_DIR}/libraries)
add_subdirectory(${PROJECT_SOURCE_DIR}/libraries)

add_subdirectory(${PROJECT_SOURCE_DIR}/programs)
