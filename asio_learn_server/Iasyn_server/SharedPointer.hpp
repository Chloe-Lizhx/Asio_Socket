#pragma once

#include <memory>
namespace com{
    class Communication;
    class SocketCommunication;
    class Request;
    using RequestPtr = std::shared_ptr<Request>;
    using CommunicationPtr = std::shared_ptr<Communication>;
    using SocketCommunicationPtr = std::shared_ptr<SocketCommunication>;
}