set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
add_executable(server1 "server1.cpp" )
target_link_libraries(server1 ${Boost_LIBRARIES})