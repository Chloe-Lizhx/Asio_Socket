#include "SocketCommunicationFactory.hpp"
#include "SocketCommunication.hpp"

namespace com{

SocketCommunicationFactory::SocketCommunicationFactory( 
    unsigned short portNumber    ,
    bool           reuseAddress  ,   
    std::string    networkName   ,
    std::string    addressDirectory )
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
        (portNumber,reuseAddress,networkName,addressDirectory);
    }

    std::string SocketCommunicationFactory::AddressDirectory()
    {
        return addressDirectory;
    }

}