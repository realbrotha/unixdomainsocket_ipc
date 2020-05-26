project(TestEpollPipe)
cmake_minimum_required(VERSION 3.5)

set(BIN_PATH ${CMAKE_CURRENT_BINARY_DIR}/output_bin)

set(COMMON_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/common )
set(UDS_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/lib )

add_subdirectory(src/lib)
add_subdirectory(src/client)
add_subdirectory(src/server)
