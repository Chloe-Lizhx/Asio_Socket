#pragma once

#include <string>
#include <stdexcept>
#include "SharedPointer.hpp"
#include "Communication.hpp"

namespace com{
    class CommunicationFactory
    {
    public:
        virtual ~CommunicationFactory(){};

        virtual CommunicationPtr newCommunication() = 0;

        virtual std::string AddressDirectory()
        {
            throw std::runtime_error("使用不当");
        }
    };
}