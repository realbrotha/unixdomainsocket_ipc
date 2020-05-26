cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 14)
set(MODULE_NAME uds_lib)

set(SOURCES
        UnixDomainSocketExport.cpp
        UnixDomainSocketClient.cpp
        UnixDomainSocketServer.cpp
        UnixDomainSocketCore.cpp
        ../common/StringTool.cpp
        ../common/FileDescriptorTool.cpp
        ../common/EpollWrapper.cpp
        ../common/SocketWrapper.cpp
        )
        #UnixDomainSocketFactoryBase.h)
include_directories(${UDS_LIB_PATH} ${COMMON_PATH})

add_library(${MODULE_NAME} STATIC ${SOURCES})
find_package(Threads REQUIRED)

target_link_libraries(${MODULE_NAME}
        ${CMAKE_THREAD_LIBS_INIT})
