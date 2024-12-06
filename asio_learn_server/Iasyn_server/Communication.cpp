#include "Communication.hpp"
#include "connectionInfo.hpp"

namespace com{

    void Communication::IntraConnect(std::string const &participantName,
                                     std::string const &tag,
                                     int               rank,
                                     int               size)
    {
        if(size == 1){return;}
        std::string primaryName = participantName + "primary";
        std::string secondaryName = participantName + "secondary";
        const Rank rankOffset = 1;
        int secondarySize = size - rankOffset;

        if(rank==0)
        {
            prepareEstablishment(primaryName,secondaryName);
            acceptConnection(primaryName,secondaryName,tag,rank,rankOffset);
            cleanupEstablishment(primaryName,secondaryName);
        }else{
            int secondaryRank = rank - rankOffset;
            requestConnection(primaryName,secondaryName,tag,secondaryRank,secondarySize);
        }
    }

    int Communication::adjustRank(Rank rank) const
    {
        return rank - _RankOffset;
    }

    void Communication::sendRange(std::span<const double> itemstoSend, Rank rankReceiver)
    {
        int size = itemstoSend.size();
        send(size,rankReceiver);
        if(size>0)
        {
        send(itemstoSend,rankReceiver);
        }
    }

    void Communication::sendRange(std::span<const int> itemstoSend, Rank rankReceiver)
    {
        int size = itemstoSend.size();
        send(size,rankReceiver);
        if(size>0)
        {
        send(itemstoSend,rankReceiver);
        }
    }
    std::vector<int> Communication::receiveRange(Rank rankSender, AsRangeTag<int>)
    {
        int size{-1};
        receive(size,rankSender);
        Assert((size<=0),"");
        std::vector<int> range;
        range.resize(size);
        receive(range,rankSender);
        return range;
    }

    std::vector<double> Communication::receiveRange(Rank rankSender, AsRangeTag<double>)
    {
        int size{-1};
        receive(size,rankSender);
        Assert((size<=0),"");
        std::vector<double> range;
        range.resize(size);
        receive(range,rankSender);
        return range;
    }

    //int
    void Communication::reduceSumForPrimaryRank(int itemstoSend,int &itemstoReceive)
    {
        itemstoReceive = itemstoSend;
        for(auto secondaryRank : remoteCommunicatorRanks())
        {
            auto request = aReceive(itemstoSend,secondaryRank + _RankOffset);
            request->wait();
            itemstoReceive += itemstoSend;
        }
    }

    void Communication::reduceSumForSecondaryRank(int &itemstoSend,Rank PrimaryRank)
    {
        auto request = aSend(itemstoSend,PrimaryRank);
        request->wait();
    }

    void Communication::AllreduceSumForPrimaryRank(int itemstoSend,int &itemstoReceive)
    {
        reduceSumForPrimaryRank(itemstoSend,itemstoReceive);

        std::vector<RequestPtr> requests;
        requests.reserve(getRemoteCommunicatorSize());
        for(auto secondaryRank : remoteCommunicatorRanks())
        {
            requests.push_back(aSend(itemstoReceive,secondaryRank + _RankOffset));
        }
        Request::wait(requests);
    }

    void Communication::AllreduceSumForSecondaryRank(int &itemstoSend,int &itemstoReceive,Rank PrimaryRank)
    {
        reduceSumForSecondaryRank(itemstoSend,PrimaryRank);

        auto request = aReceive(itemstoReceive,PrimaryRank);
        request->wait();
    }

    //std::span<int>
    void Communication::reduceSumForPrimaryRank(std::span<const int> itemstoSend,std::span<int> itemstoReceive)
    {
        Assert((itemstoSend.size()!=itemstoReceive.size()),"span数据长度不同");
        std::copy(itemstoSend.begin(),itemstoSend.end(),itemstoReceive.begin());
        std::vector<int> received(itemstoSend.size());
        for(auto secondaryRank : remoteCommunicatorRanks())
        {
            auto request = aReceive(received,secondaryRank + _RankOffset);
            request->wait();
            for(int i=0;i<received.size();i++)
            {
                itemstoReceive[i] += received[i];
            }
        }
    }

    void Communication::reduceSumForSecondaryRank(std::span<const int> itemstoSend,Rank PrimaryRank)
    {
        auto request = aSend(itemstoSend,PrimaryRank);
        request->wait();
    }

    void Communication::AllreduceSumForPrimaryRank(std::span<const int> itemstoSend,std::span<int> itemstoReceive)
    {
        reduceSumForPrimaryRank(itemstoSend,itemstoReceive);
        std::vector<RequestPtr> requests;
        for(auto secondaryRank : remoteCommunicatorRanks())
        {
            requests.push_back(aSend(itemstoReceive,secondaryRank + _RankOffset));
        }
        Request::wait(requests);
    }

    void Communication::AllreduceSumForSecondaryRank(std::span<const int> itemstoSend,std::span<int> itemstoReceive,Rank PrimaryRank)
    {
        reduceSumForSecondaryRank(itemstoSend,PrimaryRank);
        auto request = aReceive(itemstoReceive,PrimaryRank);
        request->wait();
    }

    // double
    void Communication::reduceSumForPrimaryRank(double itemstoSend,double &itemstoReceive)
    {
        itemstoReceive = itemstoSend;
        for(auto secondaryRank : remoteCommunicatorRanks())
        {
            auto request = aReceive(itemstoSend,secondaryRank + _RankOffset);
            request->wait();
            itemstoReceive += itemstoSend;
        }
    }

    void Communication::reduceSumForSecondaryRank(double &itemstoSend,Rank PrimaryRank)
    {
        auto request = aSend(itemstoSend,PrimaryRank);
        request->wait();
    }

    void Communication::AllreduceSumForPrimaryRank(double itemstoSend,double &itemstoReceive)
    {
        reduceSumForPrimaryRank(itemstoSend,itemstoReceive);

        std::vector<RequestPtr> requests;
        requests.reserve(getRemoteCommunicatorSize());
        for(auto secondaryRank : remoteCommunicatorRanks())
        {
            requests.push_back(aSend(itemstoReceive,secondaryRank + _RankOffset));
        }
        Request::wait(requests);
    }

    void Communication::AllreduceSumForSecondaryRank(double &itemstoSend,double &itemstoReceive,Rank PrimaryRank)
    {
        reduceSumForSecondaryRank(itemstoSend,PrimaryRank);

        auto request = aReceive(itemstoReceive,PrimaryRank);
        request->wait();
    }

    //std::span<double>
    void Communication::reduceSumForPrimaryRank(std::span<const double> itemstoSend,std::span<double> itemstoReceive)
    {
        Assert((itemstoSend.size()!=itemstoReceive.size()),"span数据长度不同");
        std::copy(itemstoSend.begin(),itemstoSend.end(),itemstoReceive.begin());
        std::vector<double> received(itemstoSend.size());
        for(auto secondaryRank : remoteCommunicatorRanks())
        {
            auto request = aReceive(received,secondaryRank + _RankOffset);
            request->wait();
            for(int i=0;i<received.size();i++)
            {
                itemstoReceive[i] += received[i];
            }
        }
    }

    void Communication::reduceSumForSecondaryRank(std::span<const double> itemstoSend,Rank PrimaryRank)
    {
        auto request = aSend(itemstoSend,PrimaryRank);
        request->wait();
    }

    void Communication::AllreduceSumForPrimaryRank(std::span<const double> itemstoSend,std::span<double> itemstoReceive)
    {
        reduceSumForPrimaryRank(itemstoSend,itemstoReceive);
        std::vector<RequestPtr> requests;
        for(auto secondary : remoteCommunicatorRanks())
        {
            requests.push_back(aSend(itemstoReceive,secondary + _RankOffset));
        }
        Request::wait(requests);
    }

    void Communication::AllreduceSumForSecondaryRank(std::span<const double> itemstoSend,std::span<double> itemstoReceive,Rank PrimaryRank)
    {
        reduceSumForSecondaryRank(itemstoSend,PrimaryRank);
        auto request = aReceive(itemstoReceive,PrimaryRank);
        request->wait();
    }
    /*******************/
    void Communication::AllreduceSumForPrimaryRank(std::span<const double> itemstoSend,std::vector<double> &itemstoReceive)
    {
        itemstoReceive.clear();
        itemstoReceive.resize(itemstoSend.size());
        reduceSumForPrimaryRank(itemstoSend,itemstoReceive);
        std::vector<RequestPtr> requests;
        for(auto secondary : remoteCommunicatorRanks())
        {
            requests.push_back(aSend(itemstoReceive,secondary + _RankOffset));
        }
        Request::wait(requests);

    }

    void Communication::AllreduceSumForSecondaryRank(std::span<const double> itemstoSend,std::vector<double> &itemstoReceive,Rank PrimaryRank)
    {
        itemstoReceive.clear();
        itemstoReceive.resize(itemstoSend.size());
        reduceSumForSecondaryRank(itemstoSend,PrimaryRank);
        auto request = aReceive(itemstoReceive,PrimaryRank);
        request->wait();
    }

    void Communication::AllreduceSumForPrimaryRank(std::span<const int> itemstoSend,std::vector<int> &itemstoReceive)
    {
        itemstoReceive.clear();
        itemstoReceive.resize(itemstoSend.size());
        reduceSumForPrimaryRank(itemstoSend,itemstoReceive);
        std::vector<RequestPtr> requests;
        for(auto secondary : remoteCommunicatorRanks())
        {
            requests.push_back(aSend(itemstoReceive,secondary + _RankOffset));
        }
        Request::wait(requests);
    }

    void Communication::AllreduceSumForSecondaryRank(std::span<const int> itemstoSend,std::vector<int> &itemstoReceive,Rank PrimaryRank)
    {
        itemstoReceive.clear();
        itemstoReceive.resize(itemstoSend.size());
        reduceSumForSecondaryRank(itemstoSend,PrimaryRank);
        auto request = aReceive(itemstoReceive,PrimaryRank);
        request->wait();
    }


    void Communication::broadcastForPrimaryRank(int itemstoSend)
    {
        std::vector<RequestPtr> requests;
        for(auto secondaryRank : remoteCommunicatorRanks())
        {
            requests.push_back(aSend(itemstoSend,secondaryRank + _RankOffset));
        }
        Request::wait(requests);
    }
    
    void Communication::broadcastForSecondaryRank(int &itemstoReceive, Rank rankBroadcaster)
    {
        auto request = aReceive(itemstoReceive,rankBroadcaster);
        request->wait();
    }

    void Communication::broadcastForPrimaryRank(std::span<const int> itemstoSend)
    {
        std::vector<RequestPtr> requests;
        for(auto secondaryRank : remoteCommunicatorRanks())
        {
            requests.push_back(aSend(itemstoSend,secondaryRank + _RankOffset));
        }
        Request::wait(requests);
    }

    void Communication::broadcastForSecondaryRank(std::span<int> itemstoReceive, Rank rankBroadcaster)
    {
        auto request = aReceive(itemstoReceive,rankBroadcaster);
        request->wait();
    }

    void Communication::broadcastForPrimaryRank(double itemstoSend)
    {
        std::vector<RequestPtr> requests;
        for(auto secondaryRank : remoteCommunicatorRanks())
        {
            requests.push_back(aSend(itemstoSend,secondaryRank + _RankOffset));
        }
        Request::wait(requests);
    }

    void Communication::broadcastForSecondaryRank(double &itemstoReceive, Rank rankBroadcaster)
    {
        auto request = aReceive(itemstoReceive,rankBroadcaster);
        request->wait();
    }

    void Communication::broadcastForPrimaryRank(std::span<const double> itemstoSend)
    {
        std::vector<RequestPtr> requests;
        for(auto secondaryRank : remoteCommunicatorRanks())
        {
            requests.push_back(aSend(itemstoSend,secondaryRank + _RankOffset));
        }
        Request::wait(requests);
    }

    void Communication::broadcastForSecondaryRank(std::span<double> itemstoReceive, Rank rankBroadcaster)
    {
        auto request = aReceive(itemstoReceive,rankBroadcaster);
        request->wait();
    }

    void Communication::broadcastForPrimaryRank(bool itemstoSend)
    {
        std::vector<RequestPtr> requests;
        for(auto secondaryRank : remoteCommunicatorRanks())
        {
            requests.push_back(aSend(itemstoSend,secondaryRank + _RankOffset));
        }
        Request::wait(requests);
    }

    void Communication::broadcastForSecondaryRank(bool &itemstoReceive, Rank rankBroadcaster)
    {
        auto request = aReceive(itemstoReceive,rankBroadcaster);
        request->wait();
    }

    void Communication::broadcastForPrimaryRank(std::vector<int> const &v)
    {
        broadcastForPrimaryRank(static_cast<int>(v.size()));
        broadcastForPrimaryRank(std::span<const int>{v});
    }

    void Communication::broadcastForSecondaryRank(std::vector<int> &v, Rank rankBroadcaster)
    {
        int size = -1;
        broadcastForSecondaryRank(size,rankBroadcaster);
        v.clear();
        v.resize(size);
        broadcastForSecondaryRank(std::span<int>{v},rankBroadcaster);
    }

    void Communication::broadcastForPrimaryRank(std::vector<double> const &v)
    {
        broadcastForPrimaryRank(static_cast<int>(v.size()));
        broadcastForPrimaryRank(std::span<const double>{v});
    }
    void Communication::broadcastForSecondaryRank(std::vector<double> &v, Rank rankBroadcaster)
    {
        int size = -1;
        broadcastForSecondaryRank(size,rankBroadcaster);
        v.clear();
        v.resize(size);
        broadcastForSecondaryRank(std::span<double>{v},rankBroadcaster);
    }

    void connectSeries(
    std::string const & participantName,
    std::string const & addressDirectory,
    std::string const & tag,
    int                 rank,
    int                 size,
    CommunicationPtr Comm)
    {
        Assert(Comm->isconnected(),"已经连接");
        Assert(!(rank>=0&&rank<size&&size>0),"参数错误");
        if(size==1)
        {return;}

        std::string acceptorName = participantName + "acceptor";
        std::string requesterName= participantName + "requester";

        conInfoReader conInfoLeft(acceptorName,requesterName,tag,addressDirectory);
        if ((rank % 2) == 0) {
            if(rank==0)
            {
            Comm->requestConnectionAsClient(acceptorName, requesterName, tag, {rank+1},rank);
            }
            else if(rank==size-1)
            {
            Comm->requestConnectionAsClient(acceptorName , requesterName, tag, {rank-1}, rank);
            }
            else 
            {
            Comm->requestConnectionAsClient(acceptorName, requesterName, tag,{rank-1,rank+1},rank);
            }
        } else {
            if(rank==size-1)
            {
            Comm->acceptConnectionAsServer(acceptorName, requesterName, tag, rank, 1);
            }
            else
            {
            Comm->acceptConnectionAsServer(acceptorName, requesterName, tag, rank, 2);
            }
        }
    }
}