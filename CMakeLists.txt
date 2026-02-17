cmake_minimum_required(VERSION 3.10)

project(LocalhostServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(sqlite3 STATIC sqlite3_wrapper.c)

add_executable(server main.cpp)

target_link_libraries(server PRIVATE sqlite3)

set_target_properties(server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

target_compile_definitions(server PRIVATE CPPHTTPLIB_NO_UNIX_SOCKETS)

if(WIN32)
    target_link_libraries(server PRIVATE ws2_32)
endif()

if(UNIX)
    target_link_libraries(server PRIVATE pthread)
endif()