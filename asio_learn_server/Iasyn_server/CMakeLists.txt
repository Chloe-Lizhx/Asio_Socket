project(com)
set(CMAKE_CXX_STANDARD 20)#确保能够使用std::span
set(CXX_STANDARD_REQUIRED ON)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(SocketCommunicationTest 
"SocketCommunicationTest.cpp" 
"Communication.cpp" 
"Communication.hpp" 
"connectionInfo.hpp"
"connectionInfo.cpp"
"getIpAddress.hpp"
"SocketCommunication.cpp" 
"SocketCommunication.hpp"
"Request.cpp"
"Request.hpp"
"SocketRequest.cpp"
"SocketRequest.hpp"
"SocketSendQueue.cpp"
"SocketSendQueue.hpp"
"utils/assertion.hpp"
"utils/print.hpp")
add_executable(SocketCommunicationTest ${SocketCommunicationTest})
target_compile_options(SocketCommunicationTest PRIVATE -g)
target_link_libraries(SocketCommunicationTest ${Boost_LIBRARIES})
