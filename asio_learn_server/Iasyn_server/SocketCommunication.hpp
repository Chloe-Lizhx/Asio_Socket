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
    /*portNumber = 0,代表端口号由系统分配，不建议自己设置
    reuseAddress = false,服务端因故连接断开后，一般会有2min的等待时间，这段时间内同样的IP+端口号不能再次启用
    std::string networkName = "lo",linux系统中的回环网络
    std::string addressDirectory = "."，地址交换文件夹在当前执行的程序所处的位置进行创建*/
    SocketCommunication(unsigned short portNumber = 0,
                        bool reuseAddress = false,
                        std::string networkName = "lo",
                        std::string addressDirectory = ".");

    virtual void acceptConnection(std::string const &acceptorName,
                                  std::string const &requesterName,
                                  std::string const &tag,
                                  int acceptorRank,
                                  int rankoffset = 0) override;

    virtual void acceptConnectionAsServer(std::string const &acceptorName,
                                        std::string const &requesterName,
                                        std::string const &tag,
                                        int                acceptorRank,
                                        int                requesterCommunicatorSize) override;

    virtual void requsetConnection(std::string const &acceptorName,
                                   std::string const &requesterName,
                                   std::string const &tag,
                                   int requesterRank,
                                   int requesterCommunicationSize) override;                             
    
    virtual void requestConnectionAsClient(std::string const &  acceptorName,
                                         std::string const &  requesterName,
                                         std::string const &  tag,
                                         std::set<int> const &acceptorRanks,
                                         int                  requesterRank) override;

    virtual void send(std::string const &itemtoSend,Rank rankReceiver) override;

    virtual void send(int itemstoSend,Rank rankReceiver) override;

    virtual void receive(std::string &itemtoReceive,Rank rankSender) override;

    virtual void receive(int &itemstoReceive,Rank rankSender) override;

    virtual void closeConnection() override;

    virtual size_t getRemoteCommunicatorSize() override;

    virtual void prepareEstablishment(std::string const &acceptorName,std::string const &requesterName) override;
    virtual void cleanupEstablishment(std::string const &acceptorName,std::string const &requesterName) override;

    virtual ~SocketCommunication();
    /// @brief 根据_networkName得到iP地址
    std::string getIpAddress();
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

    std::map<int,std::shared_ptr<Socket>> _sockets;
    SocketSendQueue _queue;
};
}