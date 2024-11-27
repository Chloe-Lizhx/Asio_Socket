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
    /*
        在实际使用当中wait被解锁有两种方式
        （1）在wait之前，相应的request实例已经执行notify_one，并将_complete设置为true了，那么执行wait时由于_complete为true，wait直接返回
        （2）在wait之后，相应的request实例执行notify_one激活wait，并将_complete设置为true，wait被激活后检测_complete为true后退出等待。
        wait阻塞时，lock自动解锁。
    */
    void SocketRequest::wait()
    {
        std::unique_lock<std::mutex> lock(_completeMutex);

        _completeCondition.wait(lock,[this]{return _complete;});
    }

    SocketRequest::~SocketRequest() = default;
}