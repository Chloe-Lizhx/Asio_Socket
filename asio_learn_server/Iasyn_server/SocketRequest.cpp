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

        _completeCondition.wait(lock,[this]{return _complete;});//在_complete为false时等待，为true并且被激活时退出等待。
        //wait阻塞时，lock自动解锁。
    }
}