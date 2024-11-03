#include "SocketRequest.hpp"

namespace com{
    void SocketRequest::complete()
    {
        {
            std::lock_guard<std::mutex> lock(_completeMutex);
            _complete = true;
        }
        _completeCondition.notify_one();
    }

    bool SocketRequest::test()
    {
        std::lock_guard<std::mutex> lock(_completeMutex);

        return _complete;
    }

    void SocketRequest::wait()
    {
        std::unique_lock<std::mutex> lock(_completeMutex);

        _completeCondition.wait(lock,[this]{return _complete;});
    }
}