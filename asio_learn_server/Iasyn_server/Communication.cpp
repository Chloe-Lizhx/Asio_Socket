#include "Communication.hpp"

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
            requsetConnection(primaryName,secondaryName,tag,secondaryRank,secondarySize);
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
}