set(EXECUTABLE_OUTPUT_PATH ${exepath})
add_executable(client1 "client1.cpp" )
target_compile_options(client1 PRIVATE -g)
target_link_libraries(client1 ${Boost_LIBRARIES})