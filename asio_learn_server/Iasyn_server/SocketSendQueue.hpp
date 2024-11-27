#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <mutex>
#include <functional>
#include <deque>
#include <string>

namespace com{
/*
1,该发送的功能是将连续n个异步发送可以按照我们希望的顺序发送
2、具体的实现细节
（1）第一个异步发送还没有完成，就将剩余n-1个（n-2,n-3...不一定）压入队列
（2）并不是要等到全部的异步发送事件压入队列，第一个异步发送才继续完成
（3）每一个异步发送的“完成事件”和后续异步发送“压入队列”，这两个事件用不同的线程
    对一个mutex锁争夺(注意_ready），谁争得这个锁就去执行相应的事件，我们也不知道在某一时刻
    队列中还有多少异步发送事件
3、这样做的好处：我认为是平衡负载，我们不能一次性压入所有的异步事件，也不能一次性执行异步事件，
   当异步事件太多时这样都不好。
*/
class SocketSendQueue
{
public:
    using Socket=boost::asio::ip::tcp::socket;
    SocketSendQueue() ;
    ~SocketSendQueue();

    SocketSendQueue(const SocketSendQueue &)=delete;//禁止拷贝构造
    SocketSendQueue &operator=(const SocketSendQueue &)=delete;//禁止拷贝赋值

    void dispatch(std::shared_ptr<Socket> sock,boost::asio::const_buffer data,std::function<void()> callback);
    void sendcomplete();
private:
    void process();
    struct itemtosend
    {
        std::shared_ptr<Socket> sock;
        boost::asio::const_buffer data;
        std::function<void()> callback;
    };

    bool _ready=true;
    std::deque<itemtosend> _sendqueue;
    std::mutex _queuemutex;
};
}