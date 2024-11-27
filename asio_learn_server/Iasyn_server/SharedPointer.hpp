#pragma once

#include <memory>
#include "Request.hpp"
#include "Communication.hpp"
#include "SocketCommunication.hpp"

namespace com{
    using RequestPtr = std::shared_ptr<Request>;
    using CommunicationPtr = std::shared_ptr<Communication>;
    using SocketCommunicationPtr = std::shared_ptr<SocketCommunication>;
}