#include "Communication.hpp"

namespace com{

    int Communication::adjustRank(Rank rank) const
    {
        return rank - _RankOffset;
    }
}