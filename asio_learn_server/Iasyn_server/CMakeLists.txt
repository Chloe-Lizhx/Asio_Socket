project(com)
set(CMAKE_CXX_STANDARD 20)#确保能够使用std::span
set(CXX_STANDARD_REQUIRED ON)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(SocketCommunicationTest 
"SocketCommunicationTest.cpp" 
"SharedPointer.hpp"
"CommunicationFactory.hpp"
"SocketCommunicationFactory.hpp"
"SocketCommunicationFactory.cpp"
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
set(test1 
"test/test1.cpp" 
"SharedPointer.hpp"
"CommunicationFactory.hpp"
"SocketCommunicationFactory.hpp"
"SocketCommunicationFactory.cpp"
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
set(test2 
"test/test2.cpp" 
"SharedPointer.hpp"
"CommunicationFactory.hpp"
"SocketCommunicationFactory.hpp"
"SocketCommunicationFactory.cpp"
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
#add_executable(test1 ${test1})
#add_executable(test2 ${test2})
target_compile_options(SocketCommunicationTest PRIVATE -g)
# target_compile_options(test1 PRIVATE -g)
# target_compile_options(test2 PRIVATE -g)
target_link_libraries(SocketCommunicationTest ${Boost_LIBRARIES})
# target_link_libraries(test1 ${Boost_LIBRARIES})
# target_link_libraries(test2 ${Boost_LIBRARIES})
