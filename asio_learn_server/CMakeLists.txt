cmake_minimum_required(VERSION 3.16.3)
find_package(Boost 1.74 REQUIRED COMPONENTS system regex thread)
add_subdirectory(Iasyn_server)
add_subdirectory(sync_server)