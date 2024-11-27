#pragma once 

#include "SocketCommunicationFactory.hpp"
#include "SocketCommunication.hpp"

namespace com{

SocketCommunicationFactory::SocketCommunicationFactory( 
    unsigned short portNumber       = 0,
    bool           reuseAddress     = false,
    std::string    networkName      = "lo",
    std::string    addressDirectory = ".")
    :portNumber(portNumber),
    reuseAddress(reuseAddress),
    networkName(networkName),
    addressDirectory(addressDirectory)
    {
        if(addressDirectory.empty())
        {
            addressDirectory = ".";
        }
    }

    SocketCommunicationFactory::SocketCommunicationFactory(
    std::string const &addressDirectory)
    : SocketCommunicationFactory(0, false, "lo", addressDirectory){}

    CommunicationPtr SocketCommunicationFactory::newCommunication()
    {
        return std::make_shared<SocketCommunication>
        (new SocketCommunication(portNumber,reuseAddress,networkName,addressDirectory));
    }

    std::string SocketCommunicationFactory::AddressDirectory()
    {
        return addressDirectory;
    }

}