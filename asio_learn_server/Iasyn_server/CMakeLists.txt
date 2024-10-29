set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
add_executable(server "server.cpp" "server.hpp" "SocketSendQueue.hpp" "SocketSendQueue.cpp")
target_link_libraries(server ${Boost_LIBRARIES})
