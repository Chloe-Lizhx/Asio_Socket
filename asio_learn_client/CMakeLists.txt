cmake_minimum_required(VERSION 3.16.3)
set(exepath ${PROJECT_SOURCE_DIR}/bin)
find_package(Boost 1.74 REQUIRED system regex)
add_subdirectory(Iasyn_client)
add_subdirectory(sync_client)