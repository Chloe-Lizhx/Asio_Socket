#include <stddef.h>
#include <string>
#include <boost/range/irange.hpp>

namespace com{

using Rank = int;
class Communication{
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

    virtual void acceptConnection(std::string const &acceptorName,
                                  std::string const &requesterName,
                                  std::string const &tag,
                                  int acceptorRank,
                                  int rankoffset = 0) = 0;

    virtual void requsetConnection(std::string const &acceptorName,
                                   std::string const &requesterName,
                                   std::string const &tag,
                                   int requesterRank,
                                   int requesterCommunicationSize) = 0;

    virtual void send(std::string const &itemtoSend,Rank rankReceiver) = 0;

    virtual void receive(std::string &itemtoReceive,Rank rankSender) = 0;

    virtual void closeConnection() = 0;

    virtual void prepareEstablishment(std::string const &acceptorName,std::string const &requesterName) = 0;
    virtual void cleanupEstablishment(std::string const &acceptorName,std::string const &requesterName) = 0;

    void SetRankOffset(Rank rankOffset)
    {
        _RankOffset = rankOffset;
    }

protected:
    int _RankOffset = 0;
    bool _connected = {false};
    virtual int adjustRank(Rank rank) const;

};
}