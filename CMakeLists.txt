cmake_minimum_required(VERSION 3.10)
project(MiniDB)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# Main database app
add_executable(MiniDB
    src/main.cpp
    src/hash_table.cpp
    src/storage_engine.cpp
    src/write_ahead_log.cpp
    src/snapshot_manager.cpp
    src/tcp_server.cpp
)

# Link Winsock for TCP (Windows only)
target_link_libraries(MiniDB ws2_32)

# Stress test executable
add_executable(StressTest
    src/stress_test.cpp
    src/hash_table.cpp
    src/storage_engine.cpp
    src/write_ahead_log.cpp
    src/snapshot_manager.cpp
)