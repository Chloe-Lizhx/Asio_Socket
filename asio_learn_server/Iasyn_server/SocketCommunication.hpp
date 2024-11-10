#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <thread>
#include <string>
#include <map>
#include "Communication.hpp"
#include "SocketSendQueue.hpp"

namespace com
{
class SocketCommunication : public Communication
{
public:
    SocketCommunication(unsigned short portNumber = 0,
                        bool reuseAddress = false,
                        std::string networkName = "lo",
                        std::string addressDirectory = ".");
    
    virtual void acceptConnection(std::string const &acceptorName,
                                  std::string const &requesterName,
                                  std::string const &tag,
                                  int acceptorRank,
                                  int rankoffset = 0) override;

    virtual void requsetConnection(std::string const &acceptorName,
                                   std::string const &requesterName,
                                   std::string const &tag,
                                   int requesterRank,
                                   int requesterCommunicationSize) override;

    virtual void send(std::string const &itemtoSend,Rank rankReceiver) override;

    virtual void receive(std::string &itemtoReceive,Rank rankSender) override;

    virtual void closeConnection() override;

    virtual void prepareEstablishment(std::string const &acceptorName,std::string const &requesterName) override;
    virtual void cleanupEstablishment(std::string const &acceptorName,std::string const &requesterName) override;

    virtual ~SocketCommunication();

private:
    unsigned short _portNumber;
    bool _reuseAddress;
    std::string _networkName;
    std::string _addressDirectory;

    using IO_service = boost::asio::io_service;
    using Work = boost::asio::io_service::work;
    using Socket = boost::asio::ip::tcp::socket;
    
    std::shared_ptr<IO_service> _service;
    std::shared_ptr<Work> _work;
    std::thread _thread;

    SocketSendQueue _queue;
    std::map<int,std::shared_ptr<Socket>> _sockets;

    std::string getIpAddress();

};
}