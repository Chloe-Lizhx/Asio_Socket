#include "Request.hpp"

namespace com{
    void Request::wait(std::vector<RequestPtr> &requests )
    {
        for(const auto &request:requests)
        {
            request->wait();
        }
    }
    Request::~Request() = default;
}