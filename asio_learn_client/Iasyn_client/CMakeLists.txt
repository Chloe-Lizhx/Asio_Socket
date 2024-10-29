set(EXECUTABLE_OUTPUT_PATH ${exepath})
add_executable(client "client.cpp")
target_link_libraries(client ${Boost_LIBRARIES})