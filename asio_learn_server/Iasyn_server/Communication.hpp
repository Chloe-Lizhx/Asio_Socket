#pragma once

#include <stddef.h>
#include <string>
#include <set>
#include <boost/range/irange.hpp>

namespace com{

using Rank = int;
class Communication{
public:
    Communication &operator=(Communication &&) = delete;//禁止com1=std::move(com0)这种操作

    virtual ~Communication(){}

    virtual bool isconnected()
    {
        return _connected;
    }

    virtual size_t getRemoteCommunicatorSize() = 0;

    auto remoteCommunicatorRanks()
    {
        return boost::irange<Rank>(0,static_cast<Rank>(getRemoteCommunicatorSize()));
    }
    //接受多个客户端的连接，每个客户端必须有不同的requesterRank，相同的requesterCommunicationSize
    virtual void acceptConnection(std::string const &acceptorName,
                                  std::string const &requesterName,
                                  std::string const &tag,
                                  int acceptorRank,
                                  int rankoffset = 0) = 0;

    virtual void acceptConnectionAsServer(std::string const &acceptorName,
                                        std::string const &requesterName,
                                        std::string const &tag,
                                        int                acceptorRank,
                                        int                requesterCommunicatorSize) = 0;
            
    //请求连接至服务端，每一个SocketCommunicaton对象只能调用一次该方法。
    virtual void requsetConnection(std::string const &acceptorName,
                                   std::string const &requesterName,
                                   std::string const &tag,
                                   int requesterRank,
                                   int requesterCommunicationSize) = 0;

    virtual void requestConnectionAsClient(std::string const &  acceptorName,
                                         std::string const &  requesterName,
                                         std::string const &  tag,
                                         std::set<int> const &acceptorRanks,
                                         int                  requesterRank) = 0;

    virtual void send(std::string const &itemtoSend,Rank rankReceiver) = 0;

    virtual void send(int itemstoSend,Rank rankReceiver) = 0;

    virtual void receive(std::string &itemtoReceive,Rank rankSender) = 0;

    virtual void receive(int &itemstoReceive,Rank rankSender) = 0;

    virtual void closeConnection() = 0;

    virtual void prepareEstablishment(std::string const &acceptorName,std::string const &requesterName) = 0;
    virtual void cleanupEstablishment(std::string const &acceptorName,std::string const &requesterName) = 0;

    void SetRankOffset(Rank rankOffset)
    {
        _RankOffset = rankOffset;
    }

protected:
    int _RankOffset = 0;
    bool _connected = false;
    virtual int adjustRank(Rank rank) const;

};
}