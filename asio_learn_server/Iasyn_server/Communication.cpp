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
}